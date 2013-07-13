////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Scene Node heirachy
//

class scene : public resource {
  ///////////////////////////////////////////
  //
  // node heirachy
  //

  // array of relative transforms (indexed by node index)
  dynarray<mat4t> modelToParent;

  // a sid is a local name for a node which may be repeated (indexed by node index)
  // eg. the bones for multiple skeletons
  dynarray<atom_t> sids;

  // every node has a parent node except the roots with -1 (indexed by node index)
  dynarray<int> parentNodes;

  // convert a string to a node index
  dictionary<int> nodeNamesToNodes;

  ///////////////////////////////////////////
  //
  // rendering information
  //

  // each of these is a set of (node, mesh, material)
  dynarray<ref<mesh_instance>> mesh_instances;

  // animations playing at the moment
  dynarray<ref<animation_instance>> animation_instances;

  // animations playing at the moment
  dynarray<ref<camera_instance>> camera_instances;

  // animations playing at the moment
  dynarray<ref<light_instance>> light_instances;

  // compiled set of lights
  lighting lighting_set;

  int frame_number;

  void render_impl(bump_shader &object_shader, bump_shader &skin_shader, camera_instance &cam) {
    mat4t cameraToWorld = calcModelToWorld(cam.node());

    mat4t worldToCamera;
    cameraToWorld.invertQuick(worldToCamera);
    lighting_set.compute(worldToCamera);
    cam.set_cameraToWorld(cameraToWorld);

    for (int mesh_index = 0; mesh_index != (int)mesh_instances.size(); ++mesh_index) {
      mesh_instance *mi = mesh_instances[mesh_index];
      const mesh_state *mesh = mi->get_mesh();
      const skin *skn = mi->get_skin();
      const skeleton *skel = mi->get_skeleton();
      const bump_material *mat = mi->get_material();

      /*if (frame_number == 1) {
        printf("!! %d %s\n", i, modelToWorld.toString());
      }*/

      if (!skel || !skn) {
        // normal rendering for single matrix objects
        // build a projection matrix: model -> world -> camera_instance -> projection
        // the projection space is the cube -1 <= x/w, y/w, z/w <= 1
        mat4t modelToWorld = calcModelToWorld(mi->get_node());
        mat4t modelToCamera;
        mat4t modelToProjection;
        cam.get_matrices(modelToProjection, modelToCamera, modelToWorld);
        mat->render(object_shader, modelToProjection, modelToCamera, lighting_set.data());
      } else {
        // multi-matrix rendering for characters
        mat4t modelToCamera[32];
        mat4t modelToProjection;
        int si = 0;
        int di = 0;
        mat4t cameraToProjection = cam.get_cameraToProjection();
        int num_bones = skel->get_num_bones();
        for (int si = 0; si != num_bones && di < 32; ++si) {
          int src_node = skel->get_bone(si);
          mat4t modelToWorld = calcModelToWorld(src_node);
          mat4t bindToModel = skn->get_bindToModel(di);
          mat4t meshToBind = skn->get_modelToBind();
          modelToCamera[di] = meshToBind * bindToModel * modelToWorld * worldToCamera;
          if (frame_number == 1) {
            const char *name = node_name(src_node);
            mat4t z = bindToModel * modelToWorld;
            //printf("mi=%d node=%d name=%s mx=%s\n", mesh_index, src_node, name, modelToCamera[di].toString());
            printf("mi=%d node=%d name=%s z=%s\n", mesh_index, src_node, name, z.toString());
          }
          si++;
          di++;
        }
        mat->render_skinned(skin_shader, cameraToProjection, modelToCamera, di, lighting_set.data());
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

  int add_node(const mat4t &modelToParent, int parent, atom_t sid) {
    this->modelToParent.push_back(modelToParent);
    this->parentNodes.push_back(parent);
    this->sids.push_back(sid);
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

  // how many nodes do we have?
  int num_nodes() {
    return (int)modelToParent.size();
  }

  // how many mesh instances do we have?
  int num_mesh_instances() {
    return (int)mesh_instances.size();
  }

  // how many camera_instances do we have?
  int num_camera_instances() {
    return (int)camera_instances.size();
  }

  // 
  int add_node(int parent, const mat4t &modelToParent_, const char *id, const char *sid) {
    int node_index = (int)modelToParent.size();
    modelToParent.push_back(modelToParent_);
    parentNodes.push_back(parent);
    sids.push_back(resources::get_atom(sid));
    nodeNamesToNodes[id] = node_index;
    return node_index;
  }

  // compute the node to world matrix for an individual node;
  mat4t calcModelToWorld(int node) {
    mat4t result = modelToParent[node];
    node = parentNodes[node];
    while (node != -1) {
      //result = modelToParent[node] * result;
      result = result * modelToParent[node];
      node = parentNodes[node];
    }
    return result;
  }

  void calcRelativeMatrices(mat4t *modelToCamera, int node, int num_nodes) {
    /*for (int i = 0; i != parentNodes.size(); ++i) {
      printf("%d ", parentNodes[i]);
    }
    printf(" [%d]\n", node);*/
    for (int rel_node = 1; rel_node < num_nodes; ++rel_node) {
      int abs_node = node + rel_node;
      int abs_parent = parentNodes[abs_node];
      int rel_parent = abs_parent - node;
      if (rel_parent >= 0 && rel_parent < num_nodes) {
        // use the parent transform to make a new matrix from an earlier one.
        modelToCamera[rel_node] = modelToParent[abs_node] * modelToCamera[rel_parent];
      } else {
        // rel_parent out of range: play safe and return the original matrix
        modelToCamera[rel_node] = modelToCamera[0];
      }
    }
  }

  // access camera_instance information
  camera_instance *get_camera_instance(int index) {
    return camera_instances[index];
  }

  // access the relative matrix for a specific node
  mat4t &get_modelToParent(int node) {
    return modelToParent[node];
  }

  int get_num_nodes() {
    return modelToParent.size();
  }

  int get_parent(int node_index) const {
    return parentNodes[node_index];
  }

  // get an index for a named node.
  int get_node_index(const char *name) {
    if (nodeNamesToNodes.contains(name)) {
      return nodeNamesToNodes[name];
    } else {
      return -1;
    }
  }

  // convert index to name (slow)
  const char *node_name(int index) {
    const char *name = "??";
    for (int i = 0; i != nodeNamesToNodes.num_indices(); ++i) {
      if (nodeNamesToNodes.value(i) == index) {
        name = nodeNamesToNodes.key(i);
      }
    }
    return name;
  }

  // load a scene from a collada file
  /*void make_collada_scene(collada_builder &builder, const char *name) {
    collada_builder::scene_state s = { modelToParent, sids, nodeNamesToNodes, parentNodes, mesh_instances, skeletons, meshes, materials, camera_instances };

    builder.get_scene(name, s);

    // temp. lights
    vec4 light_dir = vec4(-1, -1, -1, 0).normalize();
    vec4 light_ambient = vec4(0.3f, 0.3f, 0.3f, 1);
    vec4 light_diffuse = vec4(1, 1, 1, 1);
    vec4 light_specular = vec4(1, 1, 1, 1);
    lighting_set.add_light(vec4(0, 0, 0, 1), light_dir, light_ambient, light_diffuse, light_specular);

    // default camera_instance
    if (camera_instances.size() == 0) {
      mat4t m;
      m.loadIdentity();
      m.rotateX(90);
      m.translate(0.0, 1.0f, 1.5f);
      int node = add_node(-1, m, "default_cam", "");
      camera_instance *cam = new camera_instance();
      float n = 0.1f, f = 1000.0f;
      cam->set_params(node, -n, n, -n, n, n, f, false);
      camera_instances.push_back(cam);
    }
  }*/

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

  // call OpenGL to draw all the mesh instances (node + mesh + material)
  void render(bump_shader &object_shader, bump_shader &skin_shader, camera_instance &cam) {
    render_impl(object_shader, skin_shader, cam);
  }

  // play an animation on a node
  void play(animation *anim, int node_index, bool is_looping) {
    animation_instance *inst = new animation_instance(anim, node_index, is_looping);
    animation_instances.push_back(inst);
  }
};

