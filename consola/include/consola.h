#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <string.h>
#include "configuracion.h"
#include "conexion.h"

typedef struct {
    char* instruccion;
} t_instruccion;

typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

void iniciar_consola(int tamanio, char* path);
t_list* obtener_instrucciones(char* path);
t_instruccion* crear_instruccion(char* instruccion);
void destruir_instruccion(t_instruccion* instruccion);


#endif
