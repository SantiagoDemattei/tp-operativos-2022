#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <commons/log.h>
#include "../../shared/include/estructuras.h"
#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolo.h"
#include "../../shared/include/bibliotecas.h"
#include "../../shared/include/utils.h"

t_configuracion_kernel* leer_configuracion();
void inicializar_semaforos();

pthread_mutex_t mutex_cantidad_procesos; 
pthread_mutex_t mutex_estado_running;
pthread_mutex_t mutex_logger_kernel;
pthread_mutex_t mutex_cola_new;
pthread_mutex_t mutex_cola_ready;
pthread_mutex_t mutex_lista_blocked;
pthread_mutex_t mutex_cola_exit;
sem_t sem_planificar;
//sem_t finalizar_planificacion;

t_pcb *running;
t_queue *cola_new;
t_queue *cola_ready;
t_list *bloqueados;
t_queue *cola_exit;





#endif
