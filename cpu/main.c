#include "include/main.h"

void sighandler(int x)
{
    switch (x)
    {
    case SIGINT:
        liberar_conexion(socket_cpu_dispatch);
        liberar_conexion(socket_cpu_interrupt);
        destruir_semaforos();
        log_destroy(logger_cpu);
        liberar_estructuras_cpu(configuracion_cpu);
        exit(EXIT_SUCCESS);
    }
}

void interrupt()
{
    while(server_escuchar_interrupcion(logger_cpu, "CPU INTERRUPT", socket_cpu_interrupt) != 0);
    pthread_exit(NULL);
}

uint32_t main(void)
{

    signal(SIGINT, sighandler);
    op_code cop;
    logger_cpu = log_create("cpu.log", "CPU", true, LOG_LEVEL_INFO);
    configuracion_cpu = leer_configuracion(logger_cpu);
    socket_cpu_dispatch = crear_comunicacion_dispatch(configuracion_cpu, logger_cpu); // servidor de kernel para recibir PCB
    socket_cpu_interrupt = crear_comunicacion_interrupt(configuracion_cpu, logger_cpu);
    socket_memoria_cpu = crear_conexion_memoria(configuracion_cpu, logger_cpu); // cliente cpu conectandose a la memoria (Servidor) para recibir el tamanio de pagina

    inicializar_semaforos();
    tlb = list_create();

    send_orden_envio_tamanio(socket_memoria_cpu); // le aviso a la memoria que me mande el tamanio de pagina

    if (recv(socket_memoria_cpu, &cop, sizeof(op_code), 0) != sizeof(op_code))
    {
        loggear_error(logger_cpu, "ERROR DE CONEXION", mutex_logger_cpu);
    }
    recv_tamanio_y_cant_entradas(socket_memoria_cpu, &tamanio_pagina, &cant_entradas_por_tabla); // recibo el tamano de la pagina y la cantidad de entradas por tabla que me manda la memoria para la MMU
    liberar_conexion(socket_memoria_cpu);

    pthread_t hilo_interrupt;
    pthread_create(&hilo_interrupt, NULL, (void*)interrupt, NULL);
    pthread_detach(hilo_interrupt);
    while (server_escuchar(logger_cpu, "CPU DISPATCH", socket_cpu_dispatch) != 0);

    liberar_conexion(socket_cpu_dispatch);
    liberar_conexion(socket_cpu_interrupt);
    liberar_estructuras_cpu(configuracion_cpu);
    log_destroy(logger_cpu);
    destruir_semaforos();

    return EXIT_SUCCESS;
}