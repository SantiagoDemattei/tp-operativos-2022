#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <commons/log.h>
#include "../../shared/include/estructuras.h"
#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolo.h"
#include "../../shared/include/bibliotecas.h"
#include "../../shared/include/utils.h"

t_configuracion_kernel* leer_configuracion();

t_pcb *running;
t_queue *cola_new;
t_queue *cola_ready;
t_queue *cola_blocked;
t_queue *cola_exit;

#endif
