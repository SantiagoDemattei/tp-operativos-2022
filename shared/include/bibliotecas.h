#ifndef BIBLIOTECAS_H_
#define BIBLIOTECAS_H_

#include "commons/collections/dictionary.h"
#include <commons/config.h>
#include <pthread.h>


typedef struct {
		char *path;
		t_dictionary *properties;
	} t_config;


#endif