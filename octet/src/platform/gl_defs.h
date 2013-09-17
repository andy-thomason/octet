////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// OpenGL ES2/3 emulation for windows.
//

// condensed version of Khronos gl3.h
typedef void GLvoid;
typedef char GLchar;
typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef float GLclampf;
typedef int GLfixed;
typedef intptr_t GLintptr;
typedef size_t GLsizeiptr;

/* OpenGL ES 3.0 */

typedef unsigned short   GLhalf;
typedef long long  GLint64;
typedef unsigned long long GLuint64;
typedef struct __GLsync *GLsync;

// use an enum instead of #defines as this is the 21st century!
enum {
  GL_ES_VERSION_3_0 = 1,
  GL_ES_VERSION_2_0 = 1,

#ifdef SKIP_OPENGL_1
  /* OpenGL ES 2.0 */

  /* ClearBufferMask */
  GL_DEPTH_BUFFER_BIT = 0x00000100,
  GL_STENCIL_BUFFER_BIT = 0x00000400,
  GL_COLOR_BUFFER_BIT = 0x00004000,

  /* Boolean */
  GL_FALSE = 0,
  GL_TRUE = 1,

  /* BeginMode */
  GL_POINTS = 0x0000,
  GL_LINES = 0x0001,
  GL_LINE_LOOP = 0x0002,
  GL_LINE_STRIP = 0x0003,
  GL_TRIANGLES = 0x0004,
  GL_TRIANGLE_STRIP = 0x0005,
  GL_TRIANGLE_FAN = 0x0006,

  /* BlendingFactorDest */
  GL_ZERO = 0,
  GL_ONE = 1,
  GL_SRC_COLOR = 0x0300,
  GL_ONE_MINUS_SRC_COLOR = 0x0301,
  GL_SRC_ALPHA = 0x0302,
  GL_ONE_MINUS_SRC_ALPHA = 0x0303,
  GL_DST_ALPHA = 0x0304,
  GL_ONE_MINUS_DST_ALPHA = 0x0305,

  /* BlendingFactorSrc */
  /*      GL_ZERO */
  /*      GL_ONE */
  GL_DST_COLOR = 0x0306,
  GL_ONE_MINUS_DST_COLOR = 0x0307,
  GL_SRC_ALPHA_SATURATE = 0x0308,
  /*      GL_SRC_ALPHA */
  /*      GL_ONE_MINUS_SRC_ALPHA */
  /*      GL_DST_ALPHA */
  /*      GL_ONE_MINUS_DST_ALPHA */
#endif

  /* BlendEquationSeparate */
  GL_FUNC_ADD = 0x8006,
  GL_BLEND_EQUATION = 0x8009,
  GL_BLEND_EQUATION_RGB = 0x8009,    /* same as BLEND_EQUATION */
  GL_BLEND_EQUATION_ALPHA = 0x883D,

  /* BlendSubtract */
  GL_FUNC_SUBTRACT = 0x800A,
  GL_FUNC_REVERSE_SUBTRACT = 0x800B,

  /* Separate Blend Functions */
  GL_BLEND_DST_RGB = 0x80C8,
  GL_BLEND_SRC_RGB = 0x80C9,
  GL_BLEND_DST_ALPHA = 0x80CA,
  GL_BLEND_SRC_ALPHA = 0x80CB,
  GL_CONSTANT_COLOR = 0x8001,
  GL_ONE_MINUS_CONSTANT_COLOR = 0x8002,
  GL_CONSTANT_ALPHA = 0x8003,
  GL_ONE_MINUS_CONSTANT_ALPHA = 0x8004,
  GL_BLEND_COLOR = 0x8005,

  /* Buffer Objects */
  GL_ARRAY_BUFFER = 0x8892,
  GL_ELEMENT_ARRAY_BUFFER = 0x8893,
  GL_ARRAY_BUFFER_BINDING = 0x8894,
  GL_ELEMENT_ARRAY_BUFFER_BINDING = 0x8895,

  GL_STREAM_DRAW = 0x88E0,
  GL_STATIC_DRAW = 0x88E4,
  GL_DYNAMIC_DRAW = 0x88E8,

  GL_BUFFER_SIZE = 0x8764,
  GL_BUFFER_USAGE = 0x8765,

  GL_CURRENT_VERTEX_ATTRIB = 0x8626,

#ifdef SKIP_OPENGL_1
  /* CullFaceMode */
  GL_FRONT = 0x0404,
  GL_BACK = 0x0405,
  GL_FRONT_AND_BACK = 0x0408,

  /* DepthFunction */
  /*      GL_NEVER */
  /*      GL_LESS */
  /*      GL_EQUAL */
  /*      GL_LEQUAL */
  /*      GL_GREATER */
  /*      GL_NOTEQUAL */
  /*      GL_GEQUAL */
  /*      GL_ALWAYS */

  /* EnableCap */
  GL_TEXTURE_2D = 0x0DE1,
  GL_CULL_FACE = 0x0B44,
  GL_BLEND = 0x0BE2,
  GL_DITHER = 0x0BD0,
  GL_STENCIL_TEST = 0x0B90,
  GL_DEPTH_TEST = 0x0B71,
  GL_SCISSOR_TEST = 0x0C11,
  GL_POLYGON_OFFSET_FILL = 0x8037,
#endif
  GL_SAMPLE_ALPHA_TO_COVERAGE = 0x809E,
  GL_SAMPLE_COVERAGE = 0x80A0,

#ifdef SKIP_OPENGL_1
  /* ErrorCode */
  GL_NO_ERROR = 0,
  GL_INVALID_ENUM = 0x0500,
  GL_INVALID_VALUE = 0x0501,
  GL_INVALID_OPERATION = 0x0502,
  GL_OUT_OF_MEMORY = 0x0505,

  /* FrontFaceDirection */
  GL_CW = 0x0900,
  GL_CCW = 0x0901,

  /* GetPName */
  GL_LINE_WIDTH = 0x0B21,
  GL_ALIASED_POINT_SIZE_RANGE = 0x846D,
  GL_ALIASED_LINE_WIDTH_RANGE = 0x846E,
  GL_CULL_FACE_MODE = 0x0B45,
  GL_FRONT_FACE = 0x0B46,
  GL_DEPTH_RANGE = 0x0B70,
  GL_DEPTH_WRITEMASK = 0x0B72,
  GL_DEPTH_CLEAR_VALUE = 0x0B73,
  GL_DEPTH_FUNC = 0x0B74,
  GL_STENCIL_CLEAR_VALUE = 0x0B91,
  GL_STENCIL_FUNC = 0x0B92,
  GL_STENCIL_FAIL = 0x0B94,
  GL_STENCIL_PASS_DEPTH_FAIL = 0x0B95,
  GL_STENCIL_PASS_DEPTH_PASS = 0x0B96,
  GL_STENCIL_REF = 0x0B97,
  GL_STENCIL_VALUE_MASK = 0x0B93,
  GL_STENCIL_WRITEMASK = 0x0B98,
  GL_STENCIL_BACK_FUNC = 0x8800,
  GL_STENCIL_BACK_FAIL = 0x8801,
  GL_STENCIL_BACK_PASS_DEPTH_FAIL = 0x8802,
  GL_STENCIL_BACK_PASS_DEPTH_PASS = 0x8803,
  GL_STENCIL_BACK_REF = 0x8CA3,
  GL_STENCIL_BACK_VALUE_MASK = 0x8CA4,
  GL_STENCIL_BACK_WRITEMASK = 0x8CA5,
  GL_VIEWPORT = 0x0BA2,
  GL_SCISSOR_BOX = 0x0C10,
  /*      GL_SCISSOR_TEST */
  GL_COLOR_CLEAR_VALUE = 0x0C22,
  GL_COLOR_WRITEMASK = 0x0C23,
  GL_UNPACK_ALIGNMENT = 0x0CF5,
  GL_PACK_ALIGNMENT = 0x0D05,
  GL_MAX_TEXTURE_SIZE = 0x0D33,
  GL_MAX_VIEWPORT_DIMS = 0x0D3A,
  GL_SUBPIXEL_BITS = 0x0D50,
  GL_RED_BITS = 0x0D52,
  GL_GREEN_BITS = 0x0D53,
  GL_BLUE_BITS = 0x0D54,
  GL_ALPHA_BITS = 0x0D55,
  GL_DEPTH_BITS = 0x0D56,
  GL_STENCIL_BITS = 0x0D57,
  GL_POLYGON_OFFSET_UNITS = 0x2A00,
  /*      GL_POLYGON_OFFSET_FILL */
  GL_POLYGON_OFFSET_FACTOR = 0x8038,
  GL_TEXTURE_BINDING_2D = 0x8069,
#endif
  GL_SAMPLE_BUFFERS = 0x80A8,
  GL_SAMPLES = 0x80A9,
  GL_SAMPLE_COVERAGE_VALUE = 0x80AA,
  GL_SAMPLE_COVERAGE_INVERT = 0x80AB,
#ifdef SKIP_OPENGL_1

  /* GetTextureParameter */
  /*      GL_TEXTURE_MAG_FILTER */
  /*      GL_TEXTURE_MIN_FILTER */
  /*      GL_TEXTURE_WRAP_S */
  /*      GL_TEXTURE_WRAP_T */

  GL_NUM_COMPRESSED_TEXTURE_FORMATS = 0x86A2,
  GL_COMPRESSED_TEXTURE_FORMATS = 0x86A3,

  /* HintMode */
  GL_DONT_CARE = 0x1100,
  GL_FASTEST = 0x1101,
  GL_NICEST = 0x1102,

  /* HintTarget */
  GL_GENERATE_MIPMAP_HINT = 0x8192,

  /* DataType */
  GL_BYTE = 0x1400,
  GL_UNSIGNED_BYTE = 0x1401,
  GL_SHORT = 0x1402,
  GL_UNSIGNED_SHORT = 0x1403,
  GL_INT = 0x1404,
  GL_UNSIGNED_INT = 0x1405,
  GL_FLOAT = 0x1406,
  GL_FIXED = 0x140C,

  /* PixelFormat */
  GL_DEPTH_COMPONENT = 0x1902,
  GL_ALPHA = 0x1906,
  GL_RGB = 0x1907,
  GL_RGBA = 0x1908,
  GL_LUMINANCE = 0x1909,
  GL_LUMINANCE_ALPHA = 0x190A,

#endif
  /* PixelType */
  /*      GL_UNSIGNED_BYTE */
  GL_UNSIGNED_SHORT_4_4_4_4 = 0x8033,
  GL_UNSIGNED_SHORT_5_5_5_1 = 0x8034,
  GL_UNSIGNED_SHORT_5_6_5 = 0x8363,

  /* Shaders */
  GL_FRAGMENT_SHADER = 0x8B30,
  GL_VERTEX_SHADER = 0x8B31,
  GL_MAX_VERTEX_ATTRIBS = 0x8869,
  GL_MAX_VERTEX_UNIFORM_VECTORS = 0x8DFB,
  GL_MAX_VARYING_VECTORS = 0x8DFC,
  GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS = 0x8B4D,
  GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS = 0x8B4C,
  GL_MAX_TEXTURE_IMAGE_UNITS = 0x8872,
  GL_MAX_FRAGMENT_UNIFORM_VECTORS = 0x8DFD,
  GL_SHADER_TYPE = 0x8B4F,
  GL_DELETE_STATUS = 0x8B80,
  GL_LINK_STATUS = 0x8B82,
  GL_VALIDATE_STATUS = 0x8B83,
  GL_ATTACHED_SHADERS = 0x8B85,
  GL_ACTIVE_UNIFORMS = 0x8B86,
  GL_ACTIVE_UNIFORM_MAX_LENGTH = 0x8B87,
  GL_ACTIVE_ATTRIBUTES = 0x8B89,
  GL_ACTIVE_ATTRIBUTE_MAX_LENGTH = 0x8B8A,
  GL_SHADING_LANGUAGE_VERSION = 0x8B8C,
  GL_CURRENT_PROGRAM = 0x8B8D,

#ifdef SKIP_OPENGL_1
  /* StencilFunction */
  GL_NEVER = 0x0200,
  GL_LESS = 0x0201,
  GL_EQUAL = 0x0202,
  GL_LEQUAL = 0x0203,
  GL_GREATER = 0x0204,
  GL_NOTEQUAL = 0x0205,
  GL_GEQUAL = 0x0206,
  GL_ALWAYS = 0x0207,

  /* StencilOp */
  /*      GL_ZERO */
  GL_KEEP = 0x1E00,
  GL_REPLACE = 0x1E01,
  GL_INCR = 0x1E02,
  GL_DECR = 0x1E03,
  GL_INVERT = 0x150A,
  GL_INCR_WRAP = 0x8507,
  GL_DECR_WRAP = 0x8508,

  /* StringName */
  GL_VENDOR = 0x1F00,
  GL_RENDERER = 0x1F01,
  GL_VERSION = 0x1F02,
  GL_EXTENSIONS = 0x1F03,

  /* TextureMagFilter */
  GL_NEAREST = 0x2600,
  GL_LINEAR = 0x2601,

  /* TextureMinFilter */
  /*      GL_NEAREST */
  /*      GL_LINEAR */
  GL_NEAREST_MIPMAP_NEAREST = 0x2700,
  GL_LINEAR_MIPMAP_NEAREST = 0x2701,
  GL_NEAREST_MIPMAP_LINEAR = 0x2702,
  GL_LINEAR_MIPMAP_LINEAR = 0x2703,

  /* TextureParameterName */
  GL_TEXTURE_MAG_FILTER = 0x2800,
  GL_TEXTURE_MIN_FILTER = 0x2801,
  GL_TEXTURE_WRAP_S = 0x2802,
  GL_TEXTURE_WRAP_T = 0x2803,

  /* TextureTarget */
  /*      GL_TEXTURE_2D */
  GL_TEXTURE = 0x1702,
#endif

