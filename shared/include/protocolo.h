#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_

#include <inttypes.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include "estructuras.h"


bool send_iniciar_consola(uint32_t fd, t_list* instrucciones, uint32_t tamanioConsola);
bool recv_iniciar_consola(uint32_t fd, t_list** instrucciones, uint32_t* tamanioConsola);
bool send_numero_prueba(uint32_t fd, uint32_t numero);
bool recv_numero_prueba(uint32_t fd, uint32_t* numero);

bool send_debug(uint32_t fd);

#endif