#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include "commons/collections/dictionary.h"

typedef struct {
		char *path;
		t_dictionary *properties;
	} t_config;

typedef struct {
    char* ip;
    char* puerto;
} t_datos_conexion;

t_datos_conexion* leer_configuracion();
void liberar_estructura_datos(t_datos_conexion* datos);
char* eliminar_caracter_retorno(char* cadena);


#endif