  GL_TEXTURE_CUBE_MAP = 0x8513,
  GL_TEXTURE_BINDING_CUBE_MAP = 0x8514,
  GL_TEXTURE_CUBE_MAP_POSITIVE_X = 0x8515,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_X = 0x8516,
  GL_TEXTURE_CUBE_MAP_POSITIVE_Y = 0x8517,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_Y = 0x8518,
  GL_TEXTURE_CUBE_MAP_POSITIVE_Z = 0x8519,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_Z = 0x851A,
  GL_MAX_CUBE_MAP_TEXTURE_SIZE = 0x851C,

  /* TextureUnit */
  GL_TEXTURE0 = 0x84C0,
  GL_TEXTURE1 = 0x84C1,
  GL_TEXTURE2 = 0x84C2,
  GL_TEXTURE3 = 0x84C3,
  GL_TEXTURE4 = 0x84C4,
  GL_TEXTURE5 = 0x84C5,
  GL_TEXTURE6 = 0x84C6,
  GL_TEXTURE7 = 0x84C7,
  GL_TEXTURE8 = 0x84C8,
  GL_TEXTURE9 = 0x84C9,
  GL_TEXTURE10 = 0x84CA,
  GL_TEXTURE11 = 0x84CB,
  GL_TEXTURE12 = 0x84CC,
  GL_TEXTURE13 = 0x84CD,
  GL_TEXTURE14 = 0x84CE,
  GL_TEXTURE15 = 0x84CF,
  GL_TEXTURE16 = 0x84D0,
  GL_TEXTURE17 = 0x84D1,
  GL_TEXTURE18 = 0x84D2,
  GL_TEXTURE19 = 0x84D3,
  GL_TEXTURE20 = 0x84D4,
  GL_TEXTURE21 = 0x84D5,
  GL_TEXTURE22 = 0x84D6,
  GL_TEXTURE23 = 0x84D7,
  GL_TEXTURE24 = 0x84D8,
  GL_TEXTURE25 = 0x84D9,
  GL_TEXTURE26 = 0x84DA,
  GL_TEXTURE27 = 0x84DB,
  GL_TEXTURE28 = 0x84DC,
  GL_TEXTURE29 = 0x84DD,
  GL_TEXTURE30 = 0x84DE,
  GL_TEXTURE31 = 0x84DF,
  GL_ACTIVE_TEXTURE = 0x84E0,

  /* TextureWrapMode */
#ifdef SKIP_OPENGL_1
  GL_REPEAT = 0x2901,
#endif
  GL_CLAMP_TO_EDGE = 0x812F,
  GL_MIRRORED_REPEAT = 0x8370,

  /* Uniform Types */
  GL_FLOAT_VEC2 = 0x8B50,
  GL_FLOAT_VEC3 = 0x8B51,
  GL_FLOAT_VEC4 = 0x8B52,
  GL_INT_VEC2 = 0x8B53,
  GL_INT_VEC3 = 0x8B54,
  GL_INT_VEC4 = 0x8B55,
  GL_BOOL = 0x8B56,
  GL_BOOL_VEC2 = 0x8B57,
  GL_BOOL_VEC3 = 0x8B58,
  GL_BOOL_VEC4 = 0x8B59,
  GL_FLOAT_MAT2 = 0x8B5A,
  GL_FLOAT_MAT3 = 0x8B5B,
  GL_FLOAT_MAT4 = 0x8B5C,
  GL_SAMPLER_2D = 0x8B5E,
  GL_SAMPLER_CUBE = 0x8B60,

  /* Vertex Arrays */
  GL_VERTEX_ATTRIB_ARRAY_ENABLED = 0x8622,
  GL_VERTEX_ATTRIB_ARRAY_SIZE = 0x8623,
  GL_VERTEX_ATTRIB_ARRAY_STRIDE = 0x8624,
  GL_VERTEX_ATTRIB_ARRAY_TYPE = 0x8625,
  GL_VERTEX_ATTRIB_ARRAY_NORMALIZED = 0x886A,
  GL_VERTEX_ATTRIB_ARRAY_POINTER = 0x8645,
  GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING = 0x889F,

  /* Read Format */
  GL_IMPLEMENTATION_COLOR_READ_TYPE = 0x8B9A,
  GL_IMPLEMENTATION_COLOR_READ_FORMAT = 0x8B9B,

  /* Shader Source */
  GL_COMPILE_STATUS = 0x8B81,
  GL_INFO_LOG_LENGTH = 0x8B84,
  GL_SHADER_SOURCE_LENGTH = 0x8B88,
  GL_SHADER_COMPILER = 0x8DFA,

  /* Shader Binary */
  GL_SHADER_BINARY_FORMATS = 0x8DF8,
  GL_NUM_SHADER_BINARY_FORMATS = 0x8DF9,

  /* Shader Precision-Specified Types */
  GL_LOW_FLOAT = 0x8DF0,
  GL_MEDIUM_FLOAT = 0x8DF1,
  GL_HIGH_FLOAT = 0x8DF2,
  GL_LOW_INT = 0x8DF3,
  GL_MEDIUM_INT = 0x8DF4,
  GL_HIGH_INT = 0x8DF5,

  /* Framebuffer Object. */
  GL_FRAMEBUFFER = 0x8D40,
  GL_RENDERBUFFER = 0x8D41,

#ifdef SKIP_OPENGL_1
  GL_RGBA4 = 0x8056,
  GL_RGB5_A1 = 0x8057,
#endif
  GL_RGB565 = 0x8D62,
  GL_DEPTH_COMPONENT16 = 0x81A5,
  GL_STENCIL_INDEX8 = 0x8D48,

  GL_RENDERBUFFER_WIDTH = 0x8D42,
  GL_RENDERBUFFER_HEIGHT = 0x8D43,
  GL_RENDERBUFFER_INTERNAL_FORMAT = 0x8D44,
  GL_RENDERBUFFER_RED_SIZE = 0x8D50,
  GL_RENDERBUFFER_GREEN_SIZE = 0x8D51,
  GL_RENDERBUFFER_BLUE_SIZE = 0x8D52,
  GL_RENDERBUFFER_ALPHA_SIZE = 0x8D53,
  GL_RENDERBUFFER_DEPTH_SIZE = 0x8D54,
  GL_RENDERBUFFER_STENCIL_SIZE = 0x8D55,

  GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE = 0x8CD0,
  GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME = 0x8CD1,
  GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL = 0x8CD2,
  GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE = 0x8CD3,

  GL_COLOR_ATTACHMENT0 = 0x8CE0,
  GL_DEPTH_ATTACHMENT = 0x8D00,
  GL_STENCIL_ATTACHMENT = 0x8D20,

#ifdef SKIP_OPENGL_1
  GL_NONE = 0,
#endif

  GL_FRAMEBUFFER_COMPLETE = 0x8CD5,
  GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT = 0x8CD6,
  GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT = 0x8CD7,
  GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS = 0x8CD9,
  GL_FRAMEBUFFER_UNSUPPORTED = 0x8CDD,

  GL_FRAMEBUFFER_BINDING = 0x8CA6,
  GL_RENDERBUFFER_BINDING = 0x8CA7,
  GL_MAX_RENDERBUFFER_SIZE = 0x84E8,

  GL_INVALID_FRAMEBUFFER_OPERATION = 0x0506,

  /* OpenGL ES 3.0 */

#ifdef SKIP_OPENGL_1
  GL_READ_BUFFER = 0x0C02,
  GL_UNPACK_ROW_LENGTH = 0x0CF2,
  GL_UNPACK_SKIP_ROWS = 0x0CF3,
  GL_UNPACK_SKIP_PIXELS = 0x0CF4,
  GL_PACK_ROW_LENGTH = 0x0D02,
  GL_PACK_SKIP_ROWS = 0x0D03,
  GL_PACK_SKIP_PIXELS = 0x0D04,
  GL_COLOR = 0x1800,
  GL_DEPTH = 0x1801,
  GL_STENCIL = 0x1802,
  GL_RED = 0x1903,
  GL_RGB8 = 0x8051,
  GL_RGBA8 = 0x8058,
  GL_RGB10_A2 = 0x8059,
#endif

