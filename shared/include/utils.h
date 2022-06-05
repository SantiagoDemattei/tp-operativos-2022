#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <commons/log.h>
#include "estructuras.h"

void loggear_info(t_log* logger, char* mensaje, pthread_mutex_t mutex);
void loggear_error(t_log* logger, char* mensaje, pthread_mutex_t mutex);
void loggear_lista_instrucciones(t_list* lista_instrucciones, t_log* logger);
void destruir_argumentos(t_argumento* argumento);
void destruir_instruccion(t_instruccion* instruccionS);
char* eliminar_caracter_retorno(char* cadena);
void destructor_queue(t_pcb* pcb);
void queue_push_con_mutex(t_queue* queue, t_pcb* pcb, pthread_mutex_t mutex);
t_pcb* queue_pop_con_mutex(t_queue* queue, pthread_mutex_t mutex);
int queue_size_con_mutex(t_queue* queue, pthread_mutex_t mutex);
bool queue_vacia_con_mutex(t_queue* queue, pthread_mutex_t mutex);

#endif