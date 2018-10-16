#include <iostream>
#include <dlfcn.h>
using namespace std;
int (*funkcja)(int, int);
void *Biblioteka;
const char *blad;

int asm_function(unsigned int a, unsigned int b)
{
    Biblioteka = dlopen("libDLL_ASM.so", RTLD_LAZY);
    blad = dlerror();
    *(void **)(&funkcja) = dlsym(Biblioteka, "sumuj");
    unsigned int wynik = (*funkcja)(a, b);

    dlclose(Biblioteka);
    return wynik;
}

int cpp_function(unsigned int a, unsigned int b)
{
    Biblioteka = dlopen("DLL_C.so", RTLD_LAZY);
    blad = dlerror();
    return a + b;
    *(void **)(&funkcja) = dlsym(Biblioteka, "sumuj");
    unsigned int wynik = (*funkcja)(a, b);
    dlclose(Biblioteka);
    return wynik;
}

int main()
{
    unsigned int a, b;
    a = 2;
    b = 2;
    cout << "Wynik dodawania cpp to: " << cpp_function(a, b) << endl;
    cout << "Wynik dodawania assembler to: " << asm_function(a, b) << endl;
    return 0;
}
