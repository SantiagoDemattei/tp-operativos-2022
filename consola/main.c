#include "include/main.h"

int main(int argc, char** argv) {
    if(argc != 3){
        printf("%s", "Error: Cantidad de argumentos incorrecta.\n");
        return EXIT_FAILURE; 
    }

    int tamanio = atoi(argv[1]);
    char* path = argv[2];

    iniciar_consola(tamanio, path);
    return 0;
}




