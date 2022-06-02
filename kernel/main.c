#include "include/main.h"

void sighandler(int x) { 
    switch (x) {
        case SIGINT:
            liberar_conexion(socket_kernel);
            log_destroy(logger);
            liberar_estructuras_kernel(configuracion_kernel);
            pthread_mutex_destroy(&mutex_cantidad_procesos);
            pthread_mutex_destroy(&mutex_estado_running);
            destruir_colas_estados();
            exit(EXIT_SUCCESS);
    }
}

uint32_t main(void){

    signal(SIGINT, sighandler); 
      	
    logger = log_create("kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
    configuracion_kernel = leer_configuracion(); //me devuelve los datos para la conexion
    socket_kernel = crear_comunicacion(configuracion_kernel, logger); 
    inicializar_semaforos();
    running = NULL; //variable global -> estado running -> variable porque siempre tenemos 1 solo proceso en running 

    // funcion de creacion de colas de estados
    crear_colas_estados();
    
    
    while(server_escuchar(logger, "KERNEL", socket_kernel)!=0); //si se conecta un cliente se crea un hilo para atenderlo y recibe 1 para poder seguir conectando con mas clientes

    liberar_conexion(socket_kernel);
    liberar_estructuras_kernel(configuracion_kernel);
    destruir_colas_estados();
    pthread_mutex_destroy(&mutex_cantidad_procesos);
    log_destroy(logger);

    return EXIT_SUCCESS;
}



