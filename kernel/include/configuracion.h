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
t_log* logger;
pthread_mutex_t mutex_cantidad_procesos; 
pthread_mutex_t mutex_estado_running;
pthread_mutex_t mutex_logger_kernel;
pthread_mutex_t mutex_cola_new;
pthread_mutex_t mutex_cola_ready;
pthread_mutex_t mutex_cola_blocked;
pthread_mutex_t mutex_cola_exit;
pthread_mutex_t mutex_cola_ready_suspendido;
pthread_mutex_t mutex_variable_contador;
pthread_mutex_t mutex_info_desalojado;
pthread_mutex_t mutex_socket_memoria;
pthread_mutex_t mutex_estoy_planificando;
sem_t sem_planificar;
sem_t sem_nuevo_ready;
sem_t sem_nuevo_bloqued;
sem_t sem_running;
sem_t sem_recibir;
sem_t sem_desalojo;
sem_t sem_esperar_confirmacion;
sem_t sem_queue_suspended;
sem_t sem_inicio;
//sem_t finalizar_planificacion;

t_pcb *running;
t_queue *cola_new;
t_list *cola_ready;
t_queue *cola_blocked;
t_queue *cola_ready_suspendido;


#endif
