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

  int current_camera;
  int frame_number;

public:
  // create an empty scene
  scene() {
    frame_number = 0;
    current_camera = 0;
  }

  // how many nodes to we have?
  int num_nodes() {
    return (int)modelToParent.size();
  }

  // how many mesh instances to we have?
  int num_mesh_instances() {
    return (int)mesh_instances.size();
  }

  // compute the node to world matrix for an individual node;
  mat4 calcModelToWorld(int node) {
    mat4 result = modelToParent[node];
    node = parentNodes[node];
    while (node != -1) {
      result = modelToParent[node] * result;
      node = parentNodes[node];
    }
    return result;
  }

  // access the relative matrix for a specific node
  mat4 &node_to_parent(int node) {
    return modelToParent[node];
  }

  // get an index for a named node.
  int index(const char *name) {
    return nodeNamesToNodes[name];
  }

  // get a mesh object if one exists for this node or NULL if not
  mesh_state *geometry(int index) {
    return meshes[index];
  }
  
  // load a scene from a collada file
  void make_collada_scene(collada_builder &builder, const char *name) {
    builder.get_scene(name, modelToParent, nodeNamesToNodes, parentNodes, mesh_instances, meshes, materials);

    // temp. lights
    vec4 light_dir = vec4(1, 1, 1, 0).normalize();
    vec4 light_ambient = vec4(0.3f, 0.3f, 0.3f, 1);
    vec4 light_diffuse = vec4(1, 1, 1, 1);
    vec4 light_specular = vec4(1, 1, 1, 1);
    lighting_set.add_light(vec4(0, 0, 0, 1), light_dir, light_ambient, light_diffuse, light_specular);

    // temp. camera
    camera cam;
    float n = 0.1f, f = 1000.0f;
    cam.set_params(0, -n, n, -n, n, n, f, false);
    cameras.push_back(cam);
  }

  // call OpenGL to draw all the mesh instances (node + mesh + material)
  void render(bump_shader &shader, const mat4 &cameraToWorld) {
    frame_number++;

    mat4 worldToCamera;
    cameraToWorld.invertQuick(worldToCamera);
    lighting_set.compute(worldToCamera);
    camera &cam = cameras[0];
    cam.set_cameraToWorld(cameraToWorld);

    for (int i = 0; i != (int)mesh_instances.size(); ++i) {
      mesh_state::mesh_instance &m = mesh_instances[i];
      mat4 modelToWorld = calcModelToWorld(m.node);
      mesh_state &mesh_st = *meshes[m.mesh];
      bump_material &mat = materials[m.material];

      if (frame_number == 1) {
        printf("!! %d %s\n", i, modelToWorld.toString());
      }

      // build a projection matrix: model -> world -> camera -> projection
      // the projection space is the cube -1 <= x/w, y/w, z/w <= 1
      mat4 modelToCamera;
      mat4 modelToProjection;
      cam.get_matrices(modelToProjection, modelToCamera, modelToWorld);
      //printf("1 %s\n", modelToProjection.toString());
      //modelToProjection = mat4::build_camera_matrices(modelToCamera, worldToCamera, modelToWorld, cameraToWorld);
      //printf("2 %s\n", modelToProjection.toString());

      mat.render(shader, modelToProjection, modelToCamera, lighting_set.data());

      mesh_st.render();
    }
  }
};

