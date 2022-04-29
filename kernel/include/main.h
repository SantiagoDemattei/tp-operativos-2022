#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <signal.h>
#include "comunicacion.h"

void liberar_estructuras_kernel(t_configuracion_kernel*);

void liberar_estructuras_kernel(t_configuracion_kernel* datos_config_kernel){
    free(datos_config_kernel->ip_memoria);
    free(datos_config_kernel->ip_cpu);
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

#endif 