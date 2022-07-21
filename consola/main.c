#include "include/main.h"

// ./mainConsola.out 2048 INTEGRAL_4.txt consola.config
uint32_t main(uint32_t argc, char** argv) { //cada proceso (instancia de consola) recibe su tamaño y path archivo txt con instrucciones

    t_log* logger = log_create("consola.log", "CONSOLA", true, LOG_LEVEL_INFO);

    if(argc != 4){ 
        log_error(logger, "Error: Cantidad de parametros incorrecta");
        return EXIT_FAILURE; 
    }

    uint32_t tamanio = atoi(argv[1]);
    char* path = argv[2];
    char* path_config = argv[3];
    uint32_t conexion; 

    conexion = iniciar_consola(tamanio, path, path_config, logger); //guardamos la "linea" donde estan conectados cliente y servidor
    
    liberar_conexion(conexion);
    log_destroy(logger); 
    return 0;
}




