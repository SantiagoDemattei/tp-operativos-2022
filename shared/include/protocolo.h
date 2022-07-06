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
bool send_pcb(uint32_t fd, t_pcb *pcb, op_code cop);
bool recv_pcb(uint32_t fd, t_pcb** pcb);
bool send_inicializar_estructuras(uint32_t fd,uint32_t tamanio_proceso, uint32_t id_proceso);
bool recv_inicializar_estructuras(uint32_t fd, uint32_t* tamanio_proceso, uint32_t* id_proceso);
bool send_valor_tb(uint32_t fd, uint32_t valor_tb);
bool recv_valor_tb(uint32_t fd, uint32_t* valor_tb);
bool send_interrupcion_por_nuevo_ready(uint32_t fd);
bool send_fin_proceso(uint32_t fd);
bool send_suspension(uint32_t fd, uint32_t id);
bool recv_suspension(uint32_t fd, uint32_t* id);
bool send_confirmacion_suspension(uint32_t fd);
bool send_tamanio_y_cant_entradas(uint32_t fd, uint32_t num_pagina, uint32_t valor);
bool recv_tamanio_y_cant_entradas(uint32_t fd, uint32_t *num_pagina, uint32_t *valor);
bool send_orden_envio_tamanio(uint32_t fd);
bool send_entrada_tabla_1er_nivel(uint32_t fd, uint32_t id_tabla, uint32_t entrada_tabla_1er_nivel);
bool recv_entrada_tabla_1er_nivel(uint32_t fd, uint32_t *id_tabla, uint32_t *entrada_tabla_1er_nivel);
bool send_entrada_tabla_2do_nivel(uint32_t fd, uint32_t num_segundo_nivel, uint32_t entrada_tabla_2do_nivel, uint32_t nro_pagina);
bool recv_entrada_tabla_2do_nivel(uint32_t fd, uint32_t *num_segundo_nivel, uint32_t *entrada_tabla_2do_nivel, uint32_t *nro_pagina);
bool send_num_tabla_2do_nivel(uint32_t fd, uint32_t num_tabla_2do_nivel);
bool recv_num_tabla_2do_nivel(uint32_t fd, uint32_t *num_tabla_2do_nivel);
bool send_ejecutar_write(uint32_t fd, uint32_t marco, uint32_t desplazamiento, uint32_t valor_a_escribir, uint32_t id_proceso);
bool recv_ejecutar_write(uint32_t fd, uint32_t* marco, uint32_t* desplazamiento, uint32_t* valor_a_escribir, uint32_t* id_proceso);
bool send_ok(uint32_t fd);
bool send_ejecutar_read(uint32_t fd, uint32_t marco, uint32_t desplazamiento, uint32_t id_proceso);
bool recv_ejecutar_read(uint32_t fd, uint32_t* marco, uint32_t* desplazamiento, uint32_t* id_proceso);
bool send_ok_read(uint32_t fd, uint32_t valor_leido);
bool recv_ok_read(uint32_t fd, uint32_t* valor_leido);
bool send_frame(uint32_t fd, t_marco_presencia* marco_presencia);
bool recv_frame(uint32_t fd, t_marco_presencia** marco_presencia);
bool send_ejecutar_copy(uint32_t fd, uint32_t marco_origen, uint32_t desplazamiento_origen, uint32_t marco_destino, uint32_t desplazamiento_destino, uint32_t id_proceso);
bool recv_ejecutar_copy(uint32_t fd, uint32_t* marco_origen, uint32_t* desplazamiento_origen, uint32_t* marco_destino, uint32_t* desplazamiento_destino, uint32_t* id_proceso);
#endif