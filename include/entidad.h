#ifndef ENTIDADES_H_INCLUDED
#define ENTIDADES_H_INCLUDED

#include "../include/assets.h"
#include "../include/temporizador.h"

typedef enum {
    MIRANDO_ABAJO,
    MIRANDO_IZQUIERDA,
    MIRANDO_DERECHA,
    MIRANDO_ARRIBA,
} eOrientacion;

typedef struct {
    int columna;
    int fila;
} tUbicacion;

typedef struct {
    eImagen imagen;
    tUbicacion ubic;
    tUbicacion ubicAnterior;
    eOrientacion orientacion;
    tTemporizador temporFrame;
    char frame;
} tEntidad;

#endif // ENTIDADES_H_INCLUDED