  GL_TEXTURE_BINDING_3D = 0x806A,
  GL_UNPACK_SKIP_IMAGES = 0x806D,
  GL_UNPACK_IMAGE_HEIGHT = 0x806E,
  GL_TEXTURE_3D = 0x806F,
  GL_TEXTURE_WRAP_R = 0x8072,
  GL_MAX_3D_TEXTURE_SIZE = 0x8073,
  GL_UNSIGNED_INT_2_10_10_10_REV = 0x8368,
  GL_MAX_ELEMENTS_VERTICES = 0x80E8,
  GL_MAX_ELEMENTS_INDICES = 0x80E9,
  GL_TEXTURE_MIN_LOD = 0x813A,
  GL_TEXTURE_MAX_LOD = 0x813B,
  GL_TEXTURE_BASE_LEVEL = 0x813C,
  GL_TEXTURE_MAX_LEVEL = 0x813D,
  GL_MIN = 0x8007,
  GL_MAX = 0x8008,
  GL_DEPTH_COMPONENT24 = 0x81A6,
  GL_MAX_TEXTURE_LOD_BIAS = 0x84FD,
  GL_TEXTURE_COMPARE_MODE = 0x884C,
  GL_TEXTURE_COMPARE_FUNC = 0x884D,
  GL_CURRENT_QUERY = 0x8865,
  GL_QUERY_RESULT = 0x8866,
  GL_QUERY_RESULT_AVAILABLE = 0x8867,
  GL_BUFFER_MAPPED = 0x88BC,
  GL_BUFFER_MAP_POINTER = 0x88BD,
  GL_STREAM_READ = 0x88E1,
  GL_STREAM_COPY = 0x88E2,
  GL_STATIC_READ = 0x88E5,
  GL_STATIC_COPY = 0x88E6,
  GL_DYNAMIC_READ = 0x88E9,
  GL_DYNAMIC_COPY = 0x88EA,
  GL_MAX_DRAW_BUFFERS = 0x8824,
  GL_DRAW_BUFFER0 = 0x8825,
  GL_DRAW_BUFFER1 = 0x8826,
  GL_DRAW_BUFFER2 = 0x8827,
  GL_DRAW_BUFFER3 = 0x8828,
  GL_DRAW_BUFFER4 = 0x8829,
  GL_DRAW_BUFFER5 = 0x882A,
  GL_DRAW_BUFFER6 = 0x882B,
  GL_DRAW_BUFFER7 = 0x882C,
  GL_DRAW_BUFFER8 = 0x882D,
  GL_DRAW_BUFFER9 = 0x882E,
  GL_DRAW_BUFFER10 = 0x882F,
  GL_DRAW_BUFFER11 = 0x8830,
  GL_DRAW_BUFFER12 = 0x8831,
  GL_DRAW_BUFFER13 = 0x8832,
  GL_DRAW_BUFFER14 = 0x8833,
  GL_DRAW_BUFFER15 = 0x8834,
  GL_MAX_FRAGMENT_UNIFORM_COMPONENTS = 0x8B49,
  GL_MAX_VERTEX_UNIFORM_COMPONENTS = 0x8B4A,
  GL_SAMPLER_3D = 0x8B5F,
  GL_SAMPLER_2D_SHADOW = 0x8B62,
  GL_FRAGMENT_SHADER_DERIVATIVE_HINT = 0x8B8B,
  GL_PIXEL_PACK_BUFFER = 0x88EB,
  GL_PIXEL_UNPACK_BUFFER = 0x88EC,
  GL_PIXEL_PACK_BUFFER_BINDING = 0x88ED,
  GL_PIXEL_UNPACK_BUFFER_BINDING = 0x88EF,
  GL_FLOAT_MAT2x3 = 0x8B65,
  GL_FLOAT_MAT2x4 = 0x8B66,
  GL_FLOAT_MAT3x2 = 0x8B67,
  GL_FLOAT_MAT3x4 = 0x8B68,
  GL_FLOAT_MAT4x2 = 0x8B69,
  GL_FLOAT_MAT4x3 = 0x8B6A,
  GL_SRGB = 0x8C40,
  GL_SRGB8 = 0x8C41,
  GL_SRGB8_ALPHA8 = 0x8C43,
  GL_COMPARE_REF_TO_TEXTURE = 0x884E,
  GL_MAJOR_VERSION = 0x821B,
  GL_MINOR_VERSION = 0x821C,
  GL_NUM_EXTENSIONS = 0x821D,
  GL_RGBA32F = 0x8814,
  GL_RGB32F = 0x8815,
  GL_RGBA16F = 0x881A,
  GL_RGB16F = 0x881B,
  GL_VERTEX_ATTRIB_ARRAY_INTEGER = 0x88FD,
  GL_MAX_ARRAY_TEXTURE_LAYERS = 0x88FF,
  GL_MIN_PROGRAM_TEXEL_OFFSET = 0x8904,
  GL_MAX_PROGRAM_TEXEL_OFFSET = 0x8905,
  GL_MAX_VARYING_COMPONENTS = 0x8B4B,
  GL_TEXTURE_2D_ARRAY = 0x8C1A,
  GL_TEXTURE_BINDING_2D_ARRAY = 0x8C1D,
  GL_R11F_G11F_B10F = 0x8C3A,
  GL_UNSIGNED_INT_10F_11F_11F_REV = 0x8C3B,
  GL_RGB9_E5 = 0x8C3D,
  GL_UNSIGNED_INT_5_9_9_9_REV = 0x8C3E,
  GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH = 0x8C76,
  GL_TRANSFORM_FEEDBACK_BUFFER_MODE = 0x8C7F,
  GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS = 0x8C80,
  GL_TRANSFORM_FEEDBACK_VARYINGS = 0x8C83,
  GL_TRANSFORM_FEEDBACK_BUFFER_START = 0x8C84,
  GL_TRANSFORM_FEEDBACK_BUFFER_SIZE = 0x8C85,
  GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN = 0x8C88,
  GL_RASTERIZER_DISCARD = 0x8C89,
  GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS = 0x8C8A,
  GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS = 0x8C8B,
  GL_INTERLEAVED_ATTRIBS = 0x8C8C,
  GL_SEPARATE_ATTRIBS = 0x8C8D,
  GL_TRANSFORM_FEEDBACK_BUFFER = 0x8C8E,
  GL_TRANSFORM_FEEDBACK_BUFFER_BINDING = 0x8C8F,
  GL_RGBA32UI = 0x8D70,
  GL_RGB32UI = 0x8D71,
  GL_RGBA16UI = 0x8D76,
  GL_RGB16UI = 0x8D77,
  GL_RGBA8UI = 0x8D7C,
  GL_RGB8UI = 0x8D7D,
  GL_RGBA32I = 0x8D82,
  GL_RGB32I = 0x8D83,
  GL_RGBA16I = 0x8D88,
  GL_RGB16I = 0x8D89,
  GL_RGBA8I = 0x8D8E,
  GL_RGB8I = 0x8D8F,
  GL_RED_INTEGER = 0x8D94,
  GL_RGB_INTEGER = 0x8D98,
  GL_RGBA_INTEGER = 0x8D99,
  GL_SAMPLER_2D_ARRAY = 0x8DC1,
  GL_SAMPLER_2D_ARRAY_SHADOW = 0x8DC4,
  GL_SAMPLER_CUBE_SHADOW = 0x8DC5,
  GL_UNSIGNED_INT_VEC2 = 0x8DC6,
  GL_UNSIGNED_INT_VEC3 = 0x8DC7,
  GL_UNSIGNED_INT_VEC4 = 0x8DC8,
  GL_INT_SAMPLER_2D = 0x8DCA,
  GL_INT_SAMPLER_3D = 0x8DCB,
  GL_INT_SAMPLER_CUBE = 0x8DCC,
  GL_INT_SAMPLER_2D_ARRAY = 0x8DCF,
  GL_UNSIGNED_INT_SAMPLER_2D = 0x8DD2,
  GL_UNSIGNED_INT_SAMPLER_3D = 0x8DD3,
  GL_UNSIGNED_INT_SAMPLER_CUBE = 0x8DD4,
  GL_UNSIGNED_INT_SAMPLER_2D_ARRAY = 0x8DD7,
  GL_BUFFER_ACCESS_FLAGS = 0x911F,
  GL_BUFFER_MAP_LENGTH = 0x9120,
  GL_BUFFER_MAP_OFFSET = 0x9121,
  GL_DEPTH_COMPONENT32F = 0x8CAC,
  GL_DEPTH32F_STENCIL8 = 0x8CAD,
  GL_FLOAT_32_UNSIGNED_INT_24_8_REV = 0x8DAD,
  GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING = 0x8210,
  GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE = 0x8211,
  GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE = 0x8212,
  GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE = 0x8213,
  GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE = 0x8214,
  GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE = 0x8215,
  GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE = 0x8216,
  GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE = 0x8217,
  GL_FRAMEBUFFER_DEFAULT = 0x8218,
  GL_FRAMEBUFFER_UNDEFINED = 0x8219,
  GL_DEPTH_STENCIL_ATTACHMENT = 0x821A,
  GL_DEPTH_STENCIL = 0x84F9,
  GL_UNSIGNED_INT_24_8 = 0x84FA,
  GL_DEPTH24_STENCIL8 = 0x88F0,
  GL_UNSIGNED_NORMALIZED = 0x8C17,
  GL_DRAW_FRAMEBUFFER_BINDING = GL_FRAMEBUFFER_BINDING,
  GL_READ_FRAMEBUFFER = 0x8CA8,
  GL_DRAW_FRAMEBUFFER = 0x8CA9,
  GL_READ_FRAMEBUFFER_BINDING = 0x8CAA,
  GL_RENDERBUFFER_SAMPLES = 0x8CAB,
  GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER = 0x8CD4,
  GL_MAX_COLOR_ATTACHMENTS = 0x8CDF,
  GL_COLOR_ATTACHMENT1 = 0x8CE1,
  GL_COLOR_ATTACHMENT2 = 0x8CE2,
  GL_COLOR_ATTACHMENT3 = 0x8CE3,
  GL_COLOR_ATTACHMENT4 = 0x8CE4,
  GL_COLOR_ATTACHMENT5 = 0x8CE5,
  GL_COLOR_ATTACHMENT6 = 0x8CE6,
  GL_COLOR_ATTACHMENT7 = 0x8CE7,
  GL_COLOR_ATTACHMENT8 = 0x8CE8,
  GL_COLOR_ATTACHMENT9 = 0x8CE9,
  GL_COLOR_ATTACHMENT10 = 0x8CEA,
  GL_COLOR_ATTACHMENT11 = 0x8CEB,
  GL_COLOR_ATTACHMENT12 = 0x8CEC,
  GL_COLOR_ATTACHMENT13 = 0x8CED,
  GL_COLOR_ATTACHMENT14 = 0x8CEE,
  GL_COLOR_ATTACHMENT15 = 0x8CEF,
  GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE = 0x8D56,
  GL_MAX_SAMPLES = 0x8D57,
  GL_HALF_FLOAT = 0x140B,
  GL_MAP_READ_BIT = 0x0001,
  GL_MAP_WRITE_BIT = 0x0002,
  GL_MAP_INVALIDATE_RANGE_BIT = 0x0004,
  GL_MAP_INVALIDATE_BUFFER_BIT = 0x0008,
  GL_MAP_FLUSH_EXPLICIT_BIT = 0x0010,
  GL_MAP_UNSYNCHRONIZED_BIT = 0x0020,
  GL_RG = 0x8227,
  GL_RG_INTEGER = 0x8228,
  GL_R8 = 0x8229,
  GL_RG8 = 0x822B,
  GL_R16F = 0x822D,
  GL_R32F = 0x822E,
  GL_RG16F = 0x822F,
  GL_RG32F = 0x8230,
  GL_R8I = 0x8231,
  GL_R8UI = 0x8232,
  GL_R16I = 0x8233,
  GL_R16UI = 0x8234,
  GL_R32I = 0x8235,
  GL_R32UI = 0x8236,
  GL_RG8I = 0x8237,
  GL_RG8UI = 0x8238,
  GL_RG16I = 0x8239,
  GL_RG16UI = 0x823A,
  GL_RG32I = 0x823B,
  GL_RG32UI = 0x823C,
  GL_VERTEX_ARRAY_BINDING = 0x85B5,
  GL_R8_SNORM = 0x8F94,
  GL_RG8_SNORM = 0x8F95,
  GL_RGB8_SNORM = 0x8F96,
  GL_RGBA8_SNORM = 0x8F97,
  GL_SIGNED_NORMALIZED = 0x8F9C,
  GL_PRIMITIVE_RESTART_FIXED_INDEX = 0x8D69,
  GL_COPY_READ_BUFFER = 0x8F36,
  GL_COPY_WRITE_BUFFER = 0x8F37,
  GL_COPY_READ_BUFFER_BINDING = GL_COPY_READ_BUFFER,
  GL_COPY_WRITE_BUFFER_BINDING = GL_COPY_WRITE_BUFFER,
  GL_UNIFORM_BUFFER = 0x8A11,
  GL_UNIFORM_BUFFER_BINDING = 0x8A28,
  GL_UNIFORM_BUFFER_START = 0x8A29,
  GL_UNIFORM_BUFFER_SIZE = 0x8A2A,
  GL_MAX_VERTEX_UNIFORM_BLOCKS = 0x8A2B,
  GL_MAX_FRAGMENT_UNIFORM_BLOCKS = 0x8A2D,
  GL_MAX_COMBINED_UNIFORM_BLOCKS = 0x8A2E,
  GL_MAX_UNIFORM_BUFFER_BINDINGS = 0x8A2F,
  GL_MAX_UNIFORM_BLOCK_SIZE = 0x8A30,
  GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS = 0x8A31,
  GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS = 0x8A33,
  GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT = 0x8A34,
  GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH = 0x8A35,
  GL_ACTIVE_UNIFORM_BLOCKS = 0x8A36,
  GL_UNIFORM_TYPE = 0x8A37,
  GL_UNIFORM_SIZE = 0x8A38,
  GL_UNIFORM_NAME_LENGTH = 0x8A39,
  GL_UNIFORM_BLOCK_INDEX = 0x8A3A,
  GL_UNIFORM_OFFSET = 0x8A3B,
  GL_UNIFORM_ARRAY_STRIDE = 0x8A3C,
  GL_UNIFORM_MATRIX_STRIDE = 0x8A3D,
  GL_UNIFORM_IS_ROW_MAJOR = 0x8A3E,
  GL_UNIFORM_BLOCK_BINDING = 0x8A3F,
  GL_UNIFORM_BLOCK_DATA_SIZE = 0x8A40,
  GL_UNIFORM_BLOCK_NAME_LENGTH = 0x8A41,
  GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS = 0x8A42,
  GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES = 0x8A43,
  GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER = 0x8A44,
  GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER = 0x8A46,
  GL_INVALID_INDEX = 0xFFFFFFFFu,
  GL_MAX_VERTEX_OUTPUT_COMPONENTS = 0x9122,
  GL_MAX_FRAGMENT_INPUT_COMPONENTS = 0x9125,
  GL_MAX_SERVER_WAIT_TIMEOUT = 0x9111,
  GL_OBJECT_TYPE = 0x9112,
  GL_SYNC_CONDITION = 0x9113,
  GL_SYNC_STATUS = 0x9114,
  GL_SYNC_FLAGS = 0x9115,
  GL_SYNC_FENCE = 0x9116,
  GL_SYNC_GPU_COMMANDS_COMPLETE = 0x9117,
  GL_UNSIGNALED = 0x9118,
  GL_SIGNALED = 0x9119,
  GL_ALREADY_SIGNALED = 0x911A,
  GL_TIMEOUT_EXPIRED = 0x911B,
  GL_CONDITION_SATISFIED = 0x911C,
  GL_WAIT_FAILED = 0x911D,
  GL_SYNC_FLUSH_COMMANDS_BIT = 0x00000001,
  GL_TIMEOUT_IGNORED = ~0,
  GL_VERTEX_ATTRIB_ARRAY_DIVISOR = 0x88FE,
  GL_ANY_SAMPLES_PASSED = 0x8C2F,
  GL_ANY_SAMPLES_PASSED_CONSERVATIVE = 0x8D6A,
  GL_SAMPLER_BINDING = 0x8919,
  GL_RGB10_A2UI = 0x906F,
  GL_TEXTURE_SWIZZLE_R = 0x8E42,
  GL_TEXTURE_SWIZZLE_G = 0x8E43,
  GL_TEXTURE_SWIZZLE_B = 0x8E44,
  GL_TEXTURE_SWIZZLE_A = 0x8E45,
#ifdef SKIP_OPENGL_1
  GL_GREEN = 0x1904,
  GL_BLUE = 0x1905,
#endif
  GL_INT_2_10_10_10_REV = 0x8D9F,
  GL_TRANSFORM_FEEDBACK = 0x8E22,
  GL_TRANSFORM_FEEDBACK_PAUSED = 0x8E23,
  GL_TRANSFORM_FEEDBACK_ACTIVE = 0x8E24,
  GL_TRANSFORM_FEEDBACK_BINDING = 0x8E25,
  GL_PROGRAM_BINARY_RETRIEVABLE_HINT = 0x8257,
  GL_PROGRAM_BINARY_LENGTH = 0x8741,
  GL_NUM_PROGRAM_BINARY_FORMATS = 0x87FE,
  GL_PROGRAM_BINARY_FORMATS = 0x87FF,
  GL_COMPRESSED_R11_EAC = 0x9270,
  GL_COMPRESSED_SIGNED_R11_EAC = 0x9271,
  GL_COMPRESSED_RG11_EAC = 0x9272,
  GL_COMPRESSED_SIGNED_RG11_EAC = 0x9273,
  GL_COMPRESSED_RGB8_ETC2 = 0x9274,
  GL_COMPRESSED_SRGB8_ETC2 = 0x9275,
  GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 = 0x9276,
  GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 = 0x9277,
  GL_COMPRESSED_RGBA8_ETC2_EAC = 0x9278,
  GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC = 0x9279,
  GL_TEXTURE_IMMUTABLE_FORMAT = 0x912F,
  GL_MAX_ELEMENT_INDEX = 0x8D6B,
  GL_NUM_SAMPLE_COUNTS = 0x9380,
  GL_TEXTURE_IMMUTABLE_LEVELS = 0x8D63,
};

