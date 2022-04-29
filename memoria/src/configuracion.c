#include "../include/configuracion.h"

t_configuracion_memoria* leer_configuracion(){

    t_config* nuevo_config; // revisar struct (no importa el de commons)
    nuevo_config = config_create("./memoria.config");
    if(nuevo_config == NULL){
        printf("Error: No se pudo abrir el archivo de configuracion de memoria \n");
        exit(EXIT_FAILURE);
    }

    int puerto_escucha = config_get_int_value(nuevo_config, "PUERTO_ESCUCHA"); // leo PUERTO_ESCUCHA
    int tam_memoria = config_get_int_value(nuevo_config, "TAM_MEMORIA");
    int tam_pagina = config_get_int_value(nuevo_config, "TAM_PAGINA");
    int entradas_por_tabla = config_get_int_value(nuevo_config, "ENTRADAS_POR_TABLA");
    int retardo_memoria= config_get_int_value(nuevo_config,"RETARDO_MEMORIA");
    char* algoritmo_reemplazo = config_get_string_value(nuevo_config, "ALGORITMO_REEMPLAZO"); // leo ALGORITMO_REEMPLAZO
    int marcos_por_proceso = config_get_int_value(nuevo_config,"RETARDO_SWAP");
    char* path_swap = config_get_string_value(nuevo_config,"PATH_SWAP");

    t_configuracion_memoria* datos = malloc(sizeof(t_configuracion_memoria)); // creo estructura de datos de conexion
    datos->algoritmo_reemplazo = malloc(strlen(algoritmo_reemplazo)+1); 
    strcpy(datos->algoritmo_reemplazo, algoritmo_reemplazo);
    datos->path_swap = malloc(strlen(path_swap)+1);
    strcpy(datos->path_swap, path_swap);
    
    config_destroy(nuevo_config); // libero la memoria del config
    return datos;
}