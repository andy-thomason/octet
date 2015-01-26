////////////////////////////////////////////////////////////////////////////////
//
//(C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// fluid example based on Joss Stam paper.
//

#include <valarray>
#include <memory>

namespace octet {
  /// Scene containing a box with octet.
  class example_fluids : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;

    class mesh_fluid : public mesh {
      struct my_vertex {
        vec3p pos;
        vec3p color;
      };

      dynarray<my_vertex> vertices;

      std::vector<float> prev_density;
      std::vector<float> prev_vx;
      std::vector<float> prev_vy;

      std::vector<float> density;
      std::vector<float> vx;
      std::vector<float> vy;

      ivec3 dim;
    public:
      mesh_fluid(aabb_in bb, ivec3_in dim) : mesh(), dim(dim) {
        mesh::set_aabb(bb);

        density.resize((dim.x()+1)*(dim.y()+1));
        vx.resize((dim.x()+1)*(dim.y()+1));
        vy.resize((dim.x()+1)*(dim.y()+1));

        prev_density.resize((dim.x()+1)*(dim.y()+1));
        prev_vx.resize((dim.x()+1)*(dim.y()+1));
        prev_vy.resize((dim.x()+1)*(dim.y()+1));

        //density[50 +(dim.x()+1) * 50] = 1;
        //prev_vx[50 +(dim.x()+1) * 50] = 1;

        dynarray<uint32_t> indices;
        int stride = dim.x() + 1;
        for (int i = 0; i < dim.x(); ++i) {
          for (int j = 0; j < dim.y(); ++j) {
            indices.push_back((i+1) +(j+0)*stride);
            indices.push_back((i+0) +(j+1)*stride);
            indices.push_back((i+1) +(j+1)*stride);
            indices.push_back((i+1) +(j+0)*stride);
            indices.push_back((i+0) +(j+0)*stride);
            indices.push_back((i+0) +(j+1)*stride);
          }
        }
        set_indices(indices);
        clear_attributes();
        add_attribute(attribute_pos, 3, GL_FLOAT, 0);
        add_attribute(attribute_color, 3, GL_FLOAT, 12);
      }

      /// clamp edges to same (or negative) value of inner pixels
      /// to act as a barrier
      void set_boundary( int N, int b, float * x ) {
        auto IX = [=](int i, int j) { return i +(N+2)*j; };

	      for ( int i=1 ; i<=N ; i++ ) {
		      x[IX(0  ,i)] = b==1 ? -x[IX(1,i)] : x[IX(1,i)];
		      x[IX(N+1,i)] = b==1 ? -x[IX(N,i)] : x[IX(N,i)];
		      x[IX(i,0  )] = b==2 ? -x[IX(i,1)] : x[IX(i,1)];
		      x[IX(i,N+1)] = b==2 ? -x[IX(i,N)] : x[IX(i,N)];
	      }

	      x[IX(0  ,0  )] = 0.5f*(x[IX(1,0  )]+x[IX(0  ,1)]);
	      x[IX(0  ,N+1)] = 0.5f*(x[IX(1,N+1)]+x[IX(0  ,N)]);
	      x[IX(N+1,0  )] = 0.5f*(x[IX(N,0  )]+x[IX(N+1,1)]);
	      x[IX(N+1,N+1)] = 0.5f*(x[IX(N,N+1)]+x[IX(N+1,N)]);
      }

      /// solve diffusion equations (propagation into adjacent cells)
      /// by repeated aplication of a weighted average.
      /// Use a fixed number of iterations. 
      /// at the end x should not change (you should test this yourself)
      void gauss_siedel( int N, int b, float * x, float * x0, float a, float c ) {
        auto IX = [=](int i, int j) { return i +(N+2)*j; };

	      for ( int k=0 ; k<20 ; k++ ) {
		      for ( int i=1 ; i<=N ; i++ ) {  
            for ( int j=1 ; j<=N ; j++ ) {
			        x[IX(i,j)] = (x0[IX(i,j)] + a*(x[IX(i-1,j)]+x[IX(i+1,j)]+x[IX(i,j-1)]+x[IX(i,j+1)]))/c;
            }
          }
		      set_boundary( N, b, x );
	      }
      }

