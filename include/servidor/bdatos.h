#ifndef BDATOS_H_INCLUDED
#define BDATOS_H_INCLUDED
#include <stdio.h>
#include "..\..\include\comun\arbol.h"
#include "..\..\include\comun\comun.h"
#include "..\..\include\comun\lista.h"
#include "..\..\include\comun\protocolo.h"

#define TAM_INT_TEXTO 12
#define TAM_SIMBOLO 12

typedef enum {
    // Comandos
    CREAR, INSERTAR, ACTUALIZAR, SELECCIONAR,
    // Operadores
    IGUAL, MAYOR, MENOR, DISTINTO,
    // Tipos
    ENTERO, TEXTO,
    // Restricciones
    PK, AI,
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
    long offset;
}tIndice;

typedef struct {
    const char *cursor;
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
    tArbol arbol;
}tTabla;

typedef struct {
    tTabla tablaAbierta;
    tSecuencia secuencia;
} tBDatos;


int bdatos_iniciar(tBDatos *bDatos);
int bdatos_procesar_solcitud(tBDatos *bDatos, const char *solicitud, tLista *listaDatos, int *cantRegistrosDatos, int *tamRegistroDatos);
eSimbolo bdatos_parsear_comando(tSecuencia *secuencia);
int bdatos_apagar(tBDatos *bDatos);
int bdatos_insertar(tBDatos *bDatos);
int bdatos_actualizar(tBDatos *bDatos);
int bdatos_seleccionar(tBDatos *bDatos);
const char* bdatos_obtener_mensaje(eBDRetorno codigoError);
int bdatos_cargar_idx(tBDatos *bDatos);

int bdatos_dummy();



#endif // BDATOS_H_INCLUDED
