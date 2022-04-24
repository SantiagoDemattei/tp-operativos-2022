#include "include/main.h"



int main(int argc, char** argv) {

    t_log* logger = log_create("consola.log", "CONSOLA", true, LOG_LEVEL_INFO);

    if(argc != 3){
        log_info(logger, "Error: Cantidad de parametros incorrecta");
        printf("%s", "Error: Cantidad de argumentos incorrecta.\n");
        return EXIT_FAILURE; 
    }

    int tamanio = atoi(argv[1]);
    char* path = argv[2];
    int conexion;

    conexion = iniciar_consola(tamanio, path, logger);
    
    liberar_conexion(conexion);
    log_destroy(logger);
    return 0;
}