      /// calculate diffision by approximating with a weighted average
      void diffusion( int N, int b, float * x, float * x0, float diff, float dt ) {
	      float a = dt * diff * (N * N);
	      gauss_siedel( N, b, x, x0, a, 1+4*a );
      }

      /// carry a quantity (velocity or density) from one cell to another.
      void advection_step( int N, int b, float * d, float * d0, float * u, float * v, float dt ) {
        auto IX = [=](int i, int j) { return i +(N+2)*j; };

	      float dt0 = dt*N;
		    for ( int i=1 ; i<=N ; i++ ) {  
          for ( int j=1 ; j<=N ; j++ ) {
            // (x, y) is the address to copy from
		        float x = i-dt0*u[IX(i,j)], y = j-dt0*v[IX(i,j)];

            // clamp x and y
		        if (x<0.5f) x=0.5f; else if (x>N+0.5f) x=N+0.5f;
		        if (y<0.5f) y=0.5f; else if (y>N+0.5f) y=N+0.5f;

            // s1 and s0 are lerp coordinates [0,1) within the source cell
            int i0=(int)x, i1=i0+1;
            int j0=(int)y, j1=j0+1;
		        float s1 = x - i0, s0 = 1 - s1;
            float t1 = y - j0, t0 = 1 - t1;

            // sample the source
		        d[IX(i,j)] =
              s0*(t0*d0[IX(i0,j0)]+t1*d0[IX(i0,j1)]) +
              s1*(t0*d0[IX(i1,j0)]+t1*d0[IX(i1,j1)])
            ;
          }
        }

        // copy values out to the boundary.
	      set_boundary( N, b, d );
      }

      // stablisation step. adjust the velocity to prevent increase in energy
      // in the system.
      void project( int N, float * u, float * v, float * p, float * div ) {
        auto IX = [=](int i, int j) { return i +(N+2)*j; };

        // calculate divergence into div
        // set initial value of p
		    for ( int i=1 ; i<=N ; i++ ) {  
          for ( int j=1 ; j<=N ; j++ ) {
		        div[IX(i,j)] = -0.5f*(u[IX(i+1,j)]-u[IX(i-1,j)]+v[IX(i,j+1)]-v[IX(i,j-1)])/N;
		        p[IX(i,j)] = 0;
          }
        }

        // copy pixels to boundary
	      set_boundary( N, 0, div );
        set_boundary( N, 0, p );

        // p += div[x+/-1, y+/-1] * 4;
	      gauss_siedel( N, 0, p, div, 1, 4 );

        // calculate velocity from pressure-like "p"
		    for ( int i=1 ; i<=N ; i++ ) {  
          for ( int j=1 ; j<=N ; j++ ) {
            // u from left and right
		        u[IX(i,j)] -= 0.5f*N*(p[IX(i+1,j)]-p[IX(i-1,j)]);

            // v from up and down.
		        v[IX(i,j)] -= 0.5f*N*(p[IX(i,j+1)]-p[IX(i,j-1)]);
          }
        }

        // copy velocity to boundary
	      set_boundary( N, 1, u );
        set_boundary( N, 2, v );
      }

      /// Given a velocity field, carry a value around the simulation
      /// and diffuse the value.
      void density_step( int N, float * x, float * x0, float * u, float * v, float diff, float dt ) {
        // apply diffusion to density. If there is no velocity, the value will still spread.
	      std::swap( x0, x );
        diffusion( N, 0, x, x0, diff, dt );

        // use the velocity field to carry density around.
	      std::swap( x0, x );
        advection_step( N, 0, x, x0, u, v, dt );
      }

