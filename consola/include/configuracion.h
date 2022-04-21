#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdio.h>
#include <commons/config.h>
#include <stdlib.h>
#include <string.h>
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
} t_config_consola;

t_config_consola* leer_configuracion();
void liberar_estructura_datos(t_config_consola* datos);
char* eliminar_caracter_retorno(char* cadena);


#endif
