#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <commons/log.h>
#include <commons/collections/list.h>

typedef enum {
    //AGREGAR MAS CODIGOS PARA CONEXION
    DEBUG = 69,
    INICIAR_PROCESO,
} op_code;

typedef struct {
    char* ip;
    char* puerto;
} t_configuracion_consola;

typedef struct{
    int argumento;
} t_argumento;

typedef struct {
    char* identificador;
    t_list* argumentos;
} t_instruccion;

typedef struct {
    t_log* log;
    int fd;
    char* server_name;
} t_procesar_conexion_args;

typedef struct t_configuracion_kernel{
    char* ip_memoria;
    char* puerto_memoria;
    char* ip_cpu;
    char* puerto_cpu_dispatch;
    char* puerto_cpu_interrupt;
    char* puerto_escucha;
    char* algoritmo_planificacion;
    char* estimacion_inicial;
    char* alfa;
    int grado_multiprogramacion;
    char* tiempo_maximo_bloqueado; 
} t_configuracion_kernel;

#endif