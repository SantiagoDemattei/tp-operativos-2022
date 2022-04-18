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




#endif
