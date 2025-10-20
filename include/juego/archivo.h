#ifndef ARCHIVO_H_INCLUDED
#define ARCHIVO_H_INCLUDED

#include <stdio.h>
#include "../../include/cliente/escenario.h"

//MACROS
#define ERR_LINEA_LARGA         1
#define TAM_NOMBRE              23
#define TAM_LINEA               64

#define TAM_NOMBRE_ARCH         64


typedef struct {
    char nombre[TAM_NOMBRE + 1];
    int valor;
} tParam;

typedef struct {
    int filas;
    int columnas;
    int vidas_inicio;
    int max_num_fantasmas;
    int max_num_premios;
    int max_vidas_extra;
} tConf;


int archivo_leer_conf(FILE* arch, tConf *conf);
int archivo_escribir_conf(FILE* arch, const tConf *conf);
int archivo_escribir_escenario(tEscenario *escenario, int numRonda, long semillaRonda);

#endif // ARCHIVO_H_INCLUDED
