#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>

void iniciar_consola(int tamanio, char* path);
t_list* obtener_instrucciones(char* path);
void print_instrucciones(char* instruccion);

#endif