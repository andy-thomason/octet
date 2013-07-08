////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Scene Node heirachy
//

class scene {
  // array of relative transforms
  dynarray<mat4> modelToParent;

  // convert a string to an index
  dictionary<int> nodeNamesToNodes;

  // every node has a parent node except the roots with -1
  dynarray<int> parentNodes;

  // a skeleton is a set of nodes followed by -1. eg. skeletons 0 and 4: 0 1 2 -1 3 4 5 6 -1
  dynarray<int> skeletons;

  // for skinned meshes, we need a few more data
  dynarray<mesh_state::skin> skins;

  // each of these is a set of (node, mesh, material)
  dynarray<mesh_state::mesh_instance> mesh_instances;

  // meshes
  dynarray<mesh_state*> meshes;

  // materials
  dynarray<bump_material> materials;

  // set of lights
  lighting lighting_set;

  // set of lights
  dynarray<camera> cameras;

  int frame_number;

public:
  // create an empty scene
  scene() {
    frame_number = 0;
  }

  // how many nodes to we have?
  int num_nodes() {
    return (int)modelToParent.size();
  }

  // how many mesh instances do we have?
  int num_mesh_instances() {
    return (int)mesh_instances.size();
  }

  // how many cameras do we have?
  int num_cameras() {
    return (int)cameras.size();
  }

  int add_node(int parent, const mat4 &modelToParent_, const char *name) {
    int node_index = (int)modelToParent.size();
    modelToParent.push_back(modelToParent_);
    parentNodes.push_back(parent);
    nodeNamesToNodes[name] = node_index;
    return node_index;
  }

  // compute the node to world matrix for an individual node;
  mat4 calcModelToWorld(int node) {
    mat4 result = modelToParent[node];
    node = parentNodes[node];
    while (node != -1) {
      //result = modelToParent[node] * result;
      result = result * modelToParent[node];
      node = parentNodes[node];
    }
    return result;
  }

  void calcRelativeMatrices(mat4 *modelToCamera, int node, int num_nodes) {
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

  // access camera information
  camera &get_camera(int index) {
    return cameras[index];
  }

  // access the relative matrix for a specific node
  mat4 &node_to_parent(int node) {
    return modelToParent[node];
  }

  // get an index for a named node.
  int get_index(const char *name) {
    return nodeNamesToNodes[name];
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

  // get a mesh object if one exists for this node or NULL if not
  mesh_state *geometry(int index) {
    return meshes[index];
  }
  
  // load a scene from a collada file
  void make_collada_scene(collada_builder &builder, const char *name) {
    collada_builder::scene_state s = { modelToParent, nodeNamesToNodes, parentNodes, mesh_instances, skeletons, meshes, materials, cameras };

    builder.get_scene(name, s);

    // temp. lights
    vec4 light_dir = vec4(-1, -1, -1, 0).normalize();
    vec4 light_ambient = vec4(0.3f, 0.3f, 0.3f, 1);
    vec4 light_diffuse = vec4(1, 1, 1, 1);
    vec4 light_specular = vec4(1, 1, 1, 1);
    lighting_set.add_light(vec4(0, 0, 0, 1), light_dir, light_ambient, light_diffuse, light_specular);

    // default camera
    if (cameras.size() == 0) {
      mat4 m;
      m.loadIdentity();
      m.rotateX(90);
      m.translate(0.0, 1.0f, 1.5f);
      int node = add_node(-1, m, "default_cam");
      camera cam;
      float n = 0.1f, f = 1000.0f;
      cam.set_params(node, -n, n, -n, n, n, f, false);
      cameras.push_back(cam);
    }
  }

  // call OpenGL to draw all the mesh instances (node + mesh + material)
  void render(bump_shader &object_shader, bump_shader &skin_shader, int camera_index=0) {
    frame_number++;

    if (cameras.size() == 0) {
      return;
    }

    mat4 cameraToWorld;
    camera &cam = get_camera(camera_index);
    cameraToWorld = calcModelToWorld(cam.node());

    mat4 worldToCamera;
    cameraToWorld.invertQuick(worldToCamera);
    lighting_set.compute(worldToCamera);
    cam.set_cameraToWorld(cameraToWorld);

    for (int mesh_index = 0; mesh_index != (int)mesh_instances.size(); ++mesh_index) {
      mesh_state::mesh_instance &m = mesh_instances[mesh_index];
      mesh_state &mesh_st = *meshes[m.mesh];
      bump_material &mat = materials[m.material];
      mesh_state::skin *skin = mesh_st.get_skin();

      /*if (frame_number == 1) {
        printf("!! %d %s\n", i, modelToWorld.toString());
      }*/

      if (m.skeleton == -1 || !skin) {
        // normal rendering for single matrix objects
        // build a projection matrix: model -> world -> camera -> projection
        // the projection space is the cube -1 <= x/w, y/w, z/w <= 1
        mat4 modelToWorld = calcModelToWorld(m.node);
        mat4 modelToCamera;
        mat4 modelToProjection;
        cam.get_matrices(modelToProjection, modelToCamera, modelToWorld);
        mat.render(object_shader, modelToProjection, modelToCamera, lighting_set.data());
      } else {
        // multi-matrix rendering for characters
        mat4 modelToCamera[32];
        mat4 modelToProjection;
        int si = m.skeleton;
        int di = 0;
        mat4 cameraToProjection = cam.get_cameraToProjection();
        while (skeletons[si] != -1 && di < 32) {
          int src_node = skeletons[si];
          mat4 modelToWorld = calcModelToWorld(src_node);
          mat4 bindToModel = skin->bindToModel[di];
          mat4 meshToBind = skin->modelToBind;
          modelToCamera[di] = meshToBind * bindToModel * modelToWorld * worldToCamera;
          if (frame_number == 1) {
            const char *name = node_name(src_node);
            printf("mi=%d node=%d name=%s mx=%s\n", mesh_index, src_node, name, modelToCamera[di].toString());
          }
          si++;
          di++;
        }
        //modelToCamera[1] = modelToCamera[0];
        mat.render_skinned(skin_shader, cameraToProjection, modelToCamera, di, lighting_set.data());
      }
      mesh_st.render();
    }
  }
};

