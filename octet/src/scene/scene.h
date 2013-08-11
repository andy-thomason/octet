////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Scene Node heirachy
//

namespace octet {
  class scene : public scene_node {
    ///////////////////////////////////////////
    //
    // rendering information
    //

    // each of these is a set of (scene_node, mesh, material)
    dynarray<ref<mesh_instance> > mesh_instances;

    // animations playing at the moment
    dynarray<ref<animation_instance> > animation_instances;

    // cameras available
    dynarray<ref<camera_instance> > camera_instances;

    // lights available
    dynarray<ref<light_instance> > light_instances;

    // set this to draw bounding boxes
    bool render_aabbs;
    bool render_debug_lines;
    ref<material> debug_material;
    dynarray<vec3> debug_line_buffer;
    unsigned debug_in_ptr;

    // derived light information
    enum { max_lights = 4, light_size = 4 };
    int num_light_uniforms;
    int num_lights;
    vec4 light_uniforms[1 + max_lights * light_size ];

    int frame_number;

    void draw_aabb(const aabb &bb) {
      vec3 pos[8];
      for (int i = 0; i != 8; ++i) {
        vec3 center = bb.get_center();
        vec3 half = bb.get_half_extent();
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

      // render immediate data (this is inefficient!)
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
        atom_t kind = li->get_kind();
        if (kind == atom_ambient) {
          ambient += li->get_color();
          num_ambient++;
        } else {
          li->get_fragment_uniforms(&light_uniforms[1+num_lights*light_size], worldToCamera);
          num_lights++;
        }
      }
      if (num_ambient == 0) {
        ambient = vec4(0.5f, 0.5f, 0.5f, 1);
      }
      num_light_uniforms = 1 + num_lights * light_size;
    }

    void render_impl(bump_shader &object_shader, bump_shader &skin_shader, camera_instance &cam, float aspect_ratio) {
      mat4t cameraToWorld = cam.get_node()->calcModelToWorld();

      mat4t worldToCamera;
      cameraToWorld.invertQuick(worldToCamera);

      calc_lighting(worldToCamera);

      cam.set_cameraToWorld(cameraToWorld, aspect_ratio);
      mat4t cameraToProjection = cam.get_cameraToProjection();

      {
        // debug draw the AABBs of the mesh instances in the world.
        // draw in world space
        mat4t worldToCamera;
        mat4t worldToProjection;
        mat4t worldToWorld;
        worldToWorld.loadIdentity();
        cam.get_matrices(worldToProjection, worldToCamera, worldToWorld);
        debug_material->render(object_shader, worldToProjection, worldToCamera, light_uniforms, num_light_uniforms, num_lights);

        if (render_debug_lines) {
          glBindBuffer(GL_ARRAY_BUFFER, 0);
          glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 0, (void*)debug_line_buffer.data() );
          glEnableVertexAttribArray(attribute_pos);
    
          glDrawArrays(GL_LINES, 0, debug_line_buffer.size());
          glDisableVertexAttribArray(attribute_pos);
        }

        if (render_aabbs) {
          for (unsigned mesh_index = 0; mesh_index != mesh_instances.size(); ++mesh_index) {
            mesh_instance *mi = mesh_instances[mesh_index];
            aabb bb = mi->get_mesh()->get_aabb();
            bb = bb.get_transform(mi->get_node()->calcModelToWorld());
            draw_aabb(bb);
          }
        }
      }

      for (unsigned mesh_index = 0; mesh_index != mesh_instances.size(); ++mesh_index) {
        mesh_instance *mi = mesh_instances[mesh_index];
        mesh *msh = mi->get_mesh();
        skin *skn = msh->get_skin();
        skeleton *skel = mi->get_skeleton();
        material *mat = mi->get_material();

        mat4t modelToWorld = mi->get_node()->calcModelToWorld();
        mat4t modelToCamera;
        mat4t modelToProjection;
        cam.get_matrices(modelToProjection, modelToCamera, modelToWorld);

        if (!skel || !skn) {
          // normal rendering for single matrix objects
          // build a projection matrix: model -> world -> camera_instance -> projection
          // the projection space is the cube -1 <= x/w, y/w, z/w <= 1
          mat->render(object_shader, modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);
        } else {
          // multi-matrix rendering
          mat4t *transforms = skel->calc_transforms(modelToCamera, skn);
          int num_bones = skel->get_num_bones();
          assert(num_bones < 64);
          mat->render_skinned(skin_shader, cameraToProjection, transforms, num_bones, light_uniforms, num_light_uniforms, num_lights);
        }
        msh->render();
      }
      frame_number++;
    }
  public:
    RESOURCE_META(scene)

    // create an empty scene
    scene() {
      frame_number = 0;
      num_light_uniforms = 0;
      num_lights = 0;
      render_aabbs = true;
      render_debug_lines = true;
      debug_material = new material(vec4(1, 0, 0, 1));
      debug_line_buffer.resize(256);
      assert(is_power_of_two(debug_line_buffer.size()));
      memset(&debug_line_buffer[0], 0, debug_line_buffer.size() * sizeof(debug_line_buffer[0]));
      debug_in_ptr = 0;
    }

