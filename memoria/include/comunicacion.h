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
#include<math.h>
#include "configuracion.h"
#include "swap.h"
#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolo.h"
#include "../../shared/include/bibliotecas.h"
#include "../../shared/include/utils.h"
#include "../../shared/include/estructuras.h"


typedef enum
{ 
    CLOCK,
    CLOCK_M
} ALGORITMO_MEMORIA; 

//const uint32_t  tamanio = configuracion_memoria->entradas_por_tabla;

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
t_marco_presencia* obtener_frame(uint32_t nro_tabla_2do_nivel, uint32_t entrada_tabla_2do_nivel, uint32_t nro_pagina);
void buscar_estructura_del_proceso(uint32_t pid);
uint32_t leer_valor(uint32_t frame, uint32_t desplazamiento);
void escribir_valor(uint32_t frame, uint32_t desplazamiento, uint32_t valor_a_escribir);
void copiar_valor(uint32_t frame_origen, uint32_t desplazamiento_origen, uint32_t frame_destino, uint32_t desplazamiento_destino);
uint32_t buscar_marcos_para_asignar();
void llenar_marcos_para_el_proceso(uint32_t inicio, uint32_t fin, uint32_t contenido);
uint32_t buscar_marco_libre(uint32_t nro_pagina, void *contenido_pagina);
ALGORITMO_MEMORIA algoritmo_reemplazo();
void llenar_marcos_para_el_proceso_local(t_list* lista_marcos_del_proceso, uint32_t cant_marcos,uint32_t contenido);
void llenar_marcos_para_el_proceso(uint32_t inicio, uint32_t fin, uint32_t contenido);
t_list *buscar_paginas_con_presencia_en_1(t_list *lista_tablas_segundo_nivel);
uint32_t buscar_marcos_para_asignar_local(t_list *lista);
void encender_bit_modificado(uint32_t frame);
#endif
