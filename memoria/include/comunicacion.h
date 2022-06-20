#ifndef COMUNICACION_H_
#define COMUNICACION_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>
#include "configuracion.h"

#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolo.h"
#include "../../shared/include/bibliotecas.h"
#include "../../shared/include/utils.h"
#include "../../shared/include/estructuras.h"


//const uint32_t  tamanio = configuracion_memoria->entradas_por_tabla;

//por proceso: la cantidad de FILAS de la tabla de primer nivel es la cantidad de TABLAS de segundo nivel

typedef struct t_tabla_pagina1{ //la memoria tiene una lista con el id de la tabla q le corresponde al proceso junto con su tabla de primer nivel
    uint32_t id_tabla; //el que se guarda en el pcb (es el que devuelve memoria, al inicializar las estructuras)
    uint32_t* primer_nivel[];  //tabla de primer nivel de cada proceso  
}t_tabla_pagina1;

// uint32_t tabla_pagina_2 [][3]; //matriz que representa la tabla de paginas de 2do nivel de 4 columnas y n filas

t_list *lista_tablas_primer_nivel; //lista de tablas de primer nivel por cada proceso para que la memoria conozca la de todos los procesos
void list_add_con_mutex_tablas(t_list* lista, t_tabla_pagina1* tabla_pagina1 , pthread_mutex_t mutex);
uint32_t crear_comunicacion_kernel(t_configuracion_memoria* configuracion_memoria, t_log* logger);
uint32_t server_escuchar(t_log* logger, char* server_name, uint32_t server_socket);
static void procesar_conexion(void* void_args);
void inicializar_semaforos();

#endif