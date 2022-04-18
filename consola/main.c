#include "include/main.h"

int main(int argc, char** argv) {
    printf("Hello World!\n");

    if(argc != 3){
        return EXIT_FAILURE;
    }

    int tamanio = atoi(argv[1]);
    char* path = argv[2];

    iniciar_consola(tamanio, path);
    return 0;
}




