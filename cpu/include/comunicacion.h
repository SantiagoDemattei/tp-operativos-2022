#ifndef COMUNICACION_H_
#define COMUNICACION_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>
#include<math.h>
#include "configuracion.h"
#include "tlb_manager.h"   
#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolo.h"
#include "../../shared/include/bibliotecas.h"
#include "../../shared/include/utils.h"




uint32_t crear_comunicacion_dispatch(t_configuracion_cpu* configuracion_cpu, t_log* logger);
uint32_t crear_comunicacion_interrupt(t_configuracion_cpu* configuracion_cpu, t_log* logger);
uint32_t crear_conexion_memoria(t_configuracion_cpu* configuracion_cpu, t_log* logger);
uint32_t server_escuchar(t_log* logger, char* server_name, uint32_t server_socket);
uint32_t enumerar_instruccion (t_instruccion* instruccion);
void controlador_tiempo_blocked_proceso(t_pcb *pcb);
static void procesar_conexion(void* void_args);
void ciclo_instruccion(uint32_t* cliente_socket, t_log* logger);
void chequear_interrupciones(uint32_t* cliente_socket);
t_direccion_fisica* calcular_mmu(t_argumento* direc_logica);
t_marco_presencia* obtener_marco(uint32_t entrada_tabla_1er_nivel, uint32_t entrada_tabla_2do_nivel, uint32_t id_tabla_1);

#endif 
