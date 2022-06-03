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
bool send_inicializar_estructuras(uint32_t fd);
bool send_valor_tb(uint32_t fd, uint32_t valor_tb);
bool recv_valor_tb(uint32_t fd, uint32_t* valor_tb);
bool send_interrupcion_por_nuevo_ready(uint32_t fd);
bool send_fin_proceso(uint32_t fd);
bool send_pcb_con_tiempo_bloqueado(uint32_t fd, t_pcb *pcb, uint32_t tiempo_bloqueo);


#endif