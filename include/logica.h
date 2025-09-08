#ifndef LOGICA_H_INCLUDED
#define LOGICA_H_INCLUDED
#include <SDL.h>
#include <SDL_mixer.h>
#include "../include/input.h"
#include "../include/entidad.h"
#include "../include/escenario.h"

/// MACROS TEMPORALES
#define CANT_COLUMNAS 17
#define CANT_FILAS 17

typedef enum {
    LOGICA_EN_ESPERA,
    LOGICA_JUGANDO,
    LOGICA_FIN_PARTIDA
} eLogicaEstado;

typedef struct {
    unsigned semilla;
    unsigned numero;
} tRonda;

typedef struct {
    //tLista *rondas; //Para retroceder y avanzar entre rondas
    tRonda ronda;
    unsigned puntaje;
} tPartida;

typedef struct {
    tEscenario escenario;
    tPartida partida;
    eLogicaEstado estado;
    tEntidad *fantasmaEnMov;
    tTemporizador fantasmaMovTempor;
} tLogica;

int logica_inicializar(tLogica *logica);
void logica_destruir(tLogica *logica);
void logica_calc_resolucion(unsigned cantFilas, unsigned cantColumnas, unsigned *anchoRes, unsigned *altoRes);
int logica_actualizar(tLogica *logica);
void logica_procesar_turno(tLogica *logica, SDL_Keycode tecla);

#endif // LOGICA_H_INCLUDED
