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
bool send_debug(uint32_t fd);
bool send_pcb(uint32_t fd, t_pcb* pcb);
bool recv_pcb(uint32_t fd, t_pcb** pcb);
static void* serializar_pcb(size_t* size, t_pcb* pcb);
static void deserializar_pcb (void* stream, t_pcb** pcbF);

#endif