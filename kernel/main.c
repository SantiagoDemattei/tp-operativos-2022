#include "include/main.h"

void sighandler(int x) { 
    switch (x) {
        case SIGINT:
            liberar_conexion(socket_kernel);
            log_destroy(logger);
            liberar_estructuras_kernel(configuracion_kernel);
            destruir_semaforos();
            destruir_colas_estados();
            destruir_hilos();
            exit(EXIT_SUCCESS);
    }
}


// ./mainKernel.out kernelBase.config
uint32_t main(uint32_t argc, char** argv){

    signal(SIGINT, sighandler); //para liberar cuando matamos un proceso desde consola 
      	
    logger = log_create("kernel.log", "KERNEL", true, LOG_LEVEL_INFO);

    if(argc != 2){
        log_error(logger, "Error: Cantidad de parametros incorrecta");
        return EXIT_FAILURE;
    }
    char* path_config = argv[1];
    configuracion_kernel = leer_configuracion(path_config); //me devuelve los datos para la conexion
    socket_kernel = crear_comunicacion(configuracion_kernel, logger); //me devuelve el socket para la conexion
    inicializar_semaforos();
    running = NULL; //variable global -> estado running -> variable porque siempre tenemos 1 solo proceso en running 
    
    // funcion de creacion de colas de estados
    crear_colas_estados();
    
    pthread_t planificador; 
    pthread_t receptor;
    pthread_t bloqueador; 
    pthread_t largo_plazo; 
    pthread_create(&planificador, NULL, (void*)planificar, NULL); // encargado de planificar sacando de ready y mandando el pcb a la CPU
    pthread_create(&receptor, NULL, (void*)recibir, NULL); // encargado de escuchar mensajes de la cpu (IO o EXIT)
    pthread_create(&bloqueador, NULL, (void*)bloquear, NULL); // encargado de bloquear los procesos que se encuentran en la cola de blocked
    pthread_create(&largo_plazo, NULL, (void*)revisar_entrada_a_ready, NULL); // encargado de revisar la entrada a ready

    while(server_escuchar(logger, "KERNEL", socket_kernel)); 

    pthread_detach(planificador);
    pthread_detach(receptor);
    pthread_detach(bloqueador);
    pthread_detach(largo_plazo);
    liberar_conexion(socket_kernel);
    liberar_estructuras_kernel(configuracion_kernel);
    destruir_colas_estados();
    destruir_semaforos();
    log_destroy(logger);

    return EXIT_SUCCESS;
}



