#ifndef ARCHIVO_H_INCLUDED
#define ARCHIVO_H_INCLUDED

#include <stdio.h>

//MACROS
#define ERR_LINEA_LARGA         1
#define TAM_NOMBRE              20
#define TAM_LINEA               64

#define MASC_FILAS              0b000001
#define MASC_COLUMNAS           0b000010
#define MASC_VIDAS_INI          0b000100
#define MASC_MAX_NUM_FANTASMAS  0b001000
#define MASC_MAX_NUM_PREMIOS    0b010000
#define MASC_MAX_VIDAS_EXTRA    0b100000
#define CANT_PARAM              0b111111


typedef struct
{
    char nombre[TAM_NOMBRE];
    int valor;
}tParam;


typedef struct
{
    int filas;
    int columnas;
    int vidas_inicio;
    int max_num_fantasmas;
    int max_num_premios;
    int max_vidas_extra;
}tConf;


int archivo_leer_conf(FILE* arch, tConf *conf);

#endif // ARCHIVO_H_INCLUDED
