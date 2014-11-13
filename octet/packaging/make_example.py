import sys
import os
import dircache
import string
import stat
import shutil

proto_dir = 'src/examples/example_prototype/'
target_dir = 'src/examples/'

def make_example(src_dir, dest_dir, projname, depth):
  print(depth,"make", src_dir, dest_dir)
  try:
    os.mkdir(dest_dir)
  except:
    pass
  for fname in dircache.listdir(src_dir):
    mode = os.stat(src_dir + fname).st_mode
    if 'prototype' in fname:
      dest_name = string.replace(fname, 'prototype', projname)
    else:
      dest_name = fname
    print(depth, "fname", fname)
    if stat.S_ISDIR(mode):
      make_example(src_dir + fname + '/', dest_dir + '/' + dest_name + '/', projname, depth+"  ")
    else:
      print(depth, dest_dir + '/' + dest_name)
      try:
        test = open(dest_dir + '/' + dest_name, "rb")
        test.close()
      except:
        out = open(dest_dir + '/' + dest_name, "wb")
        for line in open(src_dir + fname):
          line = string.replace(line, 'prototype', projname)
          out.write(line)

def update():
  for dirname in dircache.listdir("src/examples/"):
    if dirname[0:8] == "example_" and dirname != "example_prototype":
      print("updating " + dirname)
      make_example(proto_dir, target_dir + dirname, dirname, "")

def clean():
  for dirname in dircache.listdir("src/examples/"):
    if dirname[0:8] == "example_" and dirname != "example_prototype":
      print("clean " + dirname)
      for dirname2 in dircache.listdir(target_dir + dirname):
        if ".sln" in dirname2 or ".vcxproj" in dirname2:
          os.remove(target_dir + dirname + "/" + dirname2)
        elif ".xcodeproj" in dirname2:
          shutil.rmtree(target_dir + dirname + "/" + dirname2)

num_args = len(sys.argv)

if num_args == 1 or "--help" in sys.argv:
  print("\nmakes or updates a new octet project in src/examples")
  print("\nusage: packaging\make_example.py projectname")
  exit()

for i in range(1,num_args):
  arg = sys.argv[i];
  if arg[0] != '-':
    make_example(proto_dir, target_dir + arg, arg, "")
  else:
    if arg == '--update':
      update()
    elif arg == '--clean':
      clean()
    else:
      print("unrecognised option " + arg)
      exit()