    void visit(visitor &v) {
      scene_node::visit(v);
      v.visit(mesh_instances, atom_mesh_instances);
      v.visit(animation_instances, atom_animation_instances);
      v.visit(camera_instances, atom_camera_instances);
      v.visit(light_instances, atom_light_instances);
    }

    static float max(float x, float y) {
      return x > y ? x : y;
    }

    // scene often arrive with no camera of lights
    void create_default_camera_and_lights() {
      // default camera_instance
      if (camera_instances.size() == 0) {
        aabb bb = get_world_aabb();
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

      // default light instance
      if (light_instances.size() == 0) {
        scene_node *node = add_scene_node();
        light_instance *li = new light_instance();
        node->access_nodeToParent().translate(100, 100, 100);
        node->access_nodeToParent().rotateX(45);
        node->access_nodeToParent().rotateY(45);
        li->set_color(vec4(1, 1, 1, 1));
        li->set_kind(atom_directional);
        li->set_node(node);
        light_instances.push_back(li);
      }
    }

    void play_all_anims(resources &dict) {
      dynarray<resource*> anims;
      dict.find_all(anims, atom_animation);

      for (unsigned i = 0; i != anims.size(); ++i) {
        animation *anim = anims[i]->get_animation();
        if (anim) {
          play(anim, true);
        }
      }
    }

    scene_node *add_scene_node() {
      scene_node *new_node = new scene_node();
      scene_node::add_child(new_node);
      return new_node;
    }

    void add_mesh_instance(mesh_instance *inst=0) {
      mesh_instances.push_back(inst);
    }

    void add_animation_instance(animation_instance *inst) {
      animation_instances.push_back(inst);
    }

    void add_camera_instance(camera_instance *inst) {
      camera_instances.push_back(inst);
    }

    void add_light_instance(light_instance *inst) {
      light_instances.push_back(inst);
    }

    // how many mesh instances do we have?
    int get_num_mesh_instances() {
      return (int)mesh_instances.size();
    }

    // how many camera_instances do we have?
    int get_num_camera_instances() {
      return (int)camera_instances.size();
    }

    // how many light_instances do we have?
    int get_num_light_instances() {
      return (int)light_instances.size();
    }

    scene_node *get_root_node() {
      return (scene_node*)this;
    }

    void set_render_aabbs(bool value) {
      render_aabbs = value;
    }

    // access camera_instance information
    camera_instance *get_camera_instance(int index) {
      return camera_instances[index];
    }

    // advance all the animation instances
    // note that we want to update before rendering or doing physics and AI actions.
    void update(float delta_time) {
      for (int idx = 0; idx != animation_instances.size(); ++idx) {
        animation_instance *inst = animation_instances[idx];
        inst->update(delta_time);
      }

      for (int idx = 0; idx != mesh_instances.size(); ++idx) {
        mesh_instance *inst = mesh_instances[idx];
        inst->update(delta_time);
      }
    }

    // call OpenGL to draw all the mesh instances (scene_node + mesh + material)
    void render(bump_shader &object_shader, bump_shader &skin_shader, camera_instance &cam, float aspect_ratio) {
      render_impl(object_shader, skin_shader, cam, aspect_ratio);
    }

    // play an animation on another target (not the same one as in the collada file)
    void play(animation *anim, animation_target *target, bool is_looping) {
      animation_instance *inst = new animation_instance(anim, target, is_looping);
      animation_instances.push_back(inst);
    }

    // play an animation with built-in targets (as in the collada file)
    void play(animation *anim, bool is_looping) {
      animation_instance *inst = new animation_instance(anim, NULL, is_looping);
      animation_instances.push_back(inst);
    }

    // find a mesh instance for a node
    mesh_instance *get_first_mesh_instance(scene_node *node) {
      for (int i = 0; i != mesh_instances.size(); ++i) {
        mesh_instance *mi = mesh_instances[i];
        if (mi && mi->get_node() == node) {
          return mi;
        }
      }
      return NULL;
    }

    // get the approximate size of the scene, not including lights or cameras
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

    // brute force & ignorance ray cast.
    // return the mesh instance and location of hits.
    // todo: build a kd tree for mesh instance bbs & mesh triangles
    void cast_ray(cast_result &result, const ray &the_ray) {
      result.mi = 0;
      result.depth = rational(0, 0);

      for (int i = 0; i != mesh_instances.size(); ++i) {
        mesh_instance *mi = mesh_instances[i];
        if (mi && mi->get_node()) {
          mat4t nodeToWorld = mi->get_node()->calcModelToWorld();
          aabb bb = mi->get_mesh()->get_aabb();
          bb = bb.get_transform(nodeToWorld);
          if (the_ray.intersects(bb)) {
            //rational depth = the_ray.intersection(bb);
            result.mi = mi;
            //static int i;
            //printf("%d hit\n", i++);
          }
        }
      }
    }

    // add a new line in world space (old ones will be lost)
    void add_debug_line(const vec3 &start, const vec3 &end) {
      if (debug_line_buffer.size()) {
        debug_line_buffer[debug_in_ptr++ & debug_line_buffer.size()-1] = start;
        debug_line_buffer[debug_in_ptr++ & debug_line_buffer.size()-1] = end;
      }
    }
  };
}

