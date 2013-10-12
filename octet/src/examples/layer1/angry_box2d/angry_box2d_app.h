////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// angry_box2d example: simple 2d physics game with sprites and sounds
//
// Level: 1
//
// Demonstrates:
//   Basic framework app
//   Shaders
//   Basic Matrices
//   Simple game mechanics
//   Texture loaded from GIF file
//   Audio
//   Simple 2D physics
//

namespace octet {
  class sprite_2d {
    // rigid body information for this sprite.
    b2Body *body;

    // what texture is on our sprite?
    int texture;

    // uvs are derived fromn positions with these scales.
    float uscale;
    float vscale;
    float uoffset;
    float voffset;

    // true if this sprite_2d is enabled.
    bool enabled;
  public:
    sprite_2d(
      b2World &world, b2BodyDef &body_def, b2FixtureDef &fixture_def,
      int _texture, float uscale_ = 1.0f, float vscale_ = 1.0f, float uoffset_ = 0.5f, float voffset_ = 0.5f
    ) {
      texture = 0;
      enabled = true;
      uscale = uscale_;
      vscale = vscale_;
      uoffset = uoffset_;
      voffset = voffset_;

      body = world.CreateBody(&body_def);
      body->CreateFixture(&fixture_def);

      texture = _texture;
      enabled = true;
    }

    void render(texture_shader &shader, mat4t &cameraToWorld) {
      if (!texture) return;

      mat4t modelToWorld;
      modelToWorld.loadIdentity();
      modelToWorld.translate(body->GetPosition().x, body->GetPosition().y, 0);
      modelToWorld.rotateZ(body->GetAngle() * (180/3.14159265f));

      // build a projection matrix: model -> world -> camera -> projection
      // the projection space is the cube -1 <= x/w, y/w, z/w <= 1
      mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

      // set up opengl to draw textured triangles using sampler 0 (GL_TEXTURE0)
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);

      shader.render(modelToProjection, 0);

      b2Fixture *fixture_def = body->GetFixtureList();
      b2Shape *shape = fixture_def->GetShape();

      float vertices[b2_maxPolygonVertices*4];
      int count = 0;

      if (shape->GetType() == b2Shape::e_polygon) {
        b2PolygonShape *poly = (b2PolygonShape *)shape;
     		count = poly->GetVertexCount();

		    float *p = vertices;
		    for (int i = 0; i != count; ++i) {
		      b2Vec2 vtx = poly->GetVertex(i);
		      p[0] = vtx.x;
		      p[1] = vtx.y;
		      p[2] = vtx.x * uscale + uoffset;
		      p[3] = vtx.y * vscale + voffset;
		      p += 4;
		    }
      }

      glVertexAttribPointer(attribute_pos, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)vertices );
      glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(vertices + 2) );
      glEnableVertexAttribArray(attribute_pos);
      glEnableVertexAttribArray(attribute_uv);
   
      // finally, draw the sprite as a fan
      glDrawArrays(GL_TRIANGLE_FAN, 0, count);
    }

    bool &is_enabled() {
      return enabled;
    }
  };

  class angry_box2d_app : public octet::app, public b2ContactListener {
    // physics systems need a "world" to stitch together rigid bodies.
    b2World box2d_world;

    // Matrix to transform points in our camera space to the world.
    // This lets us move our camera
    mat4t cameraToWorld;

    // shader to draw a textured triangle
    texture_shader texture_shader_;

    enum {
      num_sound_sources = 8,
    };

    // game state
    bool game_over;

    // sounds
    ALuint whoosh;
    ALuint bang;
    unsigned cur_source;
    ALuint sources[num_sound_sources];

    // big array of sprites
    dynarray<sprite_2d*> sprites;

    ALuint get_sound_source() { return sources[cur_source++ % num_sound_sources]; }

  public:
    // this is called when we construct the class
    angry_box2d_app(int argc, char **argv) : app(argc, argv), box2d_world(b2Vec2(0, -9.8f)) {
    }

    // this is called once OpenGL is initialized
    void app_init() {
      // set up the shader
      texture_shader_.init();

      // set up the matrices with a camera 5 units from the origin
      cameraToWorld.loadIdentity();
      cameraToWorld.translate(0, 0, 5);

      GLuint ship = resources::get_texture_handle(GL_RGBA, "assets/invaderers/ship.gif");
      GLuint GameOver = resources::get_texture_handle(GL_RGBA, "assets/invaderers/GameOver.gif");
      GLuint invaderer = resources::get_texture_handle(GL_RGBA, "assets/invaderers/invaderer.gif");

      // sounds
      whoosh = resources::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/whoosh.wav");
      bang = resources::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/bang.wav");
      cur_source = 0;
      alGenSources(num_sound_sources, sources);

      b2PolygonShape poly;

      b2BodyDef body_def;
      body_def.position.Set(0, 0);
      body_def.angle = 0;
  	  body_def.type = b2_dynamicBody;

      b2FixtureDef fixture_def;
      fixture_def.shape = &poly;

      for (int i = 0; i != 5; ++i) {
        for (int j = 0; j != 5; ++j) {
          poly.SetAsBox(0.25f, 0.25f);
          body_def.position.x = ( i - 2.5f )* 0.75f;
          body_def.position.y = ( j - 2.5f ) * 0.75f;
          body_def.angle = ( i + j ) * 0.3f;
          sprites.push_back(
            new sprite_2d(box2d_world, body_def, fixture_def, ship, 2, 2)
          );
        }
      }

      poly.SetAsBox(10.0f, 0.5f);
  	  body_def.type = b2_staticBody;

      // floor
      body_def.angle = (3.14159265f/180) * 20;
      body_def.position.Set(2, -4);
      sprites.push_back(
        new sprite_2d(box2d_world, body_def, fixture_def, invaderer)
      );

      // floor
      body_def.angle = -(3.14159265f/180) * 20;
      body_def.position.Set(-2, -4);
      sprites.push_back(
        new sprite_2d(box2d_world, body_def, fixture_def, invaderer)
      );

      game_over = false;
    }

    // called every frame to move things
    void simulate() {
      if (game_over) {
        return;
      }

      box2d_world.Step(1.0f/30, 6, 2);
    }

    // this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      simulate();

      // set a viewport - includes whole window area
      glViewport(x, y, w, h);

      // clear the background to black
      glClearColor(0, 0, 0, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // don't allow Z buffer depth testing (closer objects are always drawn in front of far ones)
      glDisable(GL_DEPTH_TEST);

      // allow alpha blend (transparency when alpha channel is 0)
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      // draw all the sprites
      for (int i = 0; i != sprites.size(); ++i) {
        sprites[i]->render(texture_shader_, cameraToWorld);
      }

      // move the listener with the camera
      vec4 &cpos = cameraToWorld.w();
      alListener3f(AL_POSITION, cpos.x(), cpos.y(), cpos.z());
    }

    // event called every time we begin a contact
    void BeginContact(b2Contact* contact) {
      ALuint source = get_sound_source();
      alSourcei(source, AL_BUFFER, bang);
      alSourcePlay(source);
    }
  };
}
