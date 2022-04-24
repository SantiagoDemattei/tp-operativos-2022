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


bool send_iniciar_consola(int fd, t_list* instrucciones, int tamanioConsola);
bool recv_iniciar_consola(int fd, t_list* instrucciones, int tamanioConsola);

bool send_debug(int fd);

#endif