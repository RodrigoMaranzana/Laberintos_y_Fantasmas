#ifndef ENTIDADES_H_INCLUDED
#define ENTIDADES_H_INCLUDED

typedef enum{
    ENTIDAD_JUGADOR,
    ENTIDAD_FANTASMA,
    ENTIDAD_PREMIO,
    ENTIDAD_VIDA,
    ENTIDAD_CANTIDAD
}eTiposEntidades;

typedef struct{
    unsigned columna;
    unsigned fila;
}tUbicacion;

typedef struct{
    tUbicacion ubic;
    unsigned vidasCant;
}tJugador;

typedef struct{
    tUbicacion ubic;
    ///puntero a funcion IA
}tFantasma;


///entidades_fantasma_ia();

#endif // ENTIDADES_H_INCLUDED
