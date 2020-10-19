mkdir build && pushd build && time gcc -O1 -o easel ../src/main.c ../src/es_painter.c ../src/es_warehouse.c -I/usr/include/SDL2 -lSDL2 -lvulkan  && popd && ./build/easel && rm -rf build
