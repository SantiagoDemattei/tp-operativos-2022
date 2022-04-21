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

typedef enum {
    //AGREGAR MAS CODIGOS PARA CONEXION
    DEBUG = 69,
    APROBAR_OPERATIVOS,
} op_code;


bool send_aprobar_operativos(int fd, uint8_t  nota1, uint8_t  nota2);
bool recv_aprobar_operativos(int fd, uint8_t* nota1, uint8_t* nota2);

bool send_debug(int fd);

#endif