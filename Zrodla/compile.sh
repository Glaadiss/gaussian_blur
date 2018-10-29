nasm -f macho64 DLL_ASM.asm -o ../build/DLL_ASM.o
ld -dylib ../build/DLL_ASM.o -o ../build/libDLL_ASM.dylib
gcc -g -fPIC -Wall -Werror -Wextra -pedantic blur.c -shared -o ../build/blur.dylib
gcc -g -ldl -o ../build/prog main.c -lpthread
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH: `pwd`"

../build/prog 2
