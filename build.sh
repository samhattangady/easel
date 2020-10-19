mkdir build && pushd build && time gcc -O1 -o easel ../src/main.c -I/usr/include/SDL2 -lSDL2 -lvulkan  && popd && ./build/easel && rm -rf build