typedef void           (GL_APIENTRY *glActiveTexture_t) (GLenum texture);
typedef void           (GL_APIENTRY *glAttachShader_t) (GLuint program, GLuint shader);
typedef void           (GL_APIENTRY *glBindAttribLocation_t) (GLuint program, GLuint index, const GLchar* name);
typedef void           (GL_APIENTRY *glBindBuffer_t) (GLenum target, GLuint buffer);
typedef void           (GL_APIENTRY *glBindFramebuffer_t) (GLenum target, GLuint framebuffer);
typedef void           (GL_APIENTRY *glBindRenderbuffer_t) (GLenum target, GLuint renderbuffer);
typedef void           (GL_APIENTRY *glBindTexture_t) (GLenum target, GLuint texture);
typedef void           (GL_APIENTRY *glBlendColor_t) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void           (GL_APIENTRY *glBlendEquation_t) (GLenum mode);
typedef void           (GL_APIENTRY *glBlendEquationSeparate_t) (GLenum modeRGB, GLenum modeAlpha);
typedef void           (GL_APIENTRY *glBlendFunc_t) (GLenum sfactor, GLenum dfactor);
typedef void           (GL_APIENTRY *glBlendFuncSeparate_t) (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
typedef void           (GL_APIENTRY *glBufferData_t) (GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
typedef void           (GL_APIENTRY *glBufferSubData_t) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
typedef GLenum         (GL_APIENTRY *glCheckFramebufferStatus_t) (GLenum target);
typedef void           (GL_APIENTRY *glClear_t) (GLbitfield mask);
typedef void           (GL_APIENTRY *glClearColor_t) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void           (GL_APIENTRY *glClearDepthf_t) (GLfloat depth);
typedef void           (GL_APIENTRY *glClearStencil_t) (GLint s);
typedef void           (GL_APIENTRY *glColorMask_t) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
typedef void           (GL_APIENTRY *glCompileShader_t) (GLuint shader);
typedef void           (GL_APIENTRY *glCompressedTexImage2D_t) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data);
typedef void           (GL_APIENTRY *glCompressedTexSubImage2D_t) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data);
typedef void           (GL_APIENTRY *glCopyTexImage2D_t) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef void           (GL_APIENTRY *glCopyTexSubImage2D_t) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef GLuint         (GL_APIENTRY *glCreateProgram_t) (void);
typedef GLuint         (GL_APIENTRY *glCreateShader_t) (GLenum type);
typedef void           (GL_APIENTRY *glCullFace_t) (GLenum mode);
typedef void           (GL_APIENTRY *glDeleteBuffers_t) (GLsizei n, const GLuint* buffers);
typedef void           (GL_APIENTRY *glDeleteFramebuffers_t) (GLsizei n, const GLuint* framebuffers);
typedef void           (GL_APIENTRY *glDeleteProgram_t) (GLuint program);
typedef void           (GL_APIENTRY *glDeleteRenderbuffers_t) (GLsizei n, const GLuint* renderbuffers);
typedef void           (GL_APIENTRY *glDeleteShader_t) (GLuint shader);
typedef void           (GL_APIENTRY *glDeleteTextures_t) (GLsizei n, const GLuint* textures);
typedef void           (GL_APIENTRY *glDepthFunc_t) (GLenum func);
typedef void           (GL_APIENTRY *glDepthMask_t) (GLboolean flag);
typedef void           (GL_APIENTRY *glDepthRangef_t) (GLfloat n, GLfloat f);
typedef void           (GL_APIENTRY *glDetachShader_t) (GLuint program, GLuint shader);
typedef void           (GL_APIENTRY *glDisable_t) (GLenum cap);
typedef void           (GL_APIENTRY *glDisableVertexAttribArray_t) (GLuint index);
typedef void           (GL_APIENTRY *glDrawArrays_t) (GLenum mode, GLint first, GLsizei count);
typedef void           (GL_APIENTRY *glDrawElements_t) (GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);
typedef void           (GL_APIENTRY *glEnable_t) (GLenum cap);
typedef void           (GL_APIENTRY *glEnableVertexAttribArray_t) (GLuint index);
typedef void           (GL_APIENTRY *glFinish_t) (void);
typedef void           (GL_APIENTRY *glFlush_t) (void);
typedef void           (GL_APIENTRY *glFramebufferRenderbuffer_t) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void           (GL_APIENTRY *glFramebufferTexture2D_t) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void           (GL_APIENTRY *glFrontFace_t) (GLenum mode);
typedef void           (GL_APIENTRY *glGenBuffers_t) (GLsizei n, GLuint* buffers);
typedef void           (GL_APIENTRY *glGenerateMipmap_t) (GLenum target);
typedef void           (GL_APIENTRY *glGenFramebuffers_t) (GLsizei n, GLuint* framebuffers);
typedef void           (GL_APIENTRY *glGenRenderbuffers_t) (GLsizei n, GLuint* renderbuffers);
typedef void           (GL_APIENTRY *glGenTextures_t) (GLsizei n, GLuint* textures);
typedef void           (GL_APIENTRY *glGetActiveAttrib_t) (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
typedef void           (GL_APIENTRY *glGetActiveUniform_t) (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
typedef void           (GL_APIENTRY *glGetAttachedShaders_t) (GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders);
typedef int            (GL_APIENTRY *glGetAttribLocation_t) (GLuint program, const GLchar* name);
typedef void           (GL_APIENTRY *glGetBooleanv_t) (GLenum pname, GLboolean* params);
typedef void           (GL_APIENTRY *glGetBufferParameteriv_t) (GLenum target, GLenum pname, GLint* params);
typedef GLenum         (GL_APIENTRY *glGetError_t) (void);
typedef void           (GL_APIENTRY *glGetFloatv_t) (GLenum pname, GLfloat* params);
typedef void           (GL_APIENTRY *glGetFramebufferAttachmentParameteriv_t) (GLenum target, GLenum attachment, GLenum pname, GLint* params);
typedef void           (GL_APIENTRY *glGetIntegerv_t) (GLenum pname, GLint* params);
typedef void           (GL_APIENTRY *glGetProgramiv_t) (GLuint program, GLenum pname, GLint* params);
typedef void           (GL_APIENTRY *glGetProgramInfoLog_t) (GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog);
typedef void           (GL_APIENTRY *glGetRenderbufferParameteriv_t) (GLenum target, GLenum pname, GLint* params);
typedef void           (GL_APIENTRY *glGetShaderiv_t) (GLuint shader, GLenum pname, GLint* params);
typedef void           (GL_APIENTRY *glGetShaderInfoLog_t) (GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog);
typedef void           (GL_APIENTRY *glGetShaderPrecisionFormat_t) (GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision);
typedef void           (GL_APIENTRY *glGetShaderSource_t) (GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source);
typedef const GLubyte* (GL_APIENTRY *glGetString_t) (GLenum name);
typedef void           (GL_APIENTRY *glGetTexParameterfv_t) (GLenum target, GLenum pname, GLfloat* params);
typedef void           (GL_APIENTRY *glGetTexParameteriv_t) (GLenum target, GLenum pname, GLint* params);
typedef void           (GL_APIENTRY *glGetUniformfv_t) (GLuint program, GLint location, GLfloat* params);
typedef void           (GL_APIENTRY *glGetUniformiv_t) (GLuint program, GLint location, GLint* params);
typedef int            (GL_APIENTRY *glGetUniformLocation_t) (GLuint program, const GLchar* name);
typedef void           (GL_APIENTRY *glGetVertexAttribfv_t) (GLuint index, GLenum pname, GLfloat* params);
typedef void           (GL_APIENTRY *glGetVertexAttribiv_t) (GLuint index, GLenum pname, GLint* params);
typedef void           (GL_APIENTRY *glGetVertexAttribPointerv_t) (GLuint index, GLenum pname, GLvoid** pointer);
typedef void           (GL_APIENTRY *glHint_t) (GLenum target, GLenum mode);
typedef GLboolean      (GL_APIENTRY *glIsBuffer_t) (GLuint buffer);
typedef GLboolean      (GL_APIENTRY *glIsEnabled_t) (GLenum cap);
typedef GLboolean      (GL_APIENTRY *glIsFramebuffer_t) (GLuint framebuffer);
typedef GLboolean      (GL_APIENTRY *glIsProgram_t) (GLuint program);
typedef GLboolean      (GL_APIENTRY *glIsRenderbuffer_t) (GLuint renderbuffer);
typedef GLboolean      (GL_APIENTRY *glIsShader_t) (GLuint shader);
typedef GLboolean      (GL_APIENTRY *glIsTexture_t) (GLuint texture);
typedef void           (GL_APIENTRY *glLineWidth_t) (GLfloat width);
typedef void           (GL_APIENTRY *glLinkProgram_t) (GLuint program);
typedef void           (GL_APIENTRY *glPixelStorei_t) (GLenum pname, GLint param);
typedef void           (GL_APIENTRY *glPolygonOffset_t) (GLfloat factor, GLfloat units);
typedef void           (GL_APIENTRY *glReadPixels_t) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
typedef void           (GL_APIENTRY *glReleaseShaderCompiler_t) (void);
typedef void           (GL_APIENTRY *glRenderbufferStorage_t) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void           (GL_APIENTRY *glSampleCoverage_t) (GLfloat value, GLboolean invert);
typedef void           (GL_APIENTRY *glScissor_t) (GLint x, GLint y, GLsizei width, GLsizei height);
typedef void           (GL_APIENTRY *glShaderBinary_t) (GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length);
typedef void           (GL_APIENTRY *glShaderSource_t) (GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
typedef void           (GL_APIENTRY *glStencilFunc_t) (GLenum func, GLint ref, GLuint mask);
typedef void           (GL_APIENTRY *glStencilFuncSeparate_t) (GLenum face, GLenum func, GLint ref, GLuint mask);
typedef void           (GL_APIENTRY *glStencilMask_t) (GLuint mask);
typedef void           (GL_APIENTRY *glStencilMaskSeparate_t) (GLenum face, GLuint mask);
typedef void           (GL_APIENTRY *glStencilOp_t) (GLenum fail, GLenum zfail, GLenum zpass);
typedef void           (GL_APIENTRY *glStencilOpSeparate_t) (GLenum face, GLenum fail, GLenum zfail, GLenum zpass);
typedef void           (GL_APIENTRY *glTexImage2D_t) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
typedef void           (GL_APIENTRY *glTexParameterf_t) (GLenum target, GLenum pname, GLfloat param);
typedef void           (GL_APIENTRY *glTexParameterfv_t) (GLenum target, GLenum pname, const GLfloat* params);
typedef void           (GL_APIENTRY *glTexParameteri_t) (GLenum target, GLenum pname, GLint param);
typedef void           (GL_APIENTRY *glTexParameteriv_t) (GLenum target, GLenum pname, const GLint* params);
typedef void           (GL_APIENTRY *glTexSubImage2D_t) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
typedef void           (GL_APIENTRY *glUniform1f_t) (GLint location, GLfloat x);
typedef void           (GL_APIENTRY *glUniform1fv_t) (GLint location, GLsizei count, const GLfloat* v);
typedef void           (GL_APIENTRY *glUniform1i_t) (GLint location, GLint x);
typedef void           (GL_APIENTRY *glUniform1iv_t) (GLint location, GLsizei count, const GLint* v);
typedef void           (GL_APIENTRY *glUniform2f_t) (GLint location, GLfloat x, GLfloat y);
typedef void           (GL_APIENTRY *glUniform2fv_t) (GLint location, GLsizei count, const GLfloat* v);
typedef void           (GL_APIENTRY *glUniform2i_t) (GLint location, GLint x, GLint y);
typedef void           (GL_APIENTRY *glUniform2iv_t) (GLint location, GLsizei count, const GLint* v);
typedef void           (GL_APIENTRY *glUniform3f_t) (GLint location, GLfloat x, GLfloat y, GLfloat z);
typedef void           (GL_APIENTRY *glUniform3fv_t) (GLint location, GLsizei count, const GLfloat* v);
typedef void           (GL_APIENTRY *glUniform3i_t) (GLint location, GLint x, GLint y, GLint z);
typedef void           (GL_APIENTRY *glUniform3iv_t) (GLint location, GLsizei count, const GLint* v);
typedef void           (GL_APIENTRY *glUniform4f_t) (GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void           (GL_APIENTRY *glUniform4fv_t) (GLint location, GLsizei count, const GLfloat* v);
typedef void           (GL_APIENTRY *glUniform4i_t) (GLint location, GLint x, GLint y, GLint z, GLint w);
typedef void           (GL_APIENTRY *glUniform4iv_t) (GLint location, GLsizei count, const GLint* v);
typedef void           (GL_APIENTRY *glUniformMatrix2fv_t) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void           (GL_APIENTRY *glUniformMatrix3fv_t) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void           (GL_APIENTRY *glUniformMatrix4fv_t) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void           (GL_APIENTRY *glUseProgram_t) (GLuint program);
typedef void           (GL_APIENTRY *glValidateProgram_t) (GLuint program);
typedef void           (GL_APIENTRY *glVertexAttrib1f_t) (GLuint indx, GLfloat x);
typedef void           (GL_APIENTRY *glVertexAttrib1fv_t) (GLuint indx, const GLfloat* values);
typedef void           (GL_APIENTRY *glVertexAttrib2f_t) (GLuint indx, GLfloat x, GLfloat y);
typedef void           (GL_APIENTRY *glVertexAttrib2fv_t) (GLuint indx, const GLfloat* values);
typedef void           (GL_APIENTRY *glVertexAttrib3f_t) (GLuint indx, GLfloat x, GLfloat y, GLfloat z);
typedef void           (GL_APIENTRY *glVertexAttrib3fv_t) (GLuint indx, const GLfloat* values);
typedef void           (GL_APIENTRY *glVertexAttrib4f_t) (GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void           (GL_APIENTRY *glVertexAttrib4fv_t) (GLuint indx, const GLfloat* values);
typedef void           (GL_APIENTRY *glVertexAttribPointer_t) (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr);
typedef void           (GL_APIENTRY *glViewport_t) (GLint x, GLint y, GLsizei width, GLsizei height);

/* OpenGL ES 3.0 */

typedef void           (GL_APIENTRY *glReadBuffer_t) (GLenum mode);
typedef void           (GL_APIENTRY *glDrawRangeElements_t) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices);
typedef void           (GL_APIENTRY *glTexImage3D_t) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
typedef void           (GL_APIENTRY *glTexSubImage3D_t) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels);
typedef void           (GL_APIENTRY *glCopyTexSubImage3D_t) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void           (GL_APIENTRY *glCompressedTexImage3D_t) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data);
typedef void           (GL_APIENTRY *glCompressedTexSubImage3D_t) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data);
typedef void           (GL_APIENTRY *glGenQueries_t) (GLsizei n, GLuint* ids);
typedef void           (GL_APIENTRY *glDeleteQueries_t) (GLsizei n, const GLuint* ids);
typedef GLboolean      (GL_APIENTRY *glIsQuery_t) (GLuint id);
typedef void           (GL_APIENTRY *glBeginQuery_t) (GLenum target, GLuint id);
typedef void           (GL_APIENTRY *glEndQuery_t) (GLenum target);
typedef void           (GL_APIENTRY *glGetQueryiv_t) (GLenum target, GLenum pname, GLint* params);
typedef void           (GL_APIENTRY *glGetQueryObjectuiv_t) (GLuint id, GLenum pname, GLuint* params);
typedef GLboolean      (GL_APIENTRY *glUnmapBuffer_t) (GLenum target);
typedef void           (GL_APIENTRY *glGetBufferPointerv_t) (GLenum target, GLenum pname, GLvoid** params);
typedef void           (GL_APIENTRY *glDrawBuffers_t) (GLsizei n, const GLenum* bufs);
typedef void           (GL_APIENTRY *glUniformMatrix2x3fv_t) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void           (GL_APIENTRY *glUniformMatrix3x2fv_t) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void           (GL_APIENTRY *glUniformMatrix2x4fv_t) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void           (GL_APIENTRY *glUniformMatrix4x2fv_t) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void           (GL_APIENTRY *glUniformMatrix3x4fv_t) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void           (GL_APIENTRY *glUniformMatrix4x3fv_t) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void           (GL_APIENTRY *glBlitFramebuffer_t) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef void           (GL_APIENTRY *glRenderbufferStorageMultisample_t) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
typedef void           (GL_APIENTRY *glFramebufferTextureLayer_t) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
typedef GLvoid*        (GL_APIENTRY *glMapBufferRange_t) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef void           (GL_APIENTRY *glFlushMappedBufferRange_t) (GLenum target, GLintptr offset, GLsizeiptr length);
typedef void           (GL_APIENTRY *glBindVertexArray_t) (GLuint array);
typedef void           (GL_APIENTRY *glDeleteVertexArrays_t) (GLsizei n, const GLuint* arrays);
typedef void           (GL_APIENTRY *glGenVertexArrays_t) (GLsizei n, GLuint* arrays);
typedef GLboolean      (GL_APIENTRY *glIsVertexArray_t) (GLuint array);
typedef void           (GL_APIENTRY *glGetIntegeri_v_t) (GLenum target, GLuint index, GLint* data);
typedef void           (GL_APIENTRY *glBeginTransformFeedback_t) (GLenum primitiveMode);
typedef void           (GL_APIENTRY *glEndTransformFeedback_t) (void);
typedef void           (GL_APIENTRY *glBindBufferRange_t) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
typedef void           (GL_APIENTRY *glBindBufferBase_t) (GLenum target, GLuint index, GLuint buffer);
typedef void           (GL_APIENTRY *glTransformFeedbackVaryings_t) (GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode);
typedef void           (GL_APIENTRY *glGetTransformFeedbackVarying_t) (GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name);
typedef void           (GL_APIENTRY *glVertexAttribIPointer_t) (GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
typedef void           (GL_APIENTRY *glGetVertexAttribIiv_t) (GLuint index, GLenum pname, GLint* params);
typedef void           (GL_APIENTRY *glGetVertexAttribIuiv_t) (GLuint index, GLenum pname, GLuint* params);
typedef void           (GL_APIENTRY *glVertexAttribI4i_t) (GLuint index, GLint x, GLint y, GLint z, GLint w);
typedef void           (GL_APIENTRY *glVertexAttribI4ui_t) (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
typedef void           (GL_APIENTRY *glVertexAttribI4iv_t) (GLuint index, const GLint* v);
typedef void           (GL_APIENTRY *glVertexAttribI4uiv_t) (GLuint index, const GLuint* v);
typedef void           (GL_APIENTRY *glGetUniformuiv_t) (GLuint program, GLint location, GLuint* params);
typedef GLint          (GL_APIENTRY *glGetFragDataLocation_t) (GLuint program, const GLchar *name);
typedef void           (GL_APIENTRY *glUniform1ui_t) (GLint location, GLuint v0);
typedef void           (GL_APIENTRY *glUniform2ui_t) (GLint location, GLuint v0, GLuint v1);
typedef void           (GL_APIENTRY *glUniform3ui_t) (GLint location, GLuint v0, GLuint v1, GLuint v2);
typedef void           (GL_APIENTRY *glUniform4ui_t) (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
typedef void           (GL_APIENTRY *glUniform1uiv_t) (GLint location, GLsizei count, const GLuint* value);
typedef void           (GL_APIENTRY *glUniform2uiv_t) (GLint location, GLsizei count, const GLuint* value);
typedef void           (GL_APIENTRY *glUniform3uiv_t) (GLint location, GLsizei count, const GLuint* value);
typedef void           (GL_APIENTRY *glUniform4uiv_t) (GLint location, GLsizei count, const GLuint* value);
typedef void           (GL_APIENTRY *glClearBufferiv_t) (GLenum buffer, GLint drawbuffer, const GLint* value);
typedef void           (GL_APIENTRY *glClearBufferuiv_t) (GLenum buffer, GLint drawbuffer, const GLuint* value);
typedef void           (GL_APIENTRY *glClearBufferfv_t) (GLenum buffer, GLint drawbuffer, const GLfloat* value);
typedef void           (GL_APIENTRY *glClearBufferfi_t) (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
typedef const GLubyte* (GL_APIENTRY *glGetStringi_t) (GLenum name, GLuint index);
typedef void           (GL_APIENTRY *glCopyBufferSubData_t) (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
typedef void           (GL_APIENTRY *glGetUniformIndices_t) (GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices);
typedef void           (GL_APIENTRY *glGetActiveUniformsiv_t) (GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params);
typedef GLuint         (GL_APIENTRY *glGetUniformBlockIndex_t) (GLuint program, const GLchar* uniformBlockName);
typedef void           (GL_APIENTRY *glGetActiveUniformBlockiv_t) (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params);
typedef void           (GL_APIENTRY *glGetActiveUniformBlockName_t) (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName);
typedef void           (GL_APIENTRY *glUniformBlockBinding_t) (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
typedef void           (GL_APIENTRY *glDrawArraysInstanced_t) (GLenum mode, GLint first, GLsizei count, GLsizei instanceCount);
typedef void           (GL_APIENTRY *glDrawElementsInstanced_t) (GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount);
typedef GLsync         (GL_APIENTRY *glFenceSync_t) (GLenum condition, GLbitfield flags);
typedef GLboolean      (GL_APIENTRY *glIsSync_t) (GLsync sync);
typedef void           (GL_APIENTRY *glDeleteSync_t) (GLsync sync);
typedef GLenum         (GL_APIENTRY *glClientWaitSync_t) (GLsync sync, GLbitfield flags, GLuint64 timeout);
typedef void           (GL_APIENTRY *glWaitSync_t) (GLsync sync, GLbitfield flags, GLuint64 timeout);
typedef void           (GL_APIENTRY *glGetInteger64v_t) (GLenum pname, GLint64* params);
typedef void           (GL_APIENTRY *glGetSynciv_t) (GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values);
typedef void           (GL_APIENTRY *glGetInteger64i_v_t) (GLenum target, GLuint index, GLint64* data);
typedef void           (GL_APIENTRY *glGetBufferParameteri64v_t) (GLenum target, GLenum pname, GLint64* params);
typedef void           (GL_APIENTRY *glGenSamplers_t) (GLsizei count, GLuint* samplers);
typedef void           (GL_APIENTRY *glDeleteSamplers_t) (GLsizei count, const GLuint* samplers);
typedef GLboolean      (GL_APIENTRY *glIsSampler_t) (GLuint sampler);
typedef void           (GL_APIENTRY *glBindSampler_t) (GLuint unit, GLuint sampler);
typedef void           (GL_APIENTRY *glSamplerParameteri_t) (GLuint sampler, GLenum pname, GLint param);
typedef void           (GL_APIENTRY *glSamplerParameteriv_t) (GLuint sampler, GLenum pname, const GLint* param);
typedef void           (GL_APIENTRY *glSamplerParameterf_t) (GLuint sampler, GLenum pname, GLfloat param);
typedef void           (GL_APIENTRY *glSamplerParameterfv_t) (GLuint sampler, GLenum pname, const GLfloat* param);
typedef void           (GL_APIENTRY *glGetSamplerParameteriv_t) (GLuint sampler, GLenum pname, GLint* params);
typedef void           (GL_APIENTRY *glGetSamplerParameterfv_t) (GLuint sampler, GLenum pname, GLfloat* params);
typedef void           (GL_APIENTRY *glVertexAttribDivisor_t) (GLuint index, GLuint divisor);
typedef void           (GL_APIENTRY *glBindTransformFeedback_t) (GLenum target, GLuint id);
typedef void           (GL_APIENTRY *glDeleteTransformFeedbacks_t) (GLsizei n, const GLuint* ids);
typedef void           (GL_APIENTRY *glGenTransformFeedbacks_t) (GLsizei n, GLuint* ids);
typedef GLboolean      (GL_APIENTRY *glIsTransformFeedback_t) (GLuint id);
typedef void           (GL_APIENTRY *glPauseTransformFeedback_t) (void);
typedef void           (GL_APIENTRY *glResumeTransformFeedback_t) (void);
typedef void           (GL_APIENTRY *glGetProgramBinary_t) (GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary);
typedef void           (GL_APIENTRY *glProgramBinary_t) (GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length);
typedef void           (GL_APIENTRY *glProgramParameteri_t) (GLuint program, GLenum pname, GLint value);
typedef void           (GL_APIENTRY *glInvalidateFramebuffer_t) (GLenum target, GLsizei numAttachments, const GLenum* attachments);
typedef void           (GL_APIENTRY *glInvalidateSubFramebuffer_t) (GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void           (GL_APIENTRY *glTexStorage2D_t) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
typedef void           (GL_APIENTRY *glTexStorage3D_t) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
typedef void           (GL_APIENTRY *glGetInternalformativ_t) (GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params);

#ifdef WIN32
  glActiveTexture_t glActiveTexture;
  glAttachShader_t glAttachShader;
  glBindAttribLocation_t glBindAttribLocation;
  glBindBuffer_t glBindBuffer;
  glBindFramebuffer_t glBindFramebuffer;
  glBindRenderbuffer_t glBindRenderbuffer;
  //glBindTexture_t glBindTexture;
  glBlendColor_t glBlendColor;
  glBlendEquation_t glBlendEquation;
  glBlendEquationSeparate_t glBlendEquationSeparate;
  //glBlendFunc_t glBlendFunc;
  glBlendFuncSeparate_t glBlendFuncSeparate;
  glBufferData_t glBufferData;
  glBufferSubData_t glBufferSubData;
  glCheckFramebufferStatus_t glCheckFramebufferStatus;
  //glClear_t glClear;
  //glClearColor_t glClearColor;
  glClearDepthf_t glClearDepthf;
  //glClearStencil_t glClearStencil;
  //glColorMask_t glColorMask;
  glCompileShader_t glCompileShader;
  glCompressedTexImage2D_t glCompressedTexImage2D;
  glCompressedTexSubImage2D_t glCompressedTexSubImage2D;
  //glCopyTexImage2D_t glCopyTexImage2D;
  //glCopyTexSubImage2D_t glCopyTexSubImage2D;
  glCreateProgram_t glCreateProgram;
  glCreateShader_t glCreateShader;
  //glCullFace_t glCullFace;
  glDeleteBuffers_t glDeleteBuffers;
  glDeleteFramebuffers_t glDeleteFramebuffers;
  glDeleteProgram_t glDeleteProgram;
  glDeleteRenderbuffers_t glDeleteRenderbuffers;
  glDeleteShader_t glDeleteShader;
  //glDeleteTextures_t glDeleteTextures;
  //glDepthFunc_t glDepthFunc;
  //glDepthMask_t glDepthMask;
  glDepthRangef_t glDepthRangef;
  glDetachShader_t glDetachShader;
  //glDisable_t glDisable;
  glDisableVertexAttribArray_t glDisableVertexAttribArray;
  //glDrawArrays_t glDrawArrays;
  //glDrawElements_t glDrawElements;
  //glEnable_t glEnable;
  glEnableVertexAttribArray_t glEnableVertexAttribArray;
  //glFinish_t glFinish;
  //glFlush_t glFlush;
  glFramebufferRenderbuffer_t glFramebufferRenderbuffer;
  glFramebufferTexture2D_t glFramebufferTexture2D;
  //glFrontFace_t glFrontFace;
  glGenBuffers_t glGenBuffers;
  glGenerateMipmap_t glGenerateMipmap;
  glGenFramebuffers_t glGenFramebuffers;
  glGenRenderbuffers_t glGenRenderbuffers;
  //glGenTextures_t glGenTextures;
  glGetActiveAttrib_t glGetActiveAttrib;
  glGetActiveUniform_t glGetActiveUniform;
  glGetAttachedShaders_t glGetAttachedShaders;
  glGetAttribLocation_t glGetAttribLocation;
  //glGetBooleanv_t glGetBooleanv;
  glGetBufferParameteriv_t glGetBufferParameteriv;
  //glGetError_t glGetError;
  //glGetFloatv_t glGetFloatv;
  glGetFramebufferAttachmentParameteriv_t glGetFramebufferAttachmentParameteriv;
  //glGetIntegerv_t glGetIntegerv;
  glGetProgramiv_t glGetProgramiv;
  glGetProgramInfoLog_t glGetProgramInfoLog;
  glGetRenderbufferParameteriv_t glGetRenderbufferParameteriv;
  glGetShaderiv_t glGetShaderiv;
  glGetShaderInfoLog_t glGetShaderInfoLog;
  glGetShaderPrecisionFormat_t glGetShaderPrecisionFormat;
  glGetShaderSource_t glGetShaderSource;
  //glGetString_t glGetString;
  //glGetTexParameterfv_t glGetTexParameterfv;
  //glGetTexParameteriv_t glGetTexParameteriv;
  glGetUniformfv_t glGetUniformfv;
  glGetUniformiv_t glGetUniformiv;
  glGetUniformLocation_t glGetUniformLocation;
  glGetVertexAttribfv_t glGetVertexAttribfv;
  glGetVertexAttribiv_t glGetVertexAttribiv;
  glGetVertexAttribPointerv_t glGetVertexAttribPointerv;
  //glHint_t glHint;
  glIsBuffer_t glIsBuffer;
  //glIsEnabled_t glIsEnabled;
  glIsFramebuffer_t glIsFramebuffer;
  glIsProgram_t glIsProgram;
  glIsRenderbuffer_t glIsRenderbuffer;
  glIsShader_t glIsShader;
  //glIsTexture_t glIsTexture;
  //glLineWidth_t glLineWidth;
  glLinkProgram_t glLinkProgram;
  //glPixelStorei_t glPixelStorei;
  //glPolygonOffset_t glPolygonOffset;
  //glReadPixels_t glReadPixels;
  glReleaseShaderCompiler_t glReleaseShaderCompiler;
  glRenderbufferStorage_t glRenderbufferStorage;
  glSampleCoverage_t glSampleCoverage;
  //glScissor_t glScissor;
  glShaderBinary_t glShaderBinary;
  glShaderSource_t glShaderSource;
  //glStencilFunc_t glStencilFunc;
  glStencilFuncSeparate_t glStencilFuncSeparate;
  //glStencilMask_t glStencilMask;
  glStencilMaskSeparate_t glStencilMaskSeparate;
  //glStencilOp_t glStencilOp;
  glStencilOpSeparate_t glStencilOpSeparate;
  //glTexImage2D_t glTexImage2D;
  //glTexParameterf_t glTexParameterf;
  //glTexParameterfv_t glTexParameterfv;
  //glTexParameteri_t glTexParameteri;
  //glTexParameteriv_t glTexParameteriv;
  //glTexSubImage2D_t glTexSubImage2D;
  glUniform1f_t glUniform1f;
  glUniform1fv_t glUniform1fv;
  glUniform1i_t glUniform1i;
  glUniform1iv_t glUniform1iv;
  glUniform2f_t glUniform2f;
  glUniform2fv_t glUniform2fv;
  glUniform2i_t glUniform2i;
  glUniform2iv_t glUniform2iv;
  glUniform3f_t glUniform3f;
  glUniform3fv_t glUniform3fv;
  glUniform3i_t glUniform3i;
  glUniform3iv_t glUniform3iv;
  glUniform4f_t glUniform4f;
  glUniform4fv_t glUniform4fv;
  glUniform4i_t glUniform4i;
  glUniform4iv_t glUniform4iv;
  glUniformMatrix2fv_t glUniformMatrix2fv;
  glUniformMatrix3fv_t glUniformMatrix3fv;
  glUniformMatrix4fv_t glUniformMatrix4fv;
  glUseProgram_t glUseProgram;
  glValidateProgram_t glValidateProgram;
  glVertexAttrib1f_t glVertexAttrib1f;
  glVertexAttrib1fv_t glVertexAttrib1fv;
  glVertexAttrib2f_t glVertexAttrib2f;
  glVertexAttrib2fv_t glVertexAttrib2fv;
  glVertexAttrib3f_t glVertexAttrib3f;
  glVertexAttrib3fv_t glVertexAttrib3fv;
  glVertexAttrib4f_t glVertexAttrib4f;
  glVertexAttrib4fv_t glVertexAttrib4fv;
  glVertexAttribPointer_t glVertexAttribPointer;
  //glViewport_t glViewport;

  /* OpenGL ES 3.0 */

  //glReadBuffer_t glReadBuffer;
  glDrawRangeElements_t glDrawRangeElements;
  glTexImage3D_t glTexImage3D;
  glTexSubImage3D_t glTexSubImage3D;
  glCopyTexSubImage3D_t glCopyTexSubImage3D;
  glCompressedTexImage3D_t glCompressedTexImage3D;
  glCompressedTexSubImage3D_t glCompressedTexSubImage3D;
  glGenQueries_t glGenQueries;
  glDeleteQueries_t glDeleteQueries;
  glIsQuery_t glIsQuery;
  glBeginQuery_t glBeginQuery;
  glEndQuery_t glEndQuery;
  glGetQueryiv_t glGetQueryiv;
  glGetQueryObjectuiv_t glGetQueryObjectuiv;
  glUnmapBuffer_t glUnmapBuffer;
  glGetBufferPointerv_t glGetBufferPointerv;
  glDrawBuffers_t glDrawBuffers;
  glUniformMatrix2x3fv_t glUniformMatrix2x3fv;
  glUniformMatrix3x2fv_t glUniformMatrix3x2fv;
  glUniformMatrix2x4fv_t glUniformMatrix2x4fv;
  glUniformMatrix4x2fv_t glUniformMatrix4x2fv;
  glUniformMatrix3x4fv_t glUniformMatrix3x4fv;
  glUniformMatrix4x3fv_t glUniformMatrix4x3fv;
  glBlitFramebuffer_t glBlitFramebuffer;
  glRenderbufferStorageMultisample_t glRenderbufferStorageMultisample;
  glFramebufferTextureLayer_t glFramebufferTextureLayer;
  glMapBufferRange_t glMapBufferRange;
  glFlushMappedBufferRange_t glFlushMappedBufferRange;
  glBindVertexArray_t glBindVertexArray;
  glDeleteVertexArrays_t glDeleteVertexArrays;
  glGenVertexArrays_t glGenVertexArrays;
  glIsVertexArray_t glIsVertexArray;
  glGetIntegeri_v_t glGetIntegeri_v;
  glBeginTransformFeedback_t glBeginTransformFeedback;
  glEndTransformFeedback_t glEndTransformFeedback;
  glBindBufferRange_t glBindBufferRange;
  glBindBufferBase_t glBindBufferBase;
  glTransformFeedbackVaryings_t glTransformFeedbackVaryings;
  glGetTransformFeedbackVarying_t glGetTransformFeedbackVarying;
  glVertexAttribIPointer_t glVertexAttribIPointer;
  glGetVertexAttribIiv_t glGetVertexAttribIiv;
  glGetVertexAttribIuiv_t glGetVertexAttribIuiv;
  glVertexAttribI4i_t glVertexAttribI4i;
  glVertexAttribI4ui_t glVertexAttribI4ui;
  glVertexAttribI4iv_t glVertexAttribI4iv;
  glVertexAttribI4uiv_t glVertexAttribI4uiv;
  glGetUniformuiv_t glGetUniformuiv;
  glGetFragDataLocation_t glGetFragDataLocation;
  glUniform1ui_t glUniform1ui;
  glUniform2ui_t glUniform2ui;
  glUniform3ui_t glUniform3ui;
  glUniform4ui_t glUniform4ui;
  glUniform1uiv_t glUniform1uiv;
  glUniform2uiv_t glUniform2uiv;
  glUniform3uiv_t glUniform3uiv;
  glUniform4uiv_t glUniform4uiv;
  glClearBufferiv_t glClearBufferiv;
  glClearBufferuiv_t glClearBufferuiv;
  glClearBufferfv_t glClearBufferfv;
  glClearBufferfi_t glClearBufferfi;
  glGetStringi_t glGetStringi;
  glCopyBufferSubData_t glCopyBufferSubData;
  glGetUniformIndices_t glGetUniformIndices;
  glGetActiveUniformsiv_t glGetActiveUniformsiv;
  glGetUniformBlockIndex_t glGetUniformBlockIndex;
  glGetActiveUniformBlockiv_t glGetActiveUniformBlockiv;
  glGetActiveUniformBlockName_t glGetActiveUniformBlockName;
  glUniformBlockBinding_t glUniformBlockBinding;
  glDrawArraysInstanced_t glDrawArraysInstanced;
  glDrawElementsInstanced_t glDrawElementsInstanced;
  glFenceSync_t glFenceSync;
  glIsSync_t glIsSync;
  glDeleteSync_t glDeleteSync;
  glClientWaitSync_t glClientWaitSync;
  glWaitSync_t glWaitSync;
  glGetInteger64v_t glGetInteger64v;
  glGetSynciv_t glGetSynciv;
  glGetInteger64i_v_t glGetInteger64i_v;
  glGetBufferParameteri64v_t glGetBufferParameteri64v;
  glGenSamplers_t glGenSamplers;
  glDeleteSamplers_t glDeleteSamplers;
  glIsSampler_t glIsSampler;
  glBindSampler_t glBindSampler;
  glSamplerParameteri_t glSamplerParameteri;
  glSamplerParameteriv_t glSamplerParameteriv;
  glSamplerParameterf_t glSamplerParameterf;
  glSamplerParameterfv_t glSamplerParameterfv;
  glGetSamplerParameteriv_t glGetSamplerParameteriv;
  glGetSamplerParameterfv_t glGetSamplerParameterfv;
  glVertexAttribDivisor_t glVertexAttribDivisor;
  glBindTransformFeedback_t glBindTransformFeedback;
  glDeleteTransformFeedbacks_t glDeleteTransformFeedbacks;
  glGenTransformFeedbacks_t glGenTransformFeedbacks;
  glIsTransformFeedback_t glIsTransformFeedback;
  glPauseTransformFeedback_t glPauseTransformFeedback;
  glResumeTransformFeedback_t glResumeTransformFeedback;
  glGetProgramBinary_t glGetProgramBinary;
  glProgramBinary_t glProgramBinary;
  glProgramParameteri_t glProgramParameteri;
  glInvalidateFramebuffer_t glInvalidateFramebuffer;
  glInvalidateSubFramebuffer_t glInvalidateSubFramebuffer;
  glTexStorage2D_t glTexStorage2D;
  glTexStorage3D_t glTexStorage3D;
  glGetInternalformativ_t glGetInternalformativ;



  void *get_proc_address(int &num_checked, int &num_ok, const char *name) {
    void *res = wglGetProcAddress(name);
    num_checked++;
    if (res) num_ok++;
    return res;
  }

  void init_wgl() {
    int num_checked = 0;
    int num_ok = 0;
    // all the functions in gl3.h
    glActiveTexture = (glActiveTexture_t)get_proc_address(num_checked, num_ok, "glActiveTexture");
    glAttachShader = (glAttachShader_t)get_proc_address(num_checked, num_ok, "glAttachShader");
    glBindAttribLocation = (glBindAttribLocation_t)get_proc_address(num_checked, num_ok, "glBindAttribLocation");
    glBindBuffer = (glBindBuffer_t)get_proc_address(num_checked, num_ok, "glBindBuffer");
    glBindFramebuffer = (glBindFramebuffer_t)get_proc_address(num_checked, num_ok, "glBindFramebuffer");
    glBindRenderbuffer = (glBindRenderbuffer_t)get_proc_address(num_checked, num_ok, "glBindRenderbuffer");
    //glBindTexture = (glBindTexture_t)get_proc_address(num_checked, num_ok, "glBindTexture");
    glBlendColor = (glBlendColor_t)get_proc_address(num_checked, num_ok, "glBlendColor");
    glBlendEquation = (glBlendEquation_t)get_proc_address(num_checked, num_ok, "glBlendEquation");
    glBlendEquationSeparate = (glBlendEquationSeparate_t)get_proc_address(num_checked, num_ok, "glBlendEquationSeparate");
    //glBlendFunc = (glBlendFunc_t)get_proc_address(num_checked, num_ok, "glBlendFunc");
    glBlendFuncSeparate = (glBlendFuncSeparate_t)get_proc_address(num_checked, num_ok, "glBlendFuncSeparate");
    glBufferData = (glBufferData_t)get_proc_address(num_checked, num_ok, "glBufferData");
    glBufferSubData = (glBufferSubData_t)get_proc_address(num_checked, num_ok, "glBufferSubData");
    glCheckFramebufferStatus = (glCheckFramebufferStatus_t)get_proc_address(num_checked, num_ok, "glCheckFramebufferStatus");
    //glClear = (glClear_t)get_proc_address(num_checked, num_ok, "glClear");
    //glClearColor = (glClearColor_t)get_proc_address(num_checked, num_ok, "glClearColor");
    glClearDepthf = (glClearDepthf_t)get_proc_address(num_checked, num_ok, "glClearDepthf");
    //glClearStencil = (glClearStencil_t)get_proc_address(num_checked, num_ok, "glClearStencil");
    //glColorMask = (glColorMask_t)get_proc_address(num_checked, num_ok, "glColorMask");
    glCompileShader = (glCompileShader_t)get_proc_address(num_checked, num_ok, "glCompileShader");
    glCompressedTexImage2D = (glCompressedTexImage2D_t)get_proc_address(num_checked, num_ok, "glCompressedTexImage2D");
    glCompressedTexSubImage2D = (glCompressedTexSubImage2D_t)get_proc_address(num_checked, num_ok, "glCompressedTexSubImage2D");
    //glCopyTexImage2D = (glCopyTexImage2D_t)get_proc_address(num_checked, num_ok, "glCopyTexImage2D");
    //glCopyTexSubImage2D = (glCopyTexSubImage2D_t)get_proc_address(num_checked, num_ok, "glCopyTexSubImage2D");
    glCreateProgram = (glCreateProgram_t)get_proc_address(num_checked, num_ok, "glCreateProgram");
    glCreateShader = (glCreateShader_t)get_proc_address(num_checked, num_ok, "glCreateShader");
    //glCullFace = (glCullFace_t)get_proc_address(num_checked, num_ok, "glCullFace");
    glDeleteBuffers = (glDeleteBuffers_t)get_proc_address(num_checked, num_ok, "glDeleteBuffers");
    glDeleteFramebuffers = (glDeleteFramebuffers_t)get_proc_address(num_checked, num_ok, "glDeleteFramebuffers");
    glDeleteProgram = (glDeleteProgram_t)get_proc_address(num_checked, num_ok, "glDeleteProgram");
    glDeleteRenderbuffers = (glDeleteRenderbuffers_t)get_proc_address(num_checked, num_ok, "glDeleteRenderbuffers");
    glDeleteShader = (glDeleteShader_t)get_proc_address(num_checked, num_ok, "glDeleteShader");
    //glDeleteTextures = (glDeleteTextures_t)get_proc_address(num_checked, num_ok, "glDeleteTextures");
    //glDepthFunc = (glDepthFunc_t)get_proc_address(num_checked, num_ok, "glDepthFunc");
    //glDepthMask = (glDepthMask_t)get_proc_address(num_checked, num_ok, "glDepthMask");
    glDepthRangef = (glDepthRangef_t)get_proc_address(num_checked, num_ok, "glDepthRangef");
    glDetachShader = (glDetachShader_t)get_proc_address(num_checked, num_ok, "glDetachShader");
    //glDisable = (glDisable_t)get_proc_address(num_checked, num_ok, "glDisable");
    glDisableVertexAttribArray = (glDisableVertexAttribArray_t)get_proc_address(num_checked, num_ok, "glDisableVertexAttribArray");
    //glDrawArrays = (glDrawArrays_t)get_proc_address(num_checked, num_ok, "glDrawArrays");
    //glDrawElements = (glDrawElements_t)get_proc_address(num_checked, num_ok, "glDrawElements");
    //glEnable = (glEnable_t)get_proc_address(num_checked, num_ok, "glEnable");
    glEnableVertexAttribArray = (glEnableVertexAttribArray_t)get_proc_address(num_checked, num_ok, "glEnableVertexAttribArray");
    //glFinish = (glFinish_t)get_proc_address(num_checked, num_ok, "glFinish");
    //glFlush = (glFlush_t)get_proc_address(num_checked, num_ok, "glFlush");
    glFramebufferRenderbuffer = (glFramebufferRenderbuffer_t)get_proc_address(num_checked, num_ok, "glFramebufferRenderbuffer");
    glFramebufferTexture2D = (glFramebufferTexture2D_t)get_proc_address(num_checked, num_ok, "glFramebufferTexture2D");
    //glFrontFace = (glFrontFace_t)get_proc_address(num_checked, num_ok, "glFrontFace");
    glGenBuffers = (glGenBuffers_t)get_proc_address(num_checked, num_ok, "glGenBuffers");
    glGenerateMipmap = (glGenerateMipmap_t)get_proc_address(num_checked, num_ok, "glGenerateMipmap");
    glGenFramebuffers = (glGenFramebuffers_t)get_proc_address(num_checked, num_ok, "glGenFramebuffers");
    glGenRenderbuffers = (glGenRenderbuffers_t)get_proc_address(num_checked, num_ok, "glGenRenderbuffers");
    //glGenTextures = (glGenTextures_t)get_proc_address(num_checked, num_ok, "glGenTextures");
    glGetActiveAttrib = (glGetActiveAttrib_t)get_proc_address(num_checked, num_ok, "glGetActiveAttrib");
    glGetActiveUniform = (glGetActiveUniform_t)get_proc_address(num_checked, num_ok, "glGetActiveUniform");
    glGetAttachedShaders = (glGetAttachedShaders_t)get_proc_address(num_checked, num_ok, "glGetAttachedShaders");
    glGetAttribLocation = (glGetAttribLocation_t)get_proc_address(num_checked, num_ok, "glGetAttribLocation");
    //glGetBooleanv = (glGetBooleanv_t)get_proc_address(num_checked, num_ok, "glGetBooleanv");
    glGetBufferParameteriv = (glGetBufferParameteriv_t)get_proc_address(num_checked, num_ok, "glGetBufferParameteriv");
    //glGetError = (glGetError_t)get_proc_address(num_checked, num_ok, "glGetError");
    //glGetFloatv = (glGetFloatv_t)get_proc_address(num_checked, num_ok, "glGetFloatv");
    glGetFramebufferAttachmentParameteriv = (glGetFramebufferAttachmentParameteriv_t)get_proc_address(num_checked, num_ok, "glGetFramebufferAttachmentParameteriv");
    //glGetIntegerv = (glGetIntegerv_t)get_proc_address(num_checked, num_ok, "glGetIntegerv");
    glGetProgramiv = (glGetProgramiv_t)get_proc_address(num_checked, num_ok, "glGetProgramiv");
    glGetProgramInfoLog = (glGetProgramInfoLog_t)get_proc_address(num_checked, num_ok, "glGetProgramInfoLog");
    glGetRenderbufferParameteriv = (glGetRenderbufferParameteriv_t)get_proc_address(num_checked, num_ok, "glGetRenderbufferParameteriv");
    glGetShaderiv = (glGetShaderiv_t)get_proc_address(num_checked, num_ok, "glGetShaderiv");
    glGetShaderInfoLog = (glGetShaderInfoLog_t)get_proc_address(num_checked, num_ok, "glGetShaderInfoLog");
    glGetShaderPrecisionFormat = (glGetShaderPrecisionFormat_t)get_proc_address(num_checked, num_ok, "glGetShaderPrecisionFormat");
    glGetShaderSource = (glGetShaderSource_t)get_proc_address(num_checked, num_ok, "glGetShaderSource");
    //glGetString = (glGetString_t)get_proc_address(num_checked, num_ok, "glGetString");
    //glGetTexParameterfv = (glGetTexParameterfv_t)get_proc_address(num_checked, num_ok, "glGetTexParameterfv");
    //glGetTexParameteriv = (glGetTexParameteriv_t)get_proc_address(num_checked, num_ok, "glGetTexParameteriv");
    glGetUniformfv = (glGetUniformfv_t)get_proc_address(num_checked, num_ok, "glGetUniformfv");
    glGetUniformiv = (glGetUniformiv_t)get_proc_address(num_checked, num_ok, "glGetUniformiv");
    glGetUniformLocation = (glGetUniformLocation_t)get_proc_address(num_checked, num_ok, "glGetUniformLocation");
    glGetVertexAttribfv = (glGetVertexAttribfv_t)get_proc_address(num_checked, num_ok, "glGetVertexAttribfv");
    glGetVertexAttribiv = (glGetVertexAttribiv_t)get_proc_address(num_checked, num_ok, "glGetVertexAttribiv");
    glGetVertexAttribPointerv = (glGetVertexAttribPointerv_t)get_proc_address(num_checked, num_ok, "glGetVertexAttribPointerv");
    //glHint = (glHint_t)get_proc_address(num_checked, num_ok, "glHint");
    glIsBuffer = (glIsBuffer_t)get_proc_address(num_checked, num_ok, "glIsBuffer");
    //glIsEnabled = (glIsEnabled_t)get_proc_address(num_checked, num_ok, "glIsEnabled");
    glIsFramebuffer = (glIsFramebuffer_t)get_proc_address(num_checked, num_ok, "glIsFramebuffer");
    glIsProgram = (glIsProgram_t)get_proc_address(num_checked, num_ok, "glIsProgram");
    glIsRenderbuffer = (glIsRenderbuffer_t)get_proc_address(num_checked, num_ok, "glIsRenderbuffer");
    glIsShader = (glIsShader_t)get_proc_address(num_checked, num_ok, "glIsShader");
    //glIsTexture = (glIsTexture_t)get_proc_address(num_checked, num_ok, "glIsTexture");
    //glLineWidth = (glLineWidth_t)get_proc_address(num_checked, num_ok, "glLineWidth");
    glLinkProgram = (glLinkProgram_t)get_proc_address(num_checked, num_ok, "glLinkProgram");
    //glPixelStorei = (glPixelStorei_t)get_proc_address(num_checked, num_ok, "glPixelStorei");
    //glPolygonOffset = (glPolygonOffset_t)get_proc_address(num_checked, num_ok, "glPolygonOffset");
    //glReadPixels = (glReadPixels_t)get_proc_address(num_checked, num_ok, "glReadPixels");
    glReleaseShaderCompiler = (glReleaseShaderCompiler_t)get_proc_address(num_checked, num_ok, "glReleaseShaderCompiler");
    glRenderbufferStorage = (glRenderbufferStorage_t)get_proc_address(num_checked, num_ok, "glRenderbufferStorage");
    glSampleCoverage = (glSampleCoverage_t)get_proc_address(num_checked, num_ok, "glSampleCoverage");
    //glScissor = (glScissor_t)get_proc_address(num_checked, num_ok, "glScissor");
    glShaderBinary = (glShaderBinary_t)get_proc_address(num_checked, num_ok, "glShaderBinary");
    glShaderSource = (glShaderSource_t)get_proc_address(num_checked, num_ok, "glShaderSource");
    //glStencilFunc = (glStencilFunc_t)get_proc_address(num_checked, num_ok, "glStencilFunc");
    glStencilFuncSeparate = (glStencilFuncSeparate_t)get_proc_address(num_checked, num_ok, "glStencilFuncSeparate");
    //glStencilMask = (glStencilMask_t)get_proc_address(num_checked, num_ok, "glStencilMask");
    glStencilMaskSeparate = (glStencilMaskSeparate_t)get_proc_address(num_checked, num_ok, "glStencilMaskSeparate");
    //glStencilOp = (glStencilOp_t)get_proc_address(num_checked, num_ok, "glStencilOp");
    glStencilOpSeparate = (glStencilOpSeparate_t)get_proc_address(num_checked, num_ok, "glStencilOpSeparate");
    //glTexImage2D = (glTexImage2D_t)get_proc_address(num_checked, num_ok, "glTexImage2D");
    //glTexParameterf = (glTexParameterf_t)get_proc_address(num_checked, num_ok, "glTexParameterf");
    //glTexParameterfv = (glTexParameterfv_t)get_proc_address(num_checked, num_ok, "glTexParameterfv");
    //glTexParameteri = (glTexParameteri_t)get_proc_address(num_checked, num_ok, "glTexParameteri");
    //glTexParameteriv = (glTexParameteriv_t)get_proc_address(num_checked, num_ok, "glTexParameteriv");
    //glTexSubImage2D = (glTexSubImage2D_t)get_proc_address(num_checked, num_ok, "glTexSubImage2D");
    glUniform1f = (glUniform1f_t)get_proc_address(num_checked, num_ok, "glUniform1f");
    glUniform1fv = (glUniform1fv_t)get_proc_address(num_checked, num_ok, "glUniform1fv");
    glUniform1i = (glUniform1i_t)get_proc_address(num_checked, num_ok, "glUniform1i");
    glUniform1iv = (glUniform1iv_t)get_proc_address(num_checked, num_ok, "glUniform1iv");
    glUniform2f = (glUniform2f_t)get_proc_address(num_checked, num_ok, "glUniform2f");
    glUniform2fv = (glUniform2fv_t)get_proc_address(num_checked, num_ok, "glUniform2fv");
    glUniform2i = (glUniform2i_t)get_proc_address(num_checked, num_ok, "glUniform2i");
    glUniform2iv = (glUniform2iv_t)get_proc_address(num_checked, num_ok, "glUniform2iv");
    glUniform3f = (glUniform3f_t)get_proc_address(num_checked, num_ok, "glUniform3f");
    glUniform3fv = (glUniform3fv_t)get_proc_address(num_checked, num_ok, "glUniform3fv");
    glUniform3i = (glUniform3i_t)get_proc_address(num_checked, num_ok, "glUniform3i");
    glUniform3iv = (glUniform3iv_t)get_proc_address(num_checked, num_ok, "glUniform3iv");
    glUniform4f = (glUniform4f_t)get_proc_address(num_checked, num_ok, "glUniform4f");
    glUniform4fv = (glUniform4fv_t)get_proc_address(num_checked, num_ok, "glUniform4fv");
    glUniform4i = (glUniform4i_t)get_proc_address(num_checked, num_ok, "glUniform4i");
    glUniform4iv = (glUniform4iv_t)get_proc_address(num_checked, num_ok, "glUniform4iv");
    glUniformMatrix2fv = (glUniformMatrix2fv_t)get_proc_address(num_checked, num_ok, "glUniformMatrix2fv");
    glUniformMatrix3fv = (glUniformMatrix3fv_t)get_proc_address(num_checked, num_ok, "glUniformMatrix3fv");
    glUniformMatrix4fv = (glUniformMatrix4fv_t)get_proc_address(num_checked, num_ok, "glUniformMatrix4fv");
    glUseProgram = (glUseProgram_t)get_proc_address(num_checked, num_ok, "glUseProgram");
    glValidateProgram = (glValidateProgram_t)get_proc_address(num_checked, num_ok, "glValidateProgram");
    glVertexAttrib1f = (glVertexAttrib1f_t)get_proc_address(num_checked, num_ok, "glVertexAttrib1f");
    glVertexAttrib1fv = (glVertexAttrib1fv_t)get_proc_address(num_checked, num_ok, "glVertexAttrib1fv");
    glVertexAttrib2f = (glVertexAttrib2f_t)get_proc_address(num_checked, num_ok, "glVertexAttrib2f");
    glVertexAttrib2fv = (glVertexAttrib2fv_t)get_proc_address(num_checked, num_ok, "glVertexAttrib2fv");
    glVertexAttrib3f = (glVertexAttrib3f_t)get_proc_address(num_checked, num_ok, "glVertexAttrib3f");
    glVertexAttrib3fv = (glVertexAttrib3fv_t)get_proc_address(num_checked, num_ok, "glVertexAttrib3fv");
    glVertexAttrib4f = (glVertexAttrib4f_t)get_proc_address(num_checked, num_ok, "glVertexAttrib4f");
    glVertexAttrib4fv = (glVertexAttrib4fv_t)get_proc_address(num_checked, num_ok, "glVertexAttrib4fv");
    glVertexAttribPointer = (glVertexAttribPointer_t)get_proc_address(num_checked, num_ok, "glVertexAttribPointer");
    //glViewport = (glViewport_t)get_proc_address(num_checked, num_ok, "glViewport");

    //printf("nc/ok=%d/%d\n", num_checked, num_ok);
    //num_checked = num_ok = 0;

    /* OpenGL ES 3.0 */

    //glReadBuffer = (glReadBuffer_t)get_proc_address(num_checked, num_ok, "glReadBuffer");
    glDrawRangeElements = (glDrawRangeElements_t)get_proc_address(num_checked, num_ok, "glDrawRangeElements");
    glTexImage3D = (glTexImage3D_t)get_proc_address(num_checked, num_ok, "glTexImage3D");
    glTexSubImage3D = (glTexSubImage3D_t)get_proc_address(num_checked, num_ok, "glTexSubImage3D");
    glCopyTexSubImage3D = (glCopyTexSubImage3D_t)get_proc_address(num_checked, num_ok, "glCopyTexSubImage3D");
    glCompressedTexImage3D = (glCompressedTexImage3D_t)get_proc_address(num_checked, num_ok, "glCompressedTexImage3D");
    glCompressedTexSubImage3D = (glCompressedTexSubImage3D_t)get_proc_address(num_checked, num_ok, "glCompressedTexSubImage3D");
    glGenQueries = (glGenQueries_t)get_proc_address(num_checked, num_ok, "glGenQueries");
    glDeleteQueries = (glDeleteQueries_t)get_proc_address(num_checked, num_ok, "glDeleteQueries");
    glIsQuery = (glIsQuery_t)get_proc_address(num_checked, num_ok, "glIsQuery");
    glBeginQuery = (glBeginQuery_t)get_proc_address(num_checked, num_ok, "glBeginQuery");
    glEndQuery = (glEndQuery_t)get_proc_address(num_checked, num_ok, "glEndQuery");
    glGetQueryiv = (glGetQueryiv_t)get_proc_address(num_checked, num_ok, "glGetQueryiv");
    glGetQueryObjectuiv = (glGetQueryObjectuiv_t)get_proc_address(num_checked, num_ok, "glGetQueryObjectuiv");
    glUnmapBuffer = (glUnmapBuffer_t)get_proc_address(num_checked, num_ok, "glUnmapBuffer");
    glGetBufferPointerv = (glGetBufferPointerv_t)get_proc_address(num_checked, num_ok, "glGetBufferPointerv");
    glDrawBuffers = (glDrawBuffers_t)get_proc_address(num_checked, num_ok, "glDrawBuffers");
    glUniformMatrix2x3fv = (glUniformMatrix2x3fv_t)get_proc_address(num_checked, num_ok, "glUniformMatrix2x3fv");
    glUniformMatrix3x2fv = (glUniformMatrix3x2fv_t)get_proc_address(num_checked, num_ok, "glUniformMatrix3x2fv");
    glUniformMatrix2x4fv = (glUniformMatrix2x4fv_t)get_proc_address(num_checked, num_ok, "glUniformMatrix2x4fv");
    glUniformMatrix4x2fv = (glUniformMatrix4x2fv_t)get_proc_address(num_checked, num_ok, "glUniformMatrix4x2fv");
    glUniformMatrix3x4fv = (glUniformMatrix3x4fv_t)get_proc_address(num_checked, num_ok, "glUniformMatrix3x4fv");
    glUniformMatrix4x3fv = (glUniformMatrix4x3fv_t)get_proc_address(num_checked, num_ok, "glUniformMatrix4x3fv");
    glBlitFramebuffer = (glBlitFramebuffer_t)get_proc_address(num_checked, num_ok, "glBlitFramebuffer");
    glRenderbufferStorageMultisample = (glRenderbufferStorageMultisample_t)get_proc_address(num_checked, num_ok, "glRenderbufferStorageMultisample");
    glFramebufferTextureLayer = (glFramebufferTextureLayer_t)get_proc_address(num_checked, num_ok, "glFramebufferTextureLayer");
    glMapBufferRange = (glMapBufferRange_t)get_proc_address(num_checked, num_ok, "glMapBufferRange");
    glFlushMappedBufferRange = (glFlushMappedBufferRange_t)get_proc_address(num_checked, num_ok, "glFlushMappedBufferRange");
    glBindVertexArray = (glBindVertexArray_t)get_proc_address(num_checked, num_ok, "glBindVertexArray");
    glDeleteVertexArrays = (glDeleteVertexArrays_t)get_proc_address(num_checked, num_ok, "glDeleteVertexArrays");
    glGenVertexArrays = (glGenVertexArrays_t)get_proc_address(num_checked, num_ok, "glGenVertexArrays");
    glIsVertexArray = (glIsVertexArray_t)get_proc_address(num_checked, num_ok, "glIsVertexArray");
    glGetIntegeri_v = (glGetIntegeri_v_t)get_proc_address(num_checked, num_ok, "glGetIntegeri_v");
    glBeginTransformFeedback = (glBeginTransformFeedback_t)get_proc_address(num_checked, num_ok, "glBeginTransformFeedback");
    glEndTransformFeedback = (glEndTransformFeedback_t)get_proc_address(num_checked, num_ok, "glEndTransformFeedback");
    glBindBufferRange = (glBindBufferRange_t)get_proc_address(num_checked, num_ok, "glBindBufferRange");
    glBindBufferBase = (glBindBufferBase_t)get_proc_address(num_checked, num_ok, "glBindBufferBase");
    glTransformFeedbackVaryings = (glTransformFeedbackVaryings_t)get_proc_address(num_checked, num_ok, "glTransformFeedbackVaryings");
    glGetTransformFeedbackVarying = (glGetTransformFeedbackVarying_t)get_proc_address(num_checked, num_ok, "glGetTransformFeedbackVarying");
    glVertexAttribIPointer = (glVertexAttribIPointer_t)get_proc_address(num_checked, num_ok, "glVertexAttribIPointer");
    glGetVertexAttribIiv = (glGetVertexAttribIiv_t)get_proc_address(num_checked, num_ok, "glGetVertexAttribIiv");
    glGetVertexAttribIuiv = (glGetVertexAttribIuiv_t)get_proc_address(num_checked, num_ok, "glGetVertexAttribIuiv");
    glVertexAttribI4i = (glVertexAttribI4i_t)get_proc_address(num_checked, num_ok, "glVertexAttribI4i");
    glVertexAttribI4ui = (glVertexAttribI4ui_t)get_proc_address(num_checked, num_ok, "glVertexAttribI4ui");
    glVertexAttribI4iv = (glVertexAttribI4iv_t)get_proc_address(num_checked, num_ok, "glVertexAttribI4iv");
    glVertexAttribI4uiv = (glVertexAttribI4uiv_t)get_proc_address(num_checked, num_ok, "glVertexAttribI4uiv");
    glGetUniformuiv = (glGetUniformuiv_t)get_proc_address(num_checked, num_ok, "glGetUniformuiv");
    glGetFragDataLocation = (glGetFragDataLocation_t)get_proc_address(num_checked, num_ok, "glGetFragDataLocation");
    glUniform1ui = (glUniform1ui_t)get_proc_address(num_checked, num_ok, "glUniform1ui");
    glUniform2ui = (glUniform2ui_t)get_proc_address(num_checked, num_ok, "glUniform2ui");
    glUniform3ui = (glUniform3ui_t)get_proc_address(num_checked, num_ok, "glUniform3ui");
    glUniform4ui = (glUniform4ui_t)get_proc_address(num_checked, num_ok, "glUniform4ui");
    glUniform1uiv = (glUniform1uiv_t)get_proc_address(num_checked, num_ok, "glUniform1uiv");
    glUniform2uiv = (glUniform2uiv_t)get_proc_address(num_checked, num_ok, "glUniform2uiv");
    glUniform3uiv = (glUniform3uiv_t)get_proc_address(num_checked, num_ok, "glUniform3uiv");
    glUniform4uiv = (glUniform4uiv_t)get_proc_address(num_checked, num_ok, "glUniform4uiv");
    glClearBufferiv = (glClearBufferiv_t)get_proc_address(num_checked, num_ok, "glClearBufferiv");
    glClearBufferuiv = (glClearBufferuiv_t)get_proc_address(num_checked, num_ok, "glClearBufferuiv");
    glClearBufferfv = (glClearBufferfv_t)get_proc_address(num_checked, num_ok, "glClearBufferfv");
    glClearBufferfi = (glClearBufferfi_t)get_proc_address(num_checked, num_ok, "glClearBufferfi");
    glGetStringi = (glGetStringi_t)get_proc_address(num_checked, num_ok, "glGetStringi");
    glCopyBufferSubData = (glCopyBufferSubData_t)get_proc_address(num_checked, num_ok, "glCopyBufferSubData");
    glGetUniformIndices = (glGetUniformIndices_t)get_proc_address(num_checked, num_ok, "glGetUniformIndices");
    glGetActiveUniformsiv = (glGetActiveUniformsiv_t)get_proc_address(num_checked, num_ok, "glGetActiveUniformsiv");
    glGetUniformBlockIndex = (glGetUniformBlockIndex_t)get_proc_address(num_checked, num_ok, "glGetUniformBlockIndex");
    glGetActiveUniformBlockiv = (glGetActiveUniformBlockiv_t)get_proc_address(num_checked, num_ok, "glGetActiveUniformBlockiv");
    glGetActiveUniformBlockName = (glGetActiveUniformBlockName_t)get_proc_address(num_checked, num_ok, "glGetActiveUniformBlockName");
    glUniformBlockBinding = (glUniformBlockBinding_t)get_proc_address(num_checked, num_ok, "glUniformBlockBinding");
    glDrawArraysInstanced = (glDrawArraysInstanced_t)get_proc_address(num_checked, num_ok, "glDrawArraysInstanced");
    glDrawElementsInstanced = (glDrawElementsInstanced_t)get_proc_address(num_checked, num_ok, "glDrawElementsInstanced");
    glFenceSync = (glFenceSync_t)get_proc_address(num_checked, num_ok, "glFenceSync");
    glIsSync = (glIsSync_t)get_proc_address(num_checked, num_ok, "glIsSync");
    glDeleteSync = (glDeleteSync_t)get_proc_address(num_checked, num_ok, "glDeleteSync");
    glClientWaitSync = (glClientWaitSync_t)get_proc_address(num_checked, num_ok, "glClientWaitSync");
    glWaitSync = (glWaitSync_t)get_proc_address(num_checked, num_ok, "glWaitSync");
    glGetInteger64v = (glGetInteger64v_t)get_proc_address(num_checked, num_ok, "glGetInteger64v");
    glGetSynciv = (glGetSynciv_t)get_proc_address(num_checked, num_ok, "glGetSynciv");
    glGetInteger64i_v = (glGetInteger64i_v_t)get_proc_address(num_checked, num_ok, "glGetInteger64i_v");
    glGetBufferParameteri64v = (glGetBufferParameteri64v_t)get_proc_address(num_checked, num_ok, "glGetBufferParameteri64v");
    glGenSamplers = (glGenSamplers_t)get_proc_address(num_checked, num_ok, "glGenSamplers");
    glDeleteSamplers = (glDeleteSamplers_t)get_proc_address(num_checked, num_ok, "glDeleteSamplers");
    glIsSampler = (glIsSampler_t)get_proc_address(num_checked, num_ok, "glIsSampler");
    glBindSampler = (glBindSampler_t)get_proc_address(num_checked, num_ok, "glBindSampler");
    glSamplerParameteri = (glSamplerParameteri_t)get_proc_address(num_checked, num_ok, "glSamplerParameteri");
    glSamplerParameteriv = (glSamplerParameteriv_t)get_proc_address(num_checked, num_ok, "glSamplerParameteriv");
    glSamplerParameterf = (glSamplerParameterf_t)get_proc_address(num_checked, num_ok, "glSamplerParameterf");
    glSamplerParameterfv = (glSamplerParameterfv_t)get_proc_address(num_checked, num_ok, "glSamplerParameterfv");
    glGetSamplerParameteriv = (glGetSamplerParameteriv_t)get_proc_address(num_checked, num_ok, "glGetSamplerParameteriv");
    glGetSamplerParameterfv = (glGetSamplerParameterfv_t)get_proc_address(num_checked, num_ok, "glGetSamplerParameterfv");
    glVertexAttribDivisor = (glVertexAttribDivisor_t)get_proc_address(num_checked, num_ok, "glVertexAttribDivisor");
    glBindTransformFeedback = (glBindTransformFeedback_t)get_proc_address(num_checked, num_ok, "glBindTransformFeedback");
    glDeleteTransformFeedbacks = (glDeleteTransformFeedbacks_t)get_proc_address(num_checked, num_ok, "glDeleteTransformFeedbacks");
    glGenTransformFeedbacks = (glGenTransformFeedbacks_t)get_proc_address(num_checked, num_ok, "glGenTransformFeedbacks");
    glIsTransformFeedback = (glIsTransformFeedback_t)get_proc_address(num_checked, num_ok, "glIsTransformFeedback");
    glPauseTransformFeedback = (glPauseTransformFeedback_t)get_proc_address(num_checked, num_ok, "glPauseTransformFeedback");
    glResumeTransformFeedback = (glResumeTransformFeedback_t)get_proc_address(num_checked, num_ok, "glResumeTransformFeedback");
    glGetProgramBinary = (glGetProgramBinary_t)get_proc_address(num_checked, num_ok, "glGetProgramBinary");
    glProgramBinary = (glProgramBinary_t)get_proc_address(num_checked, num_ok, "glProgramBinary");
    glProgramParameteri = (glProgramParameteri_t)get_proc_address(num_checked, num_ok, "glProgramParameteri");
    glTexStorage2D = (glTexStorage2D_t)get_proc_address(num_checked, num_ok, "glTexStorage2D");
    glTexStorage3D = (glTexStorage3D_t)get_proc_address(num_checked, num_ok, "glTexStorage3D");
    glGetInternalformativ = (glGetInternalformativ_t)get_proc_address(num_checked, num_ok, "glGetInternalformativ");

    printf("nc/ok=%d/%d\n", num_checked, num_ok); num_checked = num_ok = 0;

    glInvalidateSubFramebuffer = (glInvalidateSubFramebuffer_t)get_proc_address(num_checked, num_ok, "glInvalidateSubFramebuffer");
    glInvalidateFramebuffer = (glInvalidateFramebuffer_t)get_proc_address(num_checked, num_ok, "glInvalidateFramebuffer");

    printf("nc/ok=%d/%d\n", num_checked, num_ok); num_checked = num_ok = 0;
  }

#endif
