////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Animation target pure interface
//

// c++ note: this is a "pure" interface very much like a Java interface
// with no data and "pure" virtual functions (=0) which need to be defined in the implmenentation.
//
// C++ works best if multiple inheiritance only uses these kinds of interface otherwise
// we are into the world of "thunks" which gets messy fast!
//
namespace octet {
  class animation_target {
  public:
    // for ref<> containers
    virtual void add_ref() = 0;
    virtual void release() = 0;

    // called by the animation, script or RPC.
    virtual void set_value(atom_t sid, atom_t sub_target, atom_t component, float *value) = 0;
  };
}
