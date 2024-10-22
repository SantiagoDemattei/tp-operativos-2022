#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <commons/log.h>
#include "estructuras.h"
#include "time.h"

void loggear_info(t_log* logger, char* mensaje, pthread_mutex_t mutex);
void loggear_error(t_log* logger, char* mensaje, pthread_mutex_t mutex);
void loggear_tlb(t_list* lista_tlb ,t_log* logger ,pthread_mutex_t mutex);
void loggear_warning(t_log* logger, char* mensaje, pthread_mutex_t mutex);
void loggear_lista_instrucciones(t_list* lista_instrucciones, t_log* logger);
void destruir_argumentos(t_argumento* argumento);
void destruir_instruccion(t_instruccion* instruccionS);
char* eliminar_caracter_retorno(char* cadena);
void destructor_queue(t_pcb* pcb);
void queue_push_con_mutex(t_queue* queue, t_pcb* pcb, pthread_mutex_t mutex);
t_pcb* queue_pop_con_mutex(t_queue* queue, pthread_mutex_t mutex);
t_pcb* queue_peek_con_mutex(t_queue* queue, pthread_mutex_t mutex);
int queue_size_con_mutex(t_queue* queue, pthread_mutex_t mutex);
bool queue_vacia_con_mutex(t_queue* queue, pthread_mutex_t mutex);
void destruir_pcb(t_pcb* pcb);
int list_size_con_mutex(t_list* lista, pthread_mutex_t mutex);
t_pcb* list_get_and_remove_con_mutex(t_list* lista, int indice, pthread_mutex_t mutex);
void list_add_con_mutex(t_list* lista, t_pcb* pcb, pthread_mutex_t mutex);
t_pcb* queue_find_con_mutex(t_queue* queue, t_pcb* pcb_buscado, pthread_mutex_t mutex);
bool criterio_id(t_pcb* pcb, t_pcb* pcb_buscado);
void* queue_find(t_queue *self, bool(*condition)(void*));
int list_size_con_mutex_tlb(t_list* lista, pthread_mutex_t mutex);
void list_add_con_mutex_tlb(t_list* lista, t_tlb* tlb, pthread_mutex_t mutex);
uint32_t list_find_con_mutex_tlb(t_list* lista, uint32_t tlb_buscado, pthread_mutex_t mutex);
uint32_t list_find_con_mutex_tlb_indice(t_list* lista, uint32_t tlb_buscado, pthread_mutex_t mutex);
bool criterio_pagina_tlb(uint32_t tlb_buscado, t_tlb* tlb_de_la_lista);
uint32_t* list_get_con_mutex_marcos(t_list* lista, int indice, pthread_mutex_t mutex);
void loggear_tabla_pagina2(t_list *lista_tabla_segundo_nivel, t_log *logger, pthread_mutex_t mutex);
int list_size_con_mutex_marcos(t_list* lista, pthread_mutex_t mutex);
#endif