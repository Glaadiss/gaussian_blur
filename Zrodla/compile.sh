nasm -f elf64 DLL_ASM.asm -o DLL_ASM.o
ld -shared DLL_ASM.o -o libDLL_ASM.so
g++ -fPIC -shared DLL_C.cpp -o DLL_C.so
g++ main.cpp -ldl -o main

export LD_LIBRARY_PATH="$LD_LIBRARY_PATH: `pwd`"
./main