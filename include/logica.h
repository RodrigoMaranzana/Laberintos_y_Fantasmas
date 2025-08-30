#ifndef LOGICA_H_INCLUDED
#define LOGICA_H_INCLUDED

#include "../include/input.h"
#include "../include/entidades.h"

typedef struct{
    unsigned puntaje;
}tPartida;

typedef struct{
    char **tablero;
    tJugador jugador;
    //TDA Vector fantasmas
    tPartida partida;
    tKeyMap *mapaTeclas;
    unsigned mapaCant;
}tLogica;

int logica_inicializar(tLogica *logica);
void logica_destruir(tLogica *logica);
void logica_calc_resolucion(unsigned cantFilas, unsigned cantColumnas, unsigned *anchoRes, unsigned *altoRes);
int logica_actualizar(tLogica *logica, eAccion accion);

#endif // LOGICA_H_INCLUDED
