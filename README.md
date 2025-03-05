# Pong_A2
To compile this CMAKE project, make sure you have vcpkg installed and the following library installed by vcpkg:
SDL2, SDL2main, SDL2_ttf

SDL2main.lib may not be installed by vckg by default. If it happended, you have to do this by yourself.

Then in CMakeLists.txt file, edit

set(VCPKG_DIR "/path/to/your/vcpkg")

with your installed vcpkg location.

 Then, use your CMAKE compiler option of your desire. If you find it hard to compile the program, you can use Visual Studio IDE to handle the job for you.
