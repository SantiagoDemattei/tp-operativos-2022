#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <stdint.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

typedef enum
{
    // AGREGAR MAS CODIGOS PARA CONEXION
    DEBUG,
    INICIAR_PROCESO,
    ENVIAR_PCB,
    INICIALIZAR_ESTRUCTURAS,
    VALOR_TB,
    INT_NUEVO_READY,
    BLOQUEO_IO

} op_code;

typedef struct
{
    char *ip;
    char *puerto;
} t_configuracion_consola;

typedef struct
{
    uint32_t argumento;
} t_argumento;

typedef struct
{
    char *identificador;
    t_list *argumentos;
} t_instruccion;

typedef struct
{
    t_log *log;             // logger
    int fd;                 // int socket
    char *server_name;      // nombre del servidor
} t_procesar_conexion_args; // para los hilos

typedef struct t_configuracion_kernel
{
    char *ip_memoria;
    char *ip_kernel;
    char *puerto_memoria;
    char *ip_cpu;
    char *puerto_cpu_dispatch;
    char *puerto_cpu_interrupt;
    char *puerto_escucha;
    char *algoritmo_planificacion;
    char *estimacion_inicial;
    char *alfa;
    uint32_t grado_multiprogramacion;
    char *tiempo_maximo_bloqueado;
} t_configuracion_kernel;

typedef struct t_configuracion_cpu
{
    int entradas_tlb;
    char *reemplazo_tlb;
    float retardo_noop;
    char *ip_memoria;
    char *ip_cpu;
    char *puerto_memoria;
    char *puerto_escucha_dispatch;
    char *puerto_escucha_interrupt;
} t_configuracion_cpu;

typedef struct t_configuracion_memoria
{
    char *ip_memoria;
    char *puerto_escucha;
    int tam_memoria;
    int tam_pagina;
    int entradas_por_tabla;
    int retardo_memoria;
    char *algoritmo_reemplazo;
    int marcos_por_proceso;
    int retardo_swap;
    char *path_swap;
} t_configuracion_memoria;

typedef struct t_pcb
{
    uint32_t id;              // identificador del proceso
    uint32_t tamanio;         // tamaño en bytes del proceso, el mismo no cambiará a lo largo de la ejecución
    t_list *instrucciones;    // lista de instrucciones a ejecutar.
    uint32_t program_counter; // número de la próxima instrucción a ejecutar
    uint32_t tabla_pagina;
    uint32_t estimacion_rafaga;
} t_pcb;



#endif