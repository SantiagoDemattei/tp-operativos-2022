#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <commons/log.h>
#include "../../shared/include/bibliotecas.h"
#include "../../shared/include/estructuras.h"
#include "../../shared/include/utils.h"

t_configuracion_consola* leer_configuracion(char* path_config, t_log* logger);
void liberar_estructura_datos(t_configuracion_consola* datos);


#endif
