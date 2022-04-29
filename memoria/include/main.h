#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <signal.h>
#include "comunicacion.h"
#include "../../shared/include/estructuras.h"
#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolo.h"
#include "../../shared/include/bibliotecas.h"
#include "../../shared/include/utils.h"

void liberar_estructuras_memoria(t_configuracion_memoria*);

void liberar_estructuras_memoria(t_configuracion_memoria* datos_config_memoria)
{
    free(datos_config_memoria->algoritmo_reemplazo);
    free(datos_config_memoria->path_swap);
}