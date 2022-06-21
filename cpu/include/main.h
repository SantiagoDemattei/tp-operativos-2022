#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <signal.h>
#include "comunicacion.h"
#include "tlb_manager.h"
#include "../../shared/include/estructuras.h"
#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolo.h"
#include "../../shared/include/bibliotecas.h"
#include "../../shared/include/utils.h"



void liberar_estructuras_cpu(t_configuracion_cpu*);
void destruir_semaforos();

void liberar_estructuras_cpu(t_configuracion_cpu* datos_config_cpu){
    free(datos_config_cpu->reemplazo_tlb);
    free(datos_config_cpu->ip_memoria);
    free(datos_config_cpu->ip_cpu);
    free(datos_config_cpu->puerto_memoria);
    free(datos_config_cpu->puerto_escucha_dispatch);
    free(datos_config_cpu->puerto_escucha_interrupt);
    free(datos_config_cpu);
}

void destruir_semaforos(){
    pthread_mutex_destroy(&mutex_logger_cpu);
    pthread_mutex_destroy(&mutex_running_cpu);
    pthread_mutex_destroy(&mutex_interrupcion);
    pthread_mutex_destroy(&mutex_tlb);
    sem_destroy(&sem_agregar_a_tlb);
    sem_destroy(&sem_buscar_en_tlb);
}


#endif 