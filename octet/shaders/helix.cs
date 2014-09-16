
struct my_vertex {
  vec4 pos;
  vec4 color;
};

// number of steps in helix
uniform float radius1 = 1.0f;
uniform float radius2 = 7.0f;
uniform float height = 24.0f;
uniform float num_twists = 4.0f;
uniform int num_steps = 320;

// the "0" in the binding corresponds to the "0" in glBindBufferBase()
layout(std140, binding = 0) buffer dest_buf {
  my_vertex data[];
} out_buf;

// 64 items per workgroup is optimal on most GPUs
layout (local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
 
void main()
{
  uint i = uint(gl_GlobalInvocationID.x);
  float r = 1.0f, g = float(i) * (1.0f / float(num_steps)), b = 0.0f;
  float y = float(i) * (height / float(num_steps)) - height * 0.5f;
  float angle = float(i) * (num_twists * 2.0f * 3.14159265f / float(num_steps));

  // predicate the writes to avoid buffer overflows.
  if (i <= num_steps) {
    out_buf.data[i*2+0].pos = vec4(cos(angle) * radius1, y, sin(angle) * radius1, 1);
    out_buf.data[i*2+0].color = vec4(r, g, b, 1);
    out_buf.data[i*2+1].pos = vec4(cos(angle) * radius2, y, sin(angle) * radius2, 1);
    out_buf.data[i*2+1].color = vec4(r, g, b, 1);
  }
}

