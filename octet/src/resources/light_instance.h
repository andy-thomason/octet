////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Light in a scene. Note that for shadow maps, lights are cameras.
//

class light_instance : public camera_instance {
public:
  RESOURCE_META(light_instance)
  light_instance() {
  }
};

