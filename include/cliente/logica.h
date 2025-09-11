#ifndef LOGICA_H_INCLUDED
#define LOGICA_H_INCLUDED
#include <SDL.h>
#include <SDL_mixer.h>
#include "../../include/cliente/input.h"
#include "../../include/cliente/entidad.h"
#include "../../include/cliente/escenario.h"
#include "../../include/comun/cola.h"

#define FILAS_DEF            17
#define COLUMNAS_DEF         17
#define VIDAS_INICIO_DEF     3
#define NUMERO_FANTASMAS_DEF 4
#define NUMERO_PREMIOS_DEF   2
#define VIDAS_EXTRA_DEF      1

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
    tRonda ronda;
    unsigned puntaje;
} tPartida;

typedef struct{
    tUbicacion ubic;
    char direccion;
} tMovimiento;

typedef struct {
    tEscenario escenario;
    tPartida partida;
    eLogicaEstado estado;
    tEntidad *fantasmaEnMov;
    tTemporizador fantasmaMovTempor;
    tCola movimientosJugador; // Cola para historial de movimientos
} tLogica;

int logica_inicializar(tLogica *logica);
void logica_destruir(tLogica *logica);
void logica_calc_resolucion(unsigned cantFilas, unsigned cantColumnas, unsigned *anchoRes, unsigned *altoRes);
int logica_actualizar(tLogica *logica);
void logica_procesar_turno(tLogica *logica, SDL_Keycode tecla);

void logica_mostrar_historial_movimientos(tLogica *logica);

#endif // LOGICA_H_INCLUDED
