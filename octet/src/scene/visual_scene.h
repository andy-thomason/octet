////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Scene Node heirachy
//

namespace octet { namespace scene {
  /// Visual scene; contains instances of meshes, cameras and lights required to draw a scene.
  class visual_scene : public scene_node {
    ///////////////////////////////////////////
    //
    // rendering information
    //

    /// each of these is a set of (scene_node, mesh, material)
    dynarray<ref<mesh_instance> > mesh_instances;

    /// animations playing at the moment
    dynarray<ref<animation_instance> > animation_instances;

    /// cameras available
    dynarray<ref<camera_instance> > camera_instances;

    /// lights available
    dynarray<ref<light_instance> > light_instances;

    /// set this to draw bounding boxes
    bool render_aabbs;
    bool render_debug_lines;
    bool dump_vertices;
    ref<material> debug_material;
    dynarray<vec3p> debug_line_buffer;
    unsigned debug_in_ptr;

    /// derived light information
    enum { max_lights = material::max_lights, light_size = material::light_size, ambient_size = material::ambient_size };
    int num_light_uniforms;
    int num_lights;
    vec4 light_uniforms[ambient_size + max_lights * light_size ];

    int frame_number;

    /// shaders to draw triangles
    ref<bump_shader> object_shader;
    ref<bump_shader> skin_shader;

    #ifdef OCTET_BULLET
      btDefaultCollisionConfiguration config;       /// setup for the world
      btCollisionDispatcher *dispatcher;            /// handler for collisions between objects
      btDbvtBroadphase *broadphase;                 /// handler for broadphase (rough) collision
      btSequentialImpulseConstraintSolver *solver;  /// handler to resolve collisions
      btDiscreteDynamicsWorld *world;             /// physics world, contains rigid bodies
      typedef btCollisionShape collison_shape_t;
    #else
      typedef void collison_shape_t;
    #endif

    void draw_aabb(const aabb &bb) {
      vec3 pos[8];
      vec3 center = bb.get_center();
      vec3 half = bb.get_half_extent();
      for (int i = 0; i != 8; ++i) {
        pos[i] = center + half * vec3(
          (i & 1 ? 1.0f : -1.0f),
          (i & 2 ? 1.0f : -1.0f),
          (i & 4 ? 1.0f : -1.0f)
        );
      }

      static const uint16_t indices[] = {
        0, 1, 2, 3, 4, 5, 6, 7,
        0, 2, 1, 3, 4, 6, 5, 7,
        0, 4, 1, 5, 2, 6, 3, 7
      };

      /// render immediate data (this is inefficient!)
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 0, (void*)pos );
      glEnableVertexAttribArray(attribute_pos);
    
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, indices);
      glDisableVertexAttribArray(attribute_pos);
    }

    void calc_lighting(const mat4t &worldToCamera) {
      vec4 &ambient = light_uniforms[0];
      ambient = vec4(0, 0, 0, 1);
      num_lights = 0;
      int num_ambient = 0;
      for (unsigned i = 0; i != light_instances.size() && num_lights != max_lights; ++i) {
        light_instance *li = light_instances[i];
        light *light = li->get_light();
        scene_node *node = li->get_node();
        atom_t kind = light->get_kind();
        if (kind == atom_ambient) {
          ambient += light->get_color();
          num_ambient++;
        } else {
          light->get_fragment_uniforms(node, &light_uniforms[ambient_size+num_lights*light_size], worldToCamera);
          num_lights++;
        }
      }
      if (num_ambient == 0) {
        ambient = vec4(0.5f, 0.5f, 0.5f, 1);
      }
      num_light_uniforms = ambient_size + num_lights * light_size;
    }

    void render_mesh_aabbs() {
      for (unsigned mesh_index = 0; mesh_index != mesh_instances.size(); ++mesh_index) {
        mesh_instance *mi = mesh_instances[mesh_index];
        aabb bb = mi->get_mesh()->get_aabb();
        bb = bb.get_transform(mi->get_node()->calcModelToWorld());
        draw_aabb(bb);
      }
    }

    void render_debug_line_buffer() {
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 12, (void*)debug_line_buffer.data() );
      glEnableVertexAttribArray(attribute_pos);
    
