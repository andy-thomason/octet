//////////////////////////////////////////////////////////////////////////////////////////
//
// default frament shader for textures
//

// constant parameters
uniform sampler2D jupiter;
uniform sampler2D jupiter_mask;

// inputs
varying vec2 uv_;

void main() {
  vec4 foreground = texture2D(jupiter, uv_);
  vec4 background = texture2D(jupiter_mask, uv_);
  gl_FragColor = vec4(foreground.rgb, background.r);
  if (background.r < 0.5) discard;
}

