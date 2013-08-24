import datetime
import os
import string

today = datetime.datetime.today()

zip_name = r"install\octet-%d-%d-%d.zip" % (today.day, today.month, today.year)

try:
  os.remove(zip_name)
except:
  pass

files = [
r"vc2010\layer1\layer1.sln",
r"vc2010\layer1\layer1.vcxproj",
r"vc2010\layer1\layer1.vcxproj.filters",
r"vc2010\layer2\layer2.sln",
r"vc2010\layer2\layer2.vcxproj",
r"vc2010\layer2\layer2.vcxproj.filters",
r"assets\*.*",
r"doc\*.*",
r"xcode\*.*",
r"lib\*.lib",
r"webui\*.*",
r"src\*.*",
r"-x *.au",
r"-x *.xcf",
]

os.system("zip -r -X %s %s" % (zip_name, string.join(files, ' ')))

print("written  %s" % (zip_name))

