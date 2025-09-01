#ifndef LOGICA_H_INCLUDED
#define LOGICA_H_INCLUDED
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "../include/input.h"
#include "../include/entidad.h"
#include "../include/escenario.h"

typedef enum{
    LOGICA_EN_ESPERA,
    LOGICA_JUGANDO,
    LOGICA_FIN_PARTIDA
}eLogicaEstado;

typedef struct{
    unsigned semilla;
    unsigned numero;
}tRonda;

typedef struct{
    //tLista *rondas; //Para retroceder y avanzar entre rondas
    tRonda ronda;
    unsigned puntaje;
}tPartida;

typedef struct{
    tEscenario escenario;
    tPartida partida;
    tKeyMap *mapaTeclas;
    unsigned mapaCant;
    eLogicaEstado estado;
}tLogica;

int logica_inicializar(tLogica *logica);
void logica_destruir(tLogica *logica);
void logica_calc_resolucion(unsigned cantFilas, unsigned cantColumnas, unsigned *anchoRes, unsigned *altoRes);
int logica_actualizar(tLogica *logica, eAccion accion);

#endif // LOGICA_H_INCLUDED
