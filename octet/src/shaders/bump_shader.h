////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Phong shader that uses textures for all material channels
// This shader has only one light which 

class bump_shader : public phong_shader {
public:
  void init() {
    // this is the vertex shader.
    // it is called for each corner of each triangle
    // it inputs pos and uv from each corner
    // it outputs gl_Position, normal_ and uv_ to the rasterizer
    // normal_ is the normal in camera space to make calculations easier
    const char vertex_shader[] =
      "#define lowp\n"
      "#define highp\n"
      "varying lowp vec2 uv_;"
      "varying lowp vec3 normal_;"
      "varying lowp vec3 tangent_;"
      "varying lowp vec3 bitangent_;"
      ""
      "attribute highp vec4 pos;"
      "attribute lowp vec3 normal;"
      "attribute lowp vec3 tangent;"
      "attribute lowp vec3 bitangent;"
      "attribute lowp vec2 uv;"
      ""
      "uniform mat4 modelToProjection;"
      "uniform mat4 modelToCamera;"
      ""
      "void main() {"
      "  uv_ = uv;"
      "  normal_ = (modelToCamera * vec4(normal,0)).xyz;"
      "  tangent_ = (modelToCamera * vec4(tangent,0)).xyz;"
      "  bitangent_ = (modelToCamera * vec4(bitangent,0)).xyz;"
      "  gl_Position = modelToProjection * pos;"
      "}"
    ;

    // this is the fragment shader
    // after the rasterizer breaks the triangle into fragments
    // this is called for every fragment
    // it outputs gl_FragColor, the color of the pixel and inputs normal_ and uv_
    // the four samplers give emissive, diffuse, specular and ambient colors
    const char fragment_shader[] =
      "#define lowp\n"
      "#define highp\n"
      "varying lowp vec2 uv_;"
      "varying lowp vec3 normal_;"
      "varying lowp vec3 tangent_;"
      "varying lowp vec3 bitangent_;"
      "uniform lowp vec3 light_direction;"
      "uniform lowp vec4 light_diffuse;"
      "uniform lowp vec4 light_ambient;"
      "uniform lowp vec4 light_specular;"
      "uniform sampler2D samplers[5];"
      "uniform float shininess;"
      ""
      "void main() {"
      "  lowp vec3 bump = normalize(vec3(texture2D(samplers[4], uv_).xy-vec2(0.5, 0.5), 1));"
      "  lowp vec3 nnormal = normalize(bump.x * tangent_ + bump.y * bitangent_ + bump.z * normal_);"
      "  lowp vec3 half_direction = normalize(light_direction + vec3(0, 0, 1));"
      "  lowp float diffuse_factor = max(dot(light_direction, nnormal), 0.0);"
      "  lowp float specular_factor = pow(max(dot(half_direction, nnormal), 0.0), shininess);"
      "  lowp vec4 diffuse = texture2D(samplers[0], uv_);"
      "  lowp vec4 ambient = texture2D(samplers[1], uv_);"
      "  lowp vec4 emission = texture2D(samplers[2], uv_);"
      "  lowp vec4 specular = texture2D(samplers[3], uv_);"
      "  gl_FragColor = "
      "    ambient * light_ambient +"
      "    diffuse * light_diffuse * diffuse_factor +"
      "    emission +"
      "    specular * light_specular * specular_factor;"
      //"  gl_FragColor = vec4(light_direction,1) + diffuse * 0.1f;"
      "}"
    ;
    
    // use the common shader code to compile and link the shaders
    // the result is a shader program
    phong_shader::init_uniforms(vertex_shader, fragment_shader);
  }

  void render(const mat4 &modelToProjection, const mat4 &modelToCamera, const vec4 &light_direction, float shininess, vec4 &light_ambient, vec4 &light_diffuse, vec4 &light_specular) {
    phong_shader::render(modelToProjection, modelToCamera, light_direction, shininess, light_ambient, light_diffuse, light_specular, 5);
  }
};