      /// Compute the new velocity field.
      void velocity_step( int N, float * u, float * v, float * u0, float * v0, float visc, float dt ) {
        // diffuse into neighouring cells
	      std::swap( u0, u );
        diffusion( N, 1, u, u0, visc, dt );
	      std::swap( v0, v );
        diffusion( N, 2, v, v0, visc, dt );

        // stabilise the system using poisson
	      project( N, u, v, u0, v0 );
	      std::swap( u0, u );
        std::swap( v0, v );

        // use advection to move the velocity itself
	      advection_step( N, 1, u, u0, u0, v0, dt );
        advection_step( N, 2, v, v0, u0, v0, dt );

        // stabilise the system using poisson
	      project( N, u, v, u0, v0 );
      }

      void update(int frame_number) {
        float dt = 1.0f / 30;
        int N = dim.x()-1;
        assert(density.size() == (N+2)*(N+2));
        float *u = vx.data(), *v = vy.data(), *u_prev = prev_vx.data(), *v_prev = prev_vy.data();
        float *dens = density.data(), *dens_prev = prev_density.data();
        float visc = 0.0f;
        float diff = 0.0f;

        //printf("dtot=%f\n", std::accumulate(density.cbegin(), density.cend(), 0.0f));

        // fill the input values
        std::fill(prev_vx.begin(), prev_vx.end(), 0.0f);
        std::fill(prev_vy.begin(), prev_vy.end(), 0.0f);
        std::fill(prev_density.begin(), prev_density.end(), 0.0f);

        // you could use a UI to do this.
        float c = math::cos(frame_number*0.01f);
        float s = math::sin(frame_number*0.01f);
        density[50 +(dim.x()+1) * 50] += 100 * dt;
        u[50 +(dim.x()+1) * 50] += c * (100 * dt);
        v[50 +(dim.x()+1) * 50] += s * (100 * dt);

        // step the simulation.
	      //get_from_UI( dens_prev, u_prev, v_prev );
        long long t0 = __rdtsc();
	      velocity_step( N, u, v, u_prev, v_prev, visc, dt );
	      density_step( N, dens, dens_prev, u, v, diff, dt );
        long long t1 = __rdtsc();
        printf("%lld clocks\n", t1-t0);

        //printf("dtot=%f\n", std::accumulate(density.cbegin(), density.cend(), 0.0f));

        aabb bb = mesh::get_aabb();
        float sx = bb.get_half_extent().x()*(2.0f/dim.x());
        float sy = bb.get_half_extent().y()*(2.0f/dim.y());
        float cx = bb.get_center().x() - bb.get_half_extent().x();
        float cy = bb.get_center().y() - bb.get_half_extent().y();
        vertices.resize((dim.x()+1)*(dim.y()+1));
        int stride =(dim.x()+1);
        size_t d = 0;
        for (int i = 0; i <= dim.x(); ++i) {
          for (int j = 0; j <= dim.y(); ++j) {
            my_vertex v;
            v.pos = vec3p(i * sx + cx, j * sy + cy, 0);
            v.color = vec3p(std::max(0.0f, std::min(density[i+j*stride], 1.0f) ), 0, 0);
            vertices[d++] = v;
          }
        }

        mesh::set_vertices<my_vertex>(vertices);
      }
    };

    ref<mesh_fluid> the_mesh;
  public:
    /// this is called when we construct the class before everything is initialised.
    example_fluids(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();

      material *red = new material(vec4(1, 0, 0, 1), new param_shader("shaders/simple_color.vs", "shaders/simple_color.fs"));
      the_mesh = new mesh_fluid(aabb(vec3(0), vec3(10)), ivec3(100, 100, 0));
      scene_node *node = new scene_node();
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, the_mesh, red));
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy, vec4(0, 0, 0, 1));

      the_mesh->update(get_frame_number());

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      // draw the scene
      app_scene->render((float)vx / vy);
    }
  };
}
