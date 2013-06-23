////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Scene Node heirachy
//

class nodes {
  // array of relative transforms
  dynarray<mat4> modelToParent;

  // convert a string to an index
  dictionary<int> nodeNamesToNodes;

  // who's the daddy?
  dynarray<int> parentNodes;

public:
  // create an empty set of nodes
  nodes() {
  }

  // add a node to the collection
  int add_node(const char *name, const char *parent, const mat4 &modelToParentMatrix) {
    int index = (int)modelToParent.size();
    int parent_index = parent ? nodeNamesToNodes[parent] : -1;
    nodeNamesToNodes[name] = index;
    modelToParent[index] = modelToParentMatrix;
    return index;
  }

  // how many nodes to we have?
  int num_nodes() {
    return (int)modelToParent.size();
  }

  // compute the node to world matrix for an individual node;
  mat4 get_node_to_world(int node) {
    mat4 modelToWorld = modelToParent[node];
    node = parentNodes[node];
    while (node != -1) {
      modelToWorld.multMatrix(modelToParent[node]);
      node = parentNodes[node];
    }
    return modelToWorld;
  }

  // access the relative matrix for a specific node
  mat4 &node_to_parent(int node) {
    return modelToParent[node];
  }

  // get an index for a named node.
  int get_index(const char *name) {
    return nodeNamesToNodes[name];
  }
};

