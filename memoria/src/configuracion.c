#include "../include/configuracion.h"

t_configuracion_memoria* leer_configuracion(){

    t_config* nuevo_config; // revisar struct (no importa el de commons)
    nuevo_config = config_create("./memoria.config");
    if(nuevo_config == NULL){
        printf("Error: No se pudo abrir el archivo de configuracion de memoria \n");
        exit(EXIT_FAILURE);
    }

    char* ip_memoria = config_get_string_value(nuevo_config, "IP_MEMORIA");
    char* puerto_escucha = config_get_string_value(nuevo_config, "PUERTO_ESCUCHA"); // leo PUERTO_ESCUCHA
    uint32_t tam_memoria = config_get_int_value(nuevo_config, "TAM_MEMORIA");
    uint32_t tam_pagina = config_get_int_value(nuevo_config, "TAM_PAGINA");
    uint32_t entradas_por_tabla = config_get_int_value(nuevo_config, "ENTRADAS_POR_TABLA");
    uint32_t retardo_memoria= config_get_int_value(nuevo_config,"RETARDO_MEMORIA");
    char* algoritmo_reemplazo = config_get_string_value(nuevo_config, "ALGORITMO_REEMPLAZO"); // leo ALGORITMO_REEMPLAZO
    uint32_t marcos_por_proceso = config_get_int_value(nuevo_config,"RETARDO_SWAP");
    char* path_swap = config_get_string_value(nuevo_config,"PATH_SWAP");


    t_configuracion_memoria* datos = malloc(sizeof(t_configuracion_memoria)); // creo estructura de datos de conexion
    datos->ip_memoria = malloc(strlen(ip_memoria)+1); 
    strcpy(datos->ip_memoria, ip_memoria);
    datos->algoritmo_reemplazo = malloc(strlen(algoritmo_reemplazo)+1); 
    strcpy(datos->algoritmo_reemplazo, algoritmo_reemplazo);
    datos->path_swap = malloc(strlen(path_swap)+1);
    strcpy(datos->path_swap, path_swap);
    datos->puerto_escucha = malloc(strlen(puerto_escucha)+1);
    strcpy(datos->puerto_escucha, puerto_escucha);
    datos->tam_memoria = tam_memoria;
    datos->tam_pagina = tam_pagina;
    datos->entradas_por_tabla = entradas_por_tabla;
    datos->retardo_memoria = retardo_memoria;
    datos->marcos_por_proceso = marcos_por_proceso;

    config_destroy(nuevo_config); // libero la memoria del config
    return datos;
}

void inicializar_semaforos(){
    pthread_mutex_init(&mutex_logger_memoria, NULL);
    pthread_mutex_init(&mutex_valor_tp, NULL);
    pthread_mutex_init(&mutex_lista_tablas, NULL);
}   
