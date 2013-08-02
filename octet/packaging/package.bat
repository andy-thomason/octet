del install\octet-%1.zip /q
zip -r -X install\octet-%1.zip vc2010\framework.sln vc2010\framework.vcxproj vc2010\framework.vcxproj.filters vc2010\animation\animation.vcxproj vc2010\animation\animation.vcxproj.filters assets\*.* doc\*.* xcode\*.* lib\*.lib webui\*.* src\*.* -x *.au -x *.xcf 
copy install\octet-%1.zip c:\Users\andyt\andythomason.com\htdocs\files 
