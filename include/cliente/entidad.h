#ifndef ENTIDADES_H_INCLUDED
#define ENTIDADES_H_INCLUDED

#include "../../include/cliente/assets.h"
#include "../../include/cliente/temporizador.h"

typedef enum {
    ENTIDAD_JUGADOR,
    ENTIDAD_FANTASMA_AMARILLO,
    ENTIDAD_FANTASMA_AZUL,
    ENTIDAD_FANTASMA_ROSA,
    ENTIDAD_FANTASMA_ROJO,
} eEntidadTipo;

typedef enum {
    ENTIDAD_CON_VIDA,
    ENTIDAD_SIN_VIDA,
} eEntidadEstado;

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
    eEntidadTipo tipo;
    tUbicacion ubic;
    tUbicacion ubicAnterior;
    eOrientacion orientacion;
    tTemporizador temporFrame;
    eEntidadEstado estado;
    char frame;
} tEntidad;


tEntidad* entidad_crear(eEntidadTipo entidadTipo, tUbicacion ubic);
void entidad_destruir(tEntidad *entidad);



#endif // ENTIDADES_H_INCLUDED
