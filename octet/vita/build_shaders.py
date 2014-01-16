import os
import io

shaders = [
  "color_f",
  "color_v",
  "clear_f",
  "clear_v",
  "texture_f",
  "texture_v",
]

for name in shaders:
  if "_f" in name:
    profile = "sce_fp_psp2"
  else:
    profile = "sce_vp_psp2"
  cmd = "psp2cgc -profile %s ..\external\src\inline_gles2\shaders\%s.cg -o %s.bin" % (profile, name, name)
  print(cmd)
  os.system(cmd)

out = open("../external/src/cg_shaders.h", "wb")
out.write("// shaders compiled by build_shaders.py\n\n")

for name in shaders:
  f = io.FileIO("%s.bin" % (name), "rb")
  z = f.read()
  out.write("static const uint8_t cg_shaderbin_%s[] = {" % (name))
  for byte in z:
    out.write("0x%02x," % (ord(byte)))
  out.write("};\n")

out.write("\n")

"""

out.write("static const char * const cg_shader_names[] = {")
for name in shaders:
  out.write('"%s", ' % (name))
out.write("NULL};\n")
out.write("\n")
out.write("\n")
out.write("static const uint8_t * const cg_shader_addr[] = {")
for name in shaders:
  out.write('cg_shaderbin_%s, ' % (name))
out.write("NULL};\n")
out.write("\n")
out.write("\n")
"""