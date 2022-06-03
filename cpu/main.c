#include "include/main.h"

void sighandler(int x) {
    switch (x) {
        case SIGINT:
            liberar_conexion(socket_cpu_dispatch);
            liberar_conexion(socket_cpu_interrupt);
            log_destroy(logger_cpu);
            liberar_estructuras_cpu(configuracion_cpu);
            exit(EXIT_SUCCESS);
    }
}

uint32_t main(void){

    signal(SIGINT, sighandler);
      	
    logger_cpu = log_create("cpu.log", "CPU", true, LOG_LEVEL_INFO);
    configuracion_cpu = leer_configuracion(logger_cpu);
    socket_cpu_dispatch = crear_comunicacion_dispatch(configuracion_cpu, logger_cpu); // servidor de kernel
    socket_cpu_interrupt = crear_comunicacion_interrupt(configuracion_cpu, logger_cpu) ;
    inicializar_semaforos();
    
    while(server_escuchar(logger_cpu, "CPU DISPATCH", socket_cpu_dispatch) != 0 && server_escuchar(logger_cpu, "CPU INTERRUPT", socket_cpu_interrupt) != 0);

    liberar_conexion(socket_cpu_dispatch);
    liberar_conexion(socket_cpu_interrupt);
    liberar_estructuras_cpu(configuracion_cpu);
    log_destroy(logger_cpu);

    return EXIT_SUCCESS;
}
