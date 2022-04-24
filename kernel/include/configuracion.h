#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <commons/log.h>
#include "../../shared/include/bibliotecas.h"


/*
typedef struct {
		char *path;
		t_dictionary *properties;
	} t_config;
*/
typedef struct t_configuracion_kernel{
    char* ip_memoria;
    char* puerto_memoria;
    char* ip_cpu;
    char* puerto_cpu_dispatch;
    char* puerto_cpu_interrupt;
    char* puerto_escucha;
    char* algoritmo_planificacion;
    char* estimacion_inicial;
    char* alfa;
    int grado_multiprogramacion;
    char* tiempo_maximo_bloqueado; 
} t_configuracion_kernel;

t_configuracion_kernel* leer_configuracion();


#endif