      glDrawArrays(GL_LINES, 0, debug_line_buffer.size());
      glDisableVertexAttribArray(attribute_pos);
    }

    void dump_mesh_vertices(camera_instance &cam) {
      for (unsigned mesh_index = 0; mesh_index != mesh_instances.size(); ++mesh_index) {
        mesh_instance *mi = mesh_instances[mesh_index];
        mesh *msh = mi->get_mesh();
        mat4t modelToWorld = mi->get_node()->calcModelToWorld();
        mat4t modelToCamera;
        mat4t modelToProjection;
        cam.get_matrices(modelToProjection, modelToCamera, modelToWorld);
        const char *ip = (const char*)msh->get_indices()->lock_read_only();
        const char *vp = (const char*)msh->get_vertices()->lock_read_only();
        unsigned pos_offset = msh->get_offset(msh->get_slot(attribute_pos));
        unsigned stride = msh->get_stride();
        bool is_short_index = msh->get_index_type() != GL_UNSIGNED_INT;

        for (unsigned i = 0; i != msh->get_num_indices(); ++i) {
          unsigned index = is_short_index ? ((uint16_t*)ip)[i] : ((uint32_t*)ip)[i];
          const vec3p &pos = (const vec3p&)*(vp + stride * index + pos_offset);
          vec4 pos1 = vec3(pos).xyz1();
          vec4 world_pos = pos1 * modelToWorld;
          vec4 proj_pos = pos1 * modelToProjection;
          log("%5d i=%5d m=[%9.3f, %9.3f, %9.3f] w=[%9.3f, %9.3f, %9.3f] p=[%9.3f, %9.3f, %9.3f]\n",
            i, index,
            pos1.x(), pos1.y(), pos1.z(),
            world_pos.x(), world_pos.y(), world_pos.z(),
            proj_pos.x()/proj_pos.w(), proj_pos.y()/proj_pos.w(), proj_pos.z()/proj_pos.w()
          );
        }

        msh->get_indices()->unlock_read_only();
        msh->get_vertices()->unlock_read_only();
      }
    }

    void draw_debug_data(camera_instance &cam) {
      /// debug draw the AABBs of the mesh instances in the world.
      /// draw in world space
      mat4t worldToCamera;
      mat4t worldToProjection;
      mat4t worldToWorld;
      worldToWorld.loadIdentity();
      cam.get_matrices(worldToProjection, worldToCamera, worldToWorld);
      debug_material->render(worldToProjection, worldToCamera, light_uniforms, num_light_uniforms, num_lights);

      /// debug lines are a useful way of showing dynamic behaviour in the scene.
      if (render_debug_lines) {
        render_debug_line_buffer();
      }

      if (render_aabbs) {
        render_mesh_aabbs();
      }
    }

    void render_impl(bump_shader &object_shader, bump_shader &skin_shader, camera_instance &cam, float aspect_ratio) {
      mat4t cameraToWorld = cam.get_node()->calcModelToWorld();

      mat4t worldToCamera;
      cameraToWorld.invertQuick(worldToCamera);

      calc_lighting(worldToCamera);

      cam.set_cameraToWorld(cameraToWorld, aspect_ratio);
      mat4t cameraToProjection = cam.get_cameraToProjection();

      draw_debug_data(cam);

      for (unsigned mesh_index = 0; mesh_index != mesh_instances.size(); ++mesh_index) {
        mesh_instance *mi = mesh_instances[mesh_index];

        scene_node *node = mi->get_node();
        unsigned flags = mi->get_flags();

        if (
          !(flags & mesh_instance::flag_enabled) ||
          !node->calcEnabled()
        ) continue;

        mesh *msh = mi->get_mesh();
        skin *skn = msh->get_skin();
        skeleton *skel = mi->get_skeleton();
        material *mat = mi->get_material();

        mat4t modelToWorld = node->calcModelToWorld();
        mat4t modelToCamera;
        mat4t modelToProjection;
        cam.get_matrices(modelToProjection, modelToCamera, modelToWorld);
        //printf("%d %f\n", mesh_index, modelToWorld.w().y());

        // selecting LOD meshes by distance
        if (flags & mesh_instance::flag_lod) {
          float distance = -modelToCamera.w().z();
          //printf("%f %f %f\n", distance, mi->get_min_draw_distance(), mi->get_max_draw_distance());
          if (
            distance < mi->get_min_draw_distance() ||
            distance >= mi->get_max_draw_distance()
          ) {
            continue;
          }
        }

        if (!skel || !skn) {
          /// normal rendering for single matrix objects
          /// build a projection matrix: model -> world -> camera_instance -> projection
          /// the projection space is the cube -1 <= x/w, y/w, z/w <= 1
          mat->render(modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);
        } else {
          /// multi-matrix rendering
          mat4t *transforms = skel->calc_transforms(modelToCamera, skn);
          int num_bones = skel->get_num_bones();
          if(num_bones > 192) {
            GLint mvuv = 0;
            //glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &mvuv);
            printf("warning: too many bones (%d/%d)\n", num_bones, mvuv/4);
          } else {
            mat->render_skinned(cameraToProjection, transforms, num_bones, light_uniforms, num_light_uniforms, num_lights);
          }
        }

        /*if (true) {
          static bool dumped;
          if (!dumped) { msh->dump_transformed(modelToProjection); dumped = true; }
        }*/
        msh->enable_attributes();
        msh->draw();
        msh->disable_attributes();

        if (mi->get_flags() & mesh_instance::flag_selected) {
          aabb bb = mi->get_mesh()->get_aabb();
          bb = bb.get_transform(mi->get_node()->calcModelToWorld());
          draw_aabb(bb);
        }
      }
      frame_number++;
    }
  public:
    RESOURCE_META(visual_scene)

    /// Create an empty visual_scene; Use add_* functions to add components to the scene.
    visual_scene() {
      frame_number = 0;
      num_light_uniforms = 0;
      num_lights = 0;
      render_aabbs = false;
      dump_vertices = false;
      render_debug_lines = false;
      debug_material = new material(vec4(1, 0, 0, 1));
      debug_line_buffer.resize(256);
      assert(is_power_of_two(debug_line_buffer.size()));
      memset(&debug_line_buffer[0], 0, debug_line_buffer.size() * sizeof(debug_line_buffer[0]));
      debug_in_ptr = 0;

      #ifdef OCTET_BULLET
        dispatcher = new btCollisionDispatcher(&config);
        broadphase = new btDbvtBroadphase();
        solver = new btSequentialImpulseConstraintSolver();
        world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, &config);
      #endif
    }

    ~visual_scene() {
      #ifdef OCTET_BULLET
        delete world;
        delete solver;
        delete broadphase;
        delete dispatcher;
      #endif
    }

    /// helper to add a mesh to a scene and also to create the corresponding physics object
    mesh_instance *add_shape(mat4t_in mat, mesh *msh, material *mtl, bool is_dynamic=false, float mass=1, collison_shape_t *shape=NULL) {
      scene_node *node = new scene_node(this);
      node->access_nodeToParent() = mat;

      mesh_instance *result = NULL;
      if (msh && mtl) {
        result = new mesh_instance(node, msh, mtl);
        add_mesh_instance(result);
      }

      #ifdef OCTET_BULLET
        btMatrix3x3 matrix(get_btMatrix3x3(mat));
        btVector3 pos(get_btVector3(mat[3].xyz()));

        if (shape == NULL) {
          shape = is_dynamic ? msh->get_bullet_shape() : msh->get_static_bullet_shape();
        }

        if (shape) {
          btTransform transform(matrix, pos);

          btDefaultMotionState *motionState = new btDefaultMotionState(transform);
          btVector3 inertiaTensor;

          if (!is_dynamic) mass = 0;
   
          if (is_dynamic) shape->calculateLocalInertia(mass, inertiaTensor);
    
          btRigidBody * rigid_body = new btRigidBody(mass, motionState, shape, inertiaTensor);
          world->addRigidBody(rigid_body);
          rigid_body->setUserPointer(node);
          node->set_rigid_body(rigid_body);
        }
      #endif
      return result;
    }

    /// Serialization
    void visit(visitor &v) {
      scene_node::visit(v);
      v.visit(mesh_instances, atom_mesh_instances);
      v.visit(animation_instances, atom_animation_instances);
      v.visit(camera_instances, atom_camera_instances);
      v.visit(light_instances, atom_light_instances);
    }

    /// reset the scene.
    void reset() {
      mesh_instances.reset();
      animation_instances.reset();
      camera_instances.reset();
      light_instances.reset();
    }

    /// set up OpenGL state
    void begin_render(int vx, int vy, vec4_in clear_color=vec4(0.5f, 0.5f, 0.5f, 1.0f)) {
      /// set a viewport - includes whole window area
      glViewport(0, 0, vx, vy);

      /// clear the background to black
      glClearColor(clear_color.x(), clear_color.y(), clear_color.z(), clear_color.w());
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      /// allow Z buffer depth testing (closer objects are always drawn in front of far ones)
      glEnable(GL_DEPTH_TEST);

      GLint param;
      glGetIntegerv(GL_SAMPLE_BUFFERS, &param);
      if (param == 0) {
        /// if multisampling is disabled, we can't use GL_SAMPLE_COVERAGE (which I think is mean)
        /// Instead, allow alpha blend (transparency when alpha channel is 0)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      } else {
        /// if multisampling is enabled, use GL_SAMPLE_COVERAGE instead
        glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        glEnable(GL_SAMPLE_COVERAGE);
      }
    }

    static float max(float x, float y) {
      return x > y ? x : y;
    }

    /// scenes often arrive with no camera of lights
    void create_default_camera_and_lights() {
      /// default camera_instance
      if (camera_instances.size() == 0) {
        aabb bb = get_world_aabb();
        bb = bb.get_union(aabb(vec3(0, 0, 0), vec3(5, 5, 5)));
        scene_node *node = add_scene_node();
        camera_instance *cam = new camera_instance();
        float bb_size = length(bb.get_half_extent()) * 2.0f;
        float distance = max(bb.get_max().z(), bb_size) * 2;
        node->access_nodeToParent().translate(0, 0, distance);
        float f = distance * 2, n = f * 0.001f;
        cam->set_node(node);
        cam->set_perspective(0, 45, 1, n, f);
        camera_instances.push_back(cam);
      }

      /// default light instance
      if (light_instances.size() == 0) {
        scene_node *node = add_scene_node();
        light *_light = new light();
        light_instance *li = new light_instance();
        node->access_nodeToParent().translate(100, 100, 100);
        node->access_nodeToParent().rotateX(45);
        node->access_nodeToParent().rotateY(45);
        _light->set_color(vec4(1, 1, 1, 1));
        _light->set_kind(atom_directional);
        li->set_node(node);
        li->set_light(_light);
        light_instances.push_back(li);
      }

      if (!object_shader) {
        object_shader = new bump_shader();
        object_shader->init(false);
        skin_shader = new bump_shader();
        skin_shader->init(true);
      }
    }

    void play_all_anims(resource_dict &dict) {
      dynarray<resource*> anims;
      dict.find_all(anims, atom_animation);

      for (unsigned i = 0; i != anims.size(); ++i) {
        animation *anim = anims[i]->get_animation();
        if (anim) {
          play(anim, true);
        }
      }
    }

    scene_node *add_scene_node(scene_node *new_node = 0) {
      if (!new_node) {
        new_node = new scene_node();
      }
      scene_node::add_child(new_node);
      return new_node;
    }

    mesh_instance *add_mesh_instance(mesh_instance *inst=0) {
      mesh_instances.push_back(inst);
      return inst;
    }

    animation_instance *add_animation_instance(animation_instance *inst) {
      animation_instances.push_back(inst);
      return inst;
    }

    camera_instance *add_camera_instance(camera_instance *inst) {
      camera_instances.push_back(inst);
      return inst;
    }

    light_instance *add_light_instance(light_instance *inst) {
      light_instances.push_back(inst);
      return inst;
    }

    void delete_mesh_instance(mesh_instance *inst) {
      //mesh_instances.erase_by_value(inst);
    }

    void delete_animation_instance(animation_instance *inst) {
      //animation_instances.erase_by_value(inst);
    }

    void delete_camera_instance(camera_instance *inst) {
      //camera_instances.erase_by_value(inst);
    }

    void delete_light_instance(light_instance *inst) {
      //light_instances.erase_by_value(inst);
    }

    /// how many mesh instances do we have?
    int get_num_mesh_instances() {
      return (int)mesh_instances.size();
    }

    /// how many camera_instances do we have?
    int get_num_camera_instances() {
      return (int)camera_instances.size();
    }

    /// how many light_instances do we have?
    int get_num_light_instances() {
      return (int)light_instances.size();
    }

    scene_node *get_root_node() {
      return (scene_node*)this;
    }

    /// debugging aid to draw boxes around objects
    void set_render_aabbs(bool value) {
      render_aabbs = value;
    }

    /// debugging aid to draw debug lines
    void set_render_debug_lines(bool value) {
      render_debug_lines = value;
    }

    /// debugging aid to log vertices
    void set_dump_vertices(bool value) {
      dump_vertices = value;
    }

    /// access camera_instance information
    camera_instance *get_camera_instance(int index) {
      return camera_instances[index];
    }

    /// access mesh_instance information
    mesh_instance *get_mesh_instance(int index) {
      return (unsigned)index < mesh_instances.size() ? (mesh_instance*)mesh_instances[index] : (mesh_instance*)NULL;
    }

    /// access light_instance information
    light_instance *get_light_instance(int index) {
      return light_instances[index];
    }

    /// advance all the animation instances
    /// note that we want to update before rendering or doing physics and AI actions.
    void update(float delta_time) {
      #ifdef OCTET_BULLET
        world->stepSimulation(delta_time, 1, delta_time);
        btCollisionObjectArray &array = world->getCollisionObjectArray();
        for (int i = 0; i != array.size(); ++i) {
          btCollisionObject *co = array[i];
          scene_node *node = (scene_node *)co->getUserPointer();
          if (node) {
            mat4t &mat = node->access_nodeToParent();
            co->getWorldTransform().getOpenGLMatrix(mat.get());
            //printf("%d %f\n", i, mat.w().y());
          }
        }
      #endif

      for (int idx = 0; idx != animation_instances.size(); ++idx) {
        animation_instance *inst = animation_instances[idx];
        inst->update(delta_time);
      }

      for (int idx = 0; idx != mesh_instances.size(); ++idx) {
        mesh_instance *inst = mesh_instances[idx];
        inst->update(delta_time);
      }
    }

    /// render using specific shaders.
    /// call OpenGL to draw all the mesh instances (scene_node + mesh + material)
    void render(bump_shader &object_shader, bump_shader &skin_shader, camera_instance &cam, float aspect_ratio) {
      render_impl(object_shader, skin_shader, cam, aspect_ratio);
    }

    /// render using default shaders.
    void render(float aspect_ratio) {
      if (camera_instances.size() != 0) {
        camera_instance *cam = camera_instances[0];
        render_impl(*object_shader, *skin_shader, *cam, aspect_ratio);
      }
    }

    /// play an animation on another target (not the same one as in the collada file)
    void play(animation *anim, resource *target, bool is_looping) {
      animation_instance *inst = new animation_instance(anim, target, is_looping);
      animation_instances.push_back(inst);
    }

    /// play an animation with built-in targets (as in the collada file)
    void play(animation *anim, bool is_looping) {
      animation_instance *inst = new animation_instance(anim, NULL, is_looping);
      animation_instances.push_back(inst);
    }

    /// find a mesh instance for a node
    mesh_instance *get_first_mesh_instance(scene_node *node) {
      for (int i = 0; i != mesh_instances.size(); ++i) {
        mesh_instance *mi = mesh_instances[i];
        if (mi && mi->get_node() == node) {
          return mi;
        }
      }
      return NULL;
    }

    /// get the approximate size of the scene, not including lights or cameras
    aabb get_world_aabb() {
      aabb world_aabb;
      bool first = true;
      for (int i = 0; i != mesh_instances.size(); ++i) {
        mesh_instance *mi = mesh_instances[i];
        if (mi && mi->get_node()) {
          mat4t nodeToWorld = mi->get_node()->calcModelToWorld();
          aabb bb = mi->get_mesh()->get_aabb();
          bb = bb.get_transform(nodeToWorld);
          if (first) {
            world_aabb = bb;
            first = false;
          } else {
            world_aabb = world_aabb.get_union(bb);
          }
        }
      }
      return world_aabb;
    }

    struct cast_result {
      mesh_instance *mi;
      rational depth;
    };

    /// brute force & ignorance ray cast.
    /// return the mesh instance and location of hits.
    /// todo: build a kd tree for mesh instance bbs & mesh triangles
    void cast_ray(cast_result &result, const ray &the_ray) {
      result.mi = 0;
      result.depth = rational(0, 0);

      for (int i = 0; i != mesh_instances.size(); ++i) {
        mesh_instance *mi = mesh_instances[i];
        if (mi && mi->get_node()) {
          mat4t nodeToWorld = mi->get_node()->calcModelToWorld();
          mesh *mesh = mi->get_mesh();
          aabb bb = mesh->get_aabb();
          bb = bb.get_transform(nodeToWorld);
          if (the_ray.intersects(bb)) {
            mat4t worldToNode = nodeToWorld.inverse3x4();
            //ray model_ray = ray(vec3(0, 0, -1), vec3(0, 0, 2)); //the_ray.get_transform(worldToNode);
            ray model_ray = the_ray.get_transform(worldToNode);
            int indices[3] = {0};
            vec4 bary_numer(0, 0, 0, 0);
            float bary_denom;
            bool hit = mesh->ray_cast(model_ray, indices, bary_numer, bary_denom);
            if (hit) {
              rational depth(bary_numer.w() / bary_denom);
              result.depth = min(depth, result.depth);
            }
            //printf("hit=%d %s %f\n", hit, (bary_numer/bary_denom).toString(tmp, sizeof(tmp)), (float)result.depth);
          }
        }
      }
    }

    /// Debug rendering: add a new line in world space (old ones will be lost)
    void add_debug_line(const vec3 &start, const vec3 &end) {
      if (debug_line_buffer.size()) {
        debug_line_buffer[debug_in_ptr++ & debug_line_buffer.size()-1] = start;
        debug_line_buffer[debug_in_ptr++ & debug_line_buffer.size()-1] = end;
      }
    }
  };
}}

