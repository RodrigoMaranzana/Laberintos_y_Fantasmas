#ifndef ENTIDADES_H_INCLUDED
#define ENTIDADES_H_INCLUDED

#include "../include/assets.h"

typedef struct{
    unsigned columna;
    unsigned fila;
}tUbicacion;

typedef struct{
    eImagenes imagen;
    tUbicacion ubic;
    ///puntero a funcion IA
}tEntidad;


///entidades_fantasma_ia();

#endif // ENTIDADES_H_INCLUDED
