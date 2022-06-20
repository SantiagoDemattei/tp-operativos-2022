#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <stdint.h>
#include <semaphore.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

typedef enum
{
    // AGREGAR MAS CODIGOS PARA CONEXION
    DEBUG, //0
    INICIAR_PROCESO,
    ENVIAR_PCB,
    INICIALIZAR_ESTRUCTURAS,
    VALOR_TB,
    INT_NUEVO_READY,
    BLOQUEO_IO,
    LIBERAR_ESTRUCTURAS,
    INTERRUPCION,
    SUSPENSION,
    CONFIRMACION_SUSPENSION,
    WRITE_MEMORIA,
    ORDEN_ENVIO_TAMANIO

} op_code;

typedef struct  //config para la consola
{
    char *ip;
    char *puerto;
} t_configuracion_consola;

typedef struct //para los argumentos de las instrucciones
{
      uint32_t argumento;
} t_argumento;

typedef struct //cada instruccion tiene un identificador y una lista de argumentos ( NO_OP tiene la lista de argumentos vacia)
{
    char *identificador;
    t_list *argumentos;
} t_instruccion;

typedef struct
{
    t_log *log;             // logger
    uint32_t *fd;                 // int socket 
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
    uint32_t entradas_tlb;
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
    uint32_t tam_memoria;
    uint32_t tam_pagina;
    uint32_t entradas_por_tabla;
    uint32_t retardo_memoria;
    char *algoritmo_reemplazo;
    uint32_t marcos_por_proceso;
    uint32_t retardo_swap;
    char *path_swap;
} t_configuracion_memoria;

typedef struct t_pcb
{
    uint32_t id;              // identificador del proceso
    uint32_t tamanio;         // tamaño en bytes del proceso, el mismo no cambiará a lo largo de la ejecución
    t_list *instrucciones;    // lista de instrucciones a ejecutar.
    uint32_t program_counter; // número de la próxima instrucción a ejecutar
    uint32_t tabla_pagina;
    uint32_t tiempo_bloqueo;
    uint32_t *cliente_socket;
    double rafaga_real_anterior;
    double estimacion_rafaga_anterior;
    bool blocked_suspendido; 
} t_pcb;


#endif