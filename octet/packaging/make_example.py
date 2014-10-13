import sys
import os
import dircache
import string

proto_dir = 'src/examples/example_prototype/'
target_dir = 'src/examples/'

def make_example(src_dir, dest_dir, projname):
  print("make", src_dir, dest_dir)
  try:
    os.mkdir(target_dir + projname)
  except:
    pass
  for fname in dircache.listdir(src_dir):
    if 'prototype' in fname:
      dest_name = string.replace(fname, 'prototype', projname)
    else:
      dest_name = fname
    print("fname", fname)
    if '.xcodeproj' in dest_name:
      make_example(src_dir + fname, dest_dir + dest_name, projname)
    else:
      print(target_dir + projname + '/' + dest_name)
      try:
        test = open(dest_dir + projname + '/' + dest_name, "rb")
        test.close()
      except:
        out = open(dest_dir + projname + '/' + dest_name, "wb")
        for line in open(proto_dir + fname):
          line = string.replace(line, 'prototype', projname)
          out.write(line)


num_args = len(sys.argv)

if num_args == 1 or "--help" in sys.argv:
  print("\nmakes or updates a new octet project in src/examples")
  print("\nusage: packaging\make_example.py projectname")
  exit()

for i in range(1,num_args):
  arg = sys.argv[i];
  if arg[0] != '-':
    make_example(proto_dir, target_dir, arg)
  else:
    print("unrecognised option " + arg)
    exit()
