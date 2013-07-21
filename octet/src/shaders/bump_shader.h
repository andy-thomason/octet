////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Phong shader that uses textures for all material channels
// This shader has only one light which 

class bump_shader : public phong_shader {
public:
  void init(bool is_skinned=false) {
    // this is the vertex shader for regular geometry
    // it is called for each corner of each triangle
    // it inputs pos and uv from each corner
    // it outputs gl_Position, normal_ and uv_ to the rasterizer
    // normal_ is the normal in camera space to make calculations easier
    const char vertex_shader[] = SHADER_STR(
      varying vec2 uv_;
      varying vec3 normal_;
      varying vec3 tangent_;
      varying vec3 bitangent_;
      
      attribute vec4 pos;
      attribute vec3 normal;
      attribute vec3 tangent;
      attribute vec3 bitangent;
      attribute vec2 uv;
      
      uniform mat4 modelToProjection;
      uniform mat4 modelToCamera;
      
      void main() {
        uv_ = uv;
        normal_ = (modelToCamera * vec4(normal,0)).xyz;
        tangent_ = (modelToCamera * vec4(tangent,0)).xyz;
        bitangent_ = (modelToCamera * vec4(bitangent,0)).xyz;
        gl_Position = modelToProjection * pos;
      }
    );

    // this is the vertex shader for skinned geometry
    // this is the shader for skinned geometry
    // it is not terribly efficient, but does the job.
    // it is generally better to use quaternions as they
    // are smaller and don't collapse joints.
    const char skinned_vertex_shader[] = SHADER_STR(
      varying vec2 uv_;
      varying vec3 normal_;
      varying vec3 tangent_;
      varying vec3 bitangent_;
      
      attribute vec4 pos;
      attribute vec3 normal;
      attribute vec3 tangent;
      attribute vec3 bitangent;
      attribute vec2 uv;
      attribute vec3 blendweight;
      attribute vec4 blendindices;
      
      uniform mat4 cameraToProjection;
      uniform mat4 modelToCamera[32];
      
      void main() {
        uv_ = uv;
        ivec4 index = ivec4(blendindices);
        mat4 m2c0 = modelToCamera[index.x];
        mat4 m2c1 = modelToCamera[index.y];
        mat4 m2c2 = modelToCamera[index.z];
        mat4 m2c3 = modelToCamera[index.w];
        float blend0 = 1.0 - blendweight.x - blendweight.y - blendweight.z;
        mat4 blendedModelToCamera = m2c0 * blend0 + m2c1 * blendweight.x + m2c2 * blendweight.y + m2c3 * blendweight.z;
        normal_ = normalize((blendedModelToCamera * vec4(normal,0)).xyz);
        tangent_ = normalize((blendedModelToCamera * vec4(tangent,0)).xyz);
        bitangent_ = normalize((blendedModelToCamera * vec4(bitangent,0)).xyz);
        gl_Position = cameraToProjection * (blendedModelToCamera * pos);
      }
    );

    // this is the fragment shader
    // after the rasterizer breaks the triangle into fragments
    // this is called for every fragment
    // it outputs gl_FragColor, the color of the pixel and inputs normal_ and uv_
    // the four samplers give emissive, diffuse, specular and ambient colors
    const char fragment_shader[] = SHADER_STR(
      varying vec2 uv_;
      varying vec3 normal_;
      varying vec3 tangent_;
      varying vec3 bitangent_;
      uniform vec3 light_direction;
      uniform vec4 light_diffuse;
      uniform vec4 light_ambient;
      uniform vec4 light_specular;
      uniform sampler2D samplers[5];
      uniform float shininess;
      
      void main() {
        vec3 bump = normalize(vec3(texture2D(samplers[4], uv_).xy-vec2(0.5, 0.5), 1));
        vec3 nnormal = normalize(bump.x * tangent_ + bump.y * bitangent_ + bump.z * normal_);
        vec3 half_direction = normalize(light_direction + vec3(0, 0, 1));
        float diffuse_factor = max(dot(light_direction, nnormal), 0.0);
        float specular_factor = pow(max(dot(half_direction, nnormal), 0.0), shininess);
        vec4 diffuse = texture2D(samplers[0], uv_);
        vec4 ambient = texture2D(samplers[1], uv_);
        vec4 emission = texture2D(samplers[2], uv_);
        vec4 specular = texture2D(samplers[3], uv_);
        gl_FragColor = 
          ambient * light_ambient +
          diffuse * light_diffuse * diffuse_factor +
          emission +
          specular * light_specular * specular_factor;
        //gl_FragColor = vec4(normal_, 1);
        //gl_FragColor = vec4(normal_, 1);
      }
    );
    
    // use the common shader code to compile and link the shaders
    // the result is a shader program
    phong_shader::init_uniforms(is_skinned ? skinned_vertex_shader : vertex_shader, fragment_shader);
  }

  void render(const mat4t &modelToProjection, const mat4t &modelToCamera, const vec4 &light_direction, float shininess, vec4 &light_ambient, vec4 &light_diffuse, vec4 &light_specular) {
    phong_shader::render(modelToProjection, modelToCamera, light_direction, shininess, light_ambient, light_diffuse, light_specular, 5);
  }

  void render_skinned(const mat4t &cameraToProjection, const mat4t *modelToCamera, int num_matrices, const vec4 &light_direction, float shininess, vec4 &light_ambient, vec4 &light_diffuse, vec4 &light_specular) {
    phong_shader::render_skinned(cameraToProjection, modelToCamera, num_matrices, light_direction, shininess, light_ambient, light_diffuse, light_specular, 5);
  }
};
