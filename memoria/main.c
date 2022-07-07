#include "include/main.h"

uint32_t socket_memoria;

void sighandler(int x)
{
    switch (x)
    {
    case SIGINT:
        liberar_conexion(socket_memoria);
        log_destroy(logger);
        liberar_estructuras_memoria(configuracion_memoria);
        destruir_semaforos();
        exit(EXIT_SUCCESS);
    }
}

uint32_t main(void)
{

    signal(SIGINT, sighandler);
    op_code cop;
    logger = log_create("memoria.log", "MEMORIA", true, LOG_LEVEL_INFO);
    configuracion_memoria = leer_configuracion();
    socket_memoria = crear_comunicacion_kernel(configuracion_memoria, logger); // inicia el servidor para que el kernel y la cpu se conecten a la memoria
    espacio_memoria = malloc(configuracion_memoria->tam_memoria);

    cant_total_marcos = configuracion_memoria->tam_memoria / configuracion_memoria->tam_pagina; // marcos totales que va a tener la memoria (el tamaño de la pagina es igual al del marco)
    marcos_totales = list_create();                                                             

    for (int i = 0; i < cant_total_marcos; i++)
    { // inicializo el bitmap con todos los marcos como libres
        uint32_t *elemento = malloc(sizeof(uint32_t));
        *elemento = 0; //inicializa con 0 porque al principio estan todos libres 
        list_add(marcos_totales, elemento);
    }
    puntero_clock = 0;
    inicializar_semaforos();
    lista_estructuras = list_create(); // lista de tablas de primer nivel para que la memoria se guarde las tablas de primer nivel de todos los procesos

    while (server_escuchar(logger, "MEMORIA", socket_memoria) != 0)
        ; // servidor de kernel y cpu (ambos se conectan a la misma ip y puerto)

    liberar_conexion(socket_memoria);
    liberar_estructuras_memoria(configuracion_memoria);
    destruir_semaforos();
    log_destroy(logger);

    return EXIT_SUCCESS;
}
