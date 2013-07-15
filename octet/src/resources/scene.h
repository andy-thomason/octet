////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Scene Node heirachy
//

class scene : public scene_node {
  ///////////////////////////////////////////
  //
  // rendering information
  //

  // each of these is a set of (scene_node, mesh, material)
  dynarray<ref<mesh_instance>> mesh_instances;

  // animations playing at the moment
  dynarray<ref<animation_instance>> animation_instances;

  // cameras available
  dynarray<ref<camera_instance>> camera_instances;

  // lights available
  dynarray<ref<light_instance>> light_instances;

  // compiled set of active lights
  lighting lighting_set;

  int frame_number;

  /*void metadata(meta &m) {
    m.visit(modelToParent);
    m.visit(sids);
    m.visit(parentNodes);
    m.visit(mesh_instances);
    m.visit(animation_instances);
    m.visit(camera_instances);
    m.visit(light_instances);
  }*/

  void render_impl(bump_shader &object_shader, bump_shader &skin_shader, camera_instance &cam) {
    mat4t cameraToWorld = cam.get_node()->calcModelToWorld();

    mat4t worldToCamera;
    cameraToWorld.invertQuick(worldToCamera);
    lighting_set.compute(worldToCamera);
    cam.set_cameraToWorld(cameraToWorld);
    mat4t cameraToProjection = cam.get_cameraToProjection();

    for (unsigned mesh_index = 0; mesh_index != mesh_instances.size(); ++mesh_index) {
      mesh_instance *mi = mesh_instances[mesh_index];
      mesh_state *mesh = mi->get_mesh();
      skin *skn = mi->get_skin();
      skeleton *skel = mi->get_skeleton();
      bump_material *mat = mi->get_material();

      /*if (frame_number == 1) {
        printf("!! %d %s\n", i, modelToWorld.toString());
      }*/

      mat4t modelToWorld = mi->get_node()->calcModelToWorld();
      mat4t modelToCamera;
      mat4t modelToProjection;
      cam.get_matrices(modelToProjection, modelToCamera, modelToWorld);

      if (!skel || !skn) {
        // normal rendering for single matrix objects
        // build a projection matrix: model -> world -> camera_instance -> projection
        // the projection space is the cube -1 <= x/w, y/w, z/w <= 1
        mat->render(object_shader, modelToProjection, modelToCamera, lighting_set.data());
      } else {
        // multi-matrix rendering
        mat4t *transforms = skel->calc_transforms(modelToCamera, skn);
        int num_bones = skel->get_num_bones();
        mat->render_skinned(skin_shader, cameraToProjection, transforms, num_bones, lighting_set.data());
      }
      mesh->render();
    }
  }
public:
  RESOURCE_META(scene)

  // create an empty scene
  scene() {
    frame_number = 0;
  }

  void create_default_camera_and_lights() {
    // default camera_instance
    if (camera_instances.size() == 0) {
      mat4t m;
      m.loadIdentity();
      m.rotateX(90);
      m.translate(0.0, 1.0f, 1.5f);
      scene_node *node_ = add_root_node(m, 0);
      camera_instance *cam = new camera_instance();
      float n = 0.1f, f = 1000.0f;
      cam->set_params(node_, -n, n, -n, n, n, f, false);
      camera_instances.push_back(cam);
    }

    vec4 light_dir = vec4(-1, -1, -1, 0).normalize();
    vec4 light_ambient = vec4(0.3f, 0.3f, 0.3f, 1);
    vec4 light_diffuse = vec4(1, 1, 1, 1);
    vec4 light_specular = vec4(1, 1, 1, 1);
    lighting_set.add_light(vec4(0, 0, 0, 1), light_dir, light_ambient, light_diffuse, light_specular);
  }

  scene_node *add_root_node(const mat4t &modelToParent, atom_t sid) {
    scene_node *new_node = new scene_node(modelToParent, sid);
    scene_node::add_child(new_node);
    return new_node;
  }

  void add_mesh_instance(mesh_instance *inst) {
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
  int num_mesh_instances() {
    return (int)mesh_instances.size();
  }

  // how many camera_instances do we have?
  int num_camera_instances() {
    return (int)camera_instances.size();
  }

  // how many light_instances do we have?
  int num_light_instances() {
    return (int)light_instances.size();
  }

  scene_node *get_root_node() {
    return (scene_node*)this;
  }

  // access camera_instance information
  camera_instance *get_camera_instance(int index) {
    return camera_instances[index];
  }

  // advance all the animation instances
  void update(float delta_time) {
    for (int ai = 0; ai != animation_instances.size(); ++ai) {
      animation_instance *inst = animation_instances[ai];
      if (inst) {
        const animation *anim = inst->get_anim();
        for (int ch = 0; ch != anim->get_num_channels(); ++ch) {
          animation::chan_kind kind = anim->get_chan_kind(ch);
          if (kind == animation::chan_matrix) {
            mat4t m;
            anim->eval_chan(ch, inst->get_time(), &m, sizeof(m));
          }
        }

        // update the instance by the frame rate
        if (inst->update_time(delta_time)) {
          // instance is dead, stop playing
          animation_instances[ai] = NULL;
        }
      }
    }
  }

  // call OpenGL to draw all the mesh instances (scene_node + mesh + material)
  void render(bump_shader &object_shader, bump_shader &skin_shader, camera_instance &cam) {
    render_impl(object_shader, skin_shader, cam);
  }

  // play an animation on a mesh instance or other target
  void play(animation *anim, animation_target *target, bool is_looping) {
    animation_instance *inst = new animation_instance(anim, target, is_looping);
    animation_instances.push_back(inst);
  }

  mesh_instance *get_first_mesh_instance(scene_node *node_) {
    for (int i = 0; i != mesh_instances.size(); ++i) {
      if (mesh_instances[i]->get_node() == node_) {
        return mesh_instances[i];
      }
    }
    return NULL;
  }
};

