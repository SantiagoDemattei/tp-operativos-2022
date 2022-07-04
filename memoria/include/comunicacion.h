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
#include<math.h>
#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolo.h"
#include "../../shared/include/bibliotecas.h"
#include "../../shared/include/utils.h"
#include "../../shared/include/estructuras.h"


//const uint32_t  tamanio = configuracion_memoria->entradas_por_tabla;

//por proceso: la cantidad de FILAS de la tabla de primer nivel es la cantidad de TABLAS de segundo nivel

typedef struct t_tabla_pagina1{ 
    uint32_t id_tabla; //el que se guarda en el pcb (es el que devuelve memoria, al inicializar las estructuras)
    t_list* primer_nivel;  //tabla de primer nivel de cada proceso  
}t_tabla_pagina1;

typedef struct t_tabla_pagina2{
    uint32_t id_tabla;
    t_list* segundo_nivel;
}t_tabla_pagina2;

typedef struct t_estructura_2do_nivel{
    uint32_t marco;
    bool presencia;
    bool uso;
    bool modificado;
}t_estructura_2do_nivel;

typedef struct t_estructura_proceso{
uint32_t id_proceso;
void * espacio_en_memoria; 
t_tabla_pagina1 *tabla_pagina1; //cada proceso tiene la tabla de paginas de 1er nivel y
t_list *lista_tablas_segundo_nivel; //lista de tablas de segundo nivel (tantas como entradas tenga la de 1er nivel)
char* nombre_archivo_swap; //espacio de swap para los procesos
}t_estructura_proceso;


t_estructura_proceso* estructura_proceso_actual;

t_list *lista_estructuras; //lista de estructuras de cada proceso que va llegando 
void list_add_con_mutex_tablas(t_list* lista, t_estructura_proceso* tabla_pagina1 , pthread_mutex_t mutex);
uint32_t crear_comunicacion_kernel(t_configuracion_memoria* configuracion_memoria, t_log* logger);
uint32_t server_escuchar(t_log* logger, char* server_name, uint32_t server_socket);
static void procesar_conexion(void* void_args);
void inicializar_semaforos();
uint32_t obtener_tabla_2do_nivel(uint32_t id_tabla, uint32_t entrada_primer_nivel);
t_tabla_pagina1 *buscar_tabla_pagina1(uint32_t id_tabla);
uint32_t buscar_nro_tabla_segundo_nivel(t_tabla_pagina1 *tabla_pagina1, uint32_t entrada_tabla_1er_nivel);
t_marco_presencia* obtener_frame(uint32_t nro_tabla_2do_nivel, uint32_t entrada_tabla_2do_nivel);
uint32_t leer_valor(uint32_t frame, uint32_t desplazamiento);
void escribir_valor(uint32_t frame, uint32_t desplazamiento, uint32_t valor_a_escribir);
void copiar_valor(uint32_t frame_origen, uint32_t desplazamiento_origen, uint32_t frame_destino, uint32_t desplazamiento_destino);
#endif