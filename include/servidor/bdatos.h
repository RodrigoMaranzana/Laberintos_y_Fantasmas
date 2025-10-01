#ifndef BDATOS_H_INCLUDED
#define BDATOS_H_INCLUDED
#include <stdio.h>
#include "..\..\include\comun\arbol.h"
#include "..\..\include\comun\comun.h"

#define ARCH_NOMBRE "jugadores.dat"
#define ARCH_IDX_NOMBRE "jugadores.idx"

#define TAM_INT_TEXTO 12
#define TAM_SIMBOLO 12
#define TAM_IDENTIFICADOR 16
#define TAM_NOMBRE_ARCH TAM_IDENTIFICADOR + 4
#define MAX_CAMPOS_POR_TABLA 8
#define MAX_PK_POR_TABLA 2
#define TAM_MAX_TIPO_CHAR 64

typedef enum {
    ERROR_TODO_OK,
    ERROR_SINTAXIS,
    ERROR_COMANDO,
    ERROR_DEMASIADOS_CAMPOS,
    ERROR_SIN_PK,
    ERROR_DEMASIADOS_PK,
    ERROR_PK_INEXISTENTE,
    ERROR_TABLA_YA_EXISTE,
    ERROR_TABLA_NO_EXISTE,
    ERROR_ESCRITURA,
    ERROR_CANT_CAMPOS,
    ERROR_CAMPO_INEXISTENTE,
    ERROR_CAMPO_INVALIDO,
    ERROR_SIN_MEMO,
    ERROR_LECTURA,
    ERROR_ARCH,
    ERROR_PARAMETRO,
    ERROR_TAM_TEXTO,
    ERROR_CAMPO_ES_AI,
    ERROR_AI_DEBE_SER_PK,
    ERROR_AI_DEBE_SER_ENTERO,
} eError;

typedef enum {
    // Comandos
    CREAR,
    ABRIR,
    INSERTAR,
    ACTUALIZAR,
    SELECCIONAR,
    // Palabras clave
    TABLA,
    EN,
    DESDE,
    DONDE,
    // Operadores
    IGUAL,
    MAYOR,
    MENOR,
    DISTINTO,
    // Tipos
    ENTERO,
    TEXTO,
    // Restricciones
    PK,
    AI,

    SIMBOLOS_CANT,
    DESCONOCIDO,
} eSimbolo;

typedef enum {
    TIPO_ENTERO,
    TIPO_TEXTO,
    TIPO_INVALIDO
} eTipoDato;

typedef struct {
    char clave[TAM_MAX_TIPO_CHAR];
    long offset;
}tIndice;

typedef struct {
    char *cursor;
    int finSec;
}tSecuencia;

typedef struct {
    char nombre[TAM_IDENTIFICADOR];
    eTipoDato tipo;
    unsigned offsetCampo;
    unsigned tam;
    char esPK;
    char esAI;
}tCampo;

typedef struct {
    char nombreTabla[TAM_IDENTIFICADOR];
    unsigned cantCampos;
    unsigned tamRegistro;
    unsigned tamRegIdx;
    int cantRegistros;
    unsigned proximoAI;
    tCampo campos[MAX_CAMPOS_POR_TABLA];
}tEncabezado;

typedef struct {
    void *dato;
    unsigned tam;
} tValor;

typedef struct {
    tCampo campo;
    tValor valor;
    int ingresado;
} tDatoParseado;

typedef struct {
    tEncabezado encabezado;
    FILE *arch;
    FILE *archIdx;
    tArbol arbol;
    const char **simbolos;
    tSecuencia secuencia;
} tBDatos;


int bdatos_iniciar(tBDatos *bDatos);

int bdatos_procesar_solcitud(tBDatos *bDatos, const char *solicitud);

eSimbolo bdatos_parsear_comando(tSecuencia *secuencia);

int bdatos_crear(tBDatos *bDatos);
int bdatos_abrir(tBDatos *bDatos);
int bdatos_cerrar(tBDatos *bDatos);
int bdatos_insertar(tBDatos *bDatos);
int bdatos_actualizar(tBDatos *bDatos);
int bdatos_seleccionar(tBDatos *bDatos);

int bdatos_cerrar(tBDatos *bDatos);
int bdatos_cargar_idx(tBDatos *bDatos);

int bdatos_dummy();



#endif // BDATOS_H_INCLUDED
