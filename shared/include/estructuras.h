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
    DEBUG, // 0
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
    OBTENER_TAMANIO,
    ORDEN_ENVIO_TAMANIO,
    PRIMER_ACCESO,
    NUM_TABLA_SEGUNDO_NIVEL,
    SEGUNDO_ACCESO,
    EJECUTAR_WRITE,
    EJECUTAR_READ,
    EJECUTAR_COPY,
    OK,
    OK_READ,
    FRAME,
    EXTRANIO
} op_code;

typedef struct // config para la consola
{
    char *ip;
    char *puerto;
} t_configuracion_consola;

typedef struct // para los argumentos de las instrucciones
{
    uint32_t argumento;
} t_argumento;

typedef struct // cada instruccion tiene un identificador y una lista de argumentos ( NO_OP tiene la lista de argumentos vacia)
{
    char *identificador;
    t_list *argumentos;
} t_instruccion;

typedef struct
{
    t_log *log;             // logger
    uint32_t *fd;           // int socket
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
    pthread_t tid_controlador;
} t_pcb;

typedef struct t_tlb
{
    uint32_t pagina;
    uint32_t marco;
    time_t ultima_referencia;
} t_tlb;

typedef struct t_marco_presencia
{
    uint32_t marco;
    bool presencia;
} t_marco_presencia;

// por proceso: la cantidad de FILAS de la tabla de primer nivel es la cantidad de TABLAS de segundo nivel

typedef struct t_tabla_pagina1
{
    uint32_t id_tabla;    // el que se guarda en el pcb (es el que devuelve memoria, al inicializar las estructuras)
    t_list *primer_nivel; // filas de la tabla de primer nivel . Cada fila es el id de la tabla de 2do nivel.
} t_tabla_pagina1;

typedef struct t_tabla_pagina2
{
    uint32_t id_tabla;
    t_list *segundo_nivel; // filas de la tabla de segundo nivel
} t_tabla_pagina2;

typedef struct t_estructura_2do_nivel
{
    uint32_t marco;
    bool presencia;
    bool uso;
    bool modificado;
} t_estructura_2do_nivel;

typedef struct t_estructura_proceso
{
    uint32_t id_proceso;
    uint32_t tamanio_proceso;
    uint32_t marco_comienzo;            // marco donde comienza el proceso
    uint32_t marco_fin;
    t_tabla_pagina1 *tabla_pagina1;     // cada proceso tiene la tabla de paginas de 1er nivel y
    t_list *lista_tablas_segundo_nivel; // lista de tablas de segundo nivel (tantas como entradas tenga la de 1er nivel)
    char *nombre_archivo_swap;          // espacio de swap para los procesos
    void *archivo_swap;
    t_list* vector_marcos;              //marcos propios del proceso
    uint32_t puntero_clock;
} t_estructura_proceso;

typedef struct t_vector_marcos{
    uint32_t estado; // 1 = ocupado, 0 = libre (por el proceso)
    uint32_t nro_pagina; //que pagina esta guardando en ese marco
}t_vector_marcos;


typedef enum{
    CREAR_ARCHIVO_SWAP,
    ESCRIBIR_PAGINA_SWAP,
    BUSCAR_CONTENIDO_PAGINA_EN_SWAP,
}INDICADOR;


typedef struct t_estructura_swap{
    t_estructura_proceso *proceso;
    INDICADOR indicador;
    void* contenido_pagina_que_esta_cargada;
    void* contenido_pagina;
    uint32_t nro_pagina;
    uint32_t marco;
    uint32_t tamanio_proceso;
    bool es_de_cpu;
}t_estructura_swap;

#endif