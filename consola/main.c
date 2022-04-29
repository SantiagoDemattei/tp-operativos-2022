#include "include/main.h"


uint32_t main(uint32_t argc, char** argv) {

    t_log* logger = log_create("consola.log", "CONSOLA", true, LOG_LEVEL_INFO);

    if(argc != 3){
        log_info(logger, "Error: Cantidad de parametros incorrecta");
        printf("%s", "Error: Cantidad de argumentos incorrecta.\n");
        return EXIT_FAILURE; 
    }

    uint32_t tamanio = atoi(argv[1]);
    char* path = argv[2];
    uint32_t conexion;

    conexion = iniciar_consola(tamanio, path, logger);
    
    liberar_conexion(conexion);
    log_destroy(logger);
    return 0;
}




