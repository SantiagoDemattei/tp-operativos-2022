#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <signal.h>
#include "comunicacion.h"
#include "swap.h"
#include "../../shared/include/estructuras.h"
#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolo.h"
#include "../../shared/include/bibliotecas.h"
#include "../../shared/include/utils.h"

void liberar_estructuras_memoria(t_configuracion_memoria*);

void liberar_estructuras_memoria(t_configuracion_memoria* datos_config_memoria)
{   
    free(datos_config_memoria->puerto_escucha);
    free(datos_config_memoria->algoritmo_reemplazo);
    free(datos_config_memoria->path_swap);
    free(datos_config_memoria->ip_memoria);
    free(datos_config_memoria);
}

void destruir_semaforos(){
    pthread_mutex_destroy(&mutex_logger_memoria);
    pthread_mutex_destroy(&mutex_valor_tp);
    pthread_mutex_destroy(&mutex_lista_estructuras);
    pthread_mutex_destroy(&mutex_tamanio_pagina);
    pthread_mutex_destroy(&mutex_estructura_proceso_actual);
    pthread_mutex_destroy(&mutex_marcos);
    pthread_mutex_destroy(&mutex_espacio_memoria);
    pthread_mutex_destroy(&mutex_variable_global);
    sem_destroy(&sem_swap);
    sem_destroy(&sem_fin_swap);
}


#endif