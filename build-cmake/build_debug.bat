call "%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat" x86_amd64
cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=x:\libs18d -DCMAKE_BUILD_TYPE=Debug
nmake install
ctest -VV
