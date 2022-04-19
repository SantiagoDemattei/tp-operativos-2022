#ifndef CONEXION_H_
#define CONEXION_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include "configuracion.h"

int crear_conexion(t_datos_conexion* datos_conexion);
#endif
