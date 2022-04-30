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



void liberar_estructuras_cpu(t_configuracion_cpu*);

void liberar_estructuras_cpu(t_configuracion_cpu* datos_config_cpu){
    free(datos_config_cpu->reemplazo_tlb);
    free(datos_config_cpu->ip_memoria);
    free(datos_config_cpu->puerto_memoria);
    free(datos_config_cpu->puerto_escucha_dispatch);
    free(datos_config_cpu->puerto_escucha_interrupt);
    free(datos_config_cpu);
}
#endif 