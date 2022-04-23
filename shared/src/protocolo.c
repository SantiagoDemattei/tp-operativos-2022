#include "../include/protocolo.h"

// APROBAR_OPERATIVOS
static void* serializar_aprobar_operativos(uint8_t nota1, uint8_t nota2) {
    void* stream = malloc(sizeof(op_code) + sizeof(uint8_t) * 2);

    op_code cop = APROBAR_OPERATIVOS;
    memcpy(stream, &cop, sizeof(op_code));
    memcpy(stream+sizeof(op_code), &nota1, sizeof(uint8_t));
    memcpy(stream+sizeof(op_code)+sizeof(uint8_t), &nota2, sizeof(uint8_t));
    return stream;
}

static void deserializar_aprobar_operativos(void* stream, uint8_t* nota1, uint8_t* nota2) {
    memcpy(nota1, stream, sizeof(uint8_t));
    memcpy(nota2, stream+sizeof(uint8_t), sizeof(uint8_t));
}

bool send_aprobar_operativos(int fd, uint8_t nota1, uint8_t nota2) {
    size_t size = sizeof(op_code) + sizeof(uint8_t) * 2;
    void* stream = serializar_aprobar_operativos(nota1, nota2);
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_aprobar_operativos(int fd, uint8_t* nota1, uint8_t* nota2) {
    size_t size = sizeof(uint8_t) * 2;
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    
    deserializar_aprobar_operativos(stream, nota1, nota2);

    free(stream);
    return true;
}


// DEBUG
bool send_debug(int fd) {
    op_code cop = DEBUG;
    if (send(fd, &cop, sizeof(op_code), 0) != sizeof(op_code))
        return false;
    return true;
}