#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <signal.h>
#include "comunicacion.h"

pthread_t planificador;
pthread_t receptor;
pthread_t bloqueador;

void liberar_estructuras_kernel(t_configuracion_kernel*);
void crear_colas_estados();
void destruir_semaforos();

void liberar_estructuras_kernel(t_configuracion_kernel* datos_config_kernel){
    free(datos_config_kernel->ip_memoria);
    free(datos_config_kernel->ip_cpu);
    free(datos_config_kernel->ip_kernel);
    free(datos_config_kernel->algoritmo_planificacion);
    free(datos_config_kernel->estimacion_inicial);
    free(datos_config_kernel->alfa);
    free(datos_config_kernel->tiempo_maximo_bloqueado);
    free(datos_config_kernel->puerto_memoria);
    free(datos_config_kernel->puerto_cpu_dispatch);
    free(datos_config_kernel->puerto_cpu_interrupt);
    free(datos_config_kernel->puerto_escucha);  
    free(datos_config_kernel);
}

void crear_colas_estados(){ 
    cola_new = queue_create();
    cola_ready = list_create();
    cola_blocked = queue_create();
    cola_ready_suspendido = queue_create();
}

void destruir_colas_estados(){
    queue_destroy_and_destroy_elements(cola_new, (void*) destructor_queue);
    list_destroy_and_destroy_elements(cola_ready, (void* ) destructor_queue);
    queue_destroy_and_destroy_elements(cola_blocked, (void*) destructor_queue);
    queue_destroy_and_destroy_elements(cola_ready_suspendido, (void*) destructor_queue);
}

void destruir_semaforos(){
    pthread_mutex_destroy(&mutex_cantidad_procesos);
    pthread_mutex_destroy(&mutex_estado_running);
    pthread_mutex_destroy(&mutex_cantidad_procesos); 
    pthread_mutex_destroy(&mutex_estado_running);
    pthread_mutex_destroy(&mutex_logger_kernel);
    pthread_mutex_destroy(&mutex_cola_new);
    pthread_mutex_destroy(&mutex_cola_ready);
    pthread_mutex_destroy(&mutex_cola_blocked);
    pthread_mutex_destroy(&mutex_cola_exit);
    pthread_mutex_destroy(&mutex_cola_ready_suspendido);
    sem_destroy(&sem_planificar);
    sem_destroy(&sem_nuevo_ready);
    sem_destroy(&sem_nuevo_bloqued);
    sem_destroy(&sem_recibir);
    sem_destroy(&sem_esperar_confirmacion);
}

void destruir_hilos(){
    pthread_detach(planificador);
    pthread_detach(receptor);
    pthread_detach(bloqueador);
}

#endif 