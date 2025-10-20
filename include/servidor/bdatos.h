#ifndef BDATOS_H_INCLUDED
#define BDATOS_H_INCLUDED
#include <stdio.h>
#include "..\..\include\comun\arbol.h"
#include "..\..\include\comun\comun.h"
#include "..\..\include\comun\lista.h"
#include "..\..\include\comun\protocolo.h"
#include "..\..\include\comun\vector.h"

#define TAM_INT_TEXTO 12
#define TAM_SIMBOLO 12

typedef enum {
    // Comandos
    CREAR, INSERTAR, ACTUALIZAR, SELECCIONAR,
    // Operadores
    IGUAL, MAYOR, MENOR, DISTINTO, TOP,
    // Tipos
    ENTERO, TEXTO,
    // Conector
    DONDE,
    // Restricciones
    PK, AI, IS,
    // Control
    SIMBOLOS_CANT, DESCONOCIDO,
} eSimbolo;

typedef enum {
    TIPO_ENTERO,
    TIPO_TEXTO,
    TIPO_INVALIDO
} eTipoDato;

typedef struct {
    char clave[TAM_MAX_TIPO_CHAR];
    unsigned cantOffsets;
    tLista listaOffsets;
} tIndiceIS;

typedef struct {
    char clave[TAM_MAX_TIPO_CHAR];
    long offset;
} tIndicePK;

typedef struct {
    const char *cursor;
    int finSec;
} tSecuencia;

typedef struct {
    char nombre[TAM_IDENTIFICADOR];
    eTipoDato tipo;
    unsigned offsetCampo;
    unsigned tam;
    char esPK;
    char esAI;
    char esIS;
} tCampo;

typedef struct {
    char nombreTabla[TAM_IDENTIFICADOR];
    unsigned cantCampos;
    unsigned tamRegistro;
    int cantRegistros;
    unsigned tamRegIdx;
    unsigned proximoAI;
    tVector  vecCampos;
} tEncabezado;

typedef struct {
    tCampo campo;
    void *dato;
} tCampoValor;

typedef struct {
    tEncabezado encabezado;
    FILE *arch;
    tArbol arbolPK;
    tArbol arbolIS;
} tTabla;

typedef struct {
    tTabla tablaAbierta;
    tSecuencia secuencia;
} tBDatos;

int bdatos_iniciar(tBDatos *bDatos);
int bdatos_procesar_solicitud(tBDatos *bDatos, const char *solicitud, tLista *listaDatos, int *cantRegistrosDatos);
eSimbolo bdatos_parsear_comando(tSecuencia *secuencia);
int bdatos_apagar(tBDatos *bDatos);
int bdatos_insertar(tBDatos *bDatos);
int bdatos_actualizar(tBDatos *bDatos);
int bdatos_seleccionar(tBDatos *bDatos);
const char* bdatos_obtener_mensaje(eBDRetorno codigoError);
int bdatos_cargar_idx(tBDatos *bDatos);
char* bdatos_registro_a_texto(const tEncabezado *encabezado, const char *registro);

int bdatos_dummy();



#endif // BDATOS_H_INCLUDED
