#ifndef LOGICA_H_INCLUDED
#define LOGICA_H_INCLUDED
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
    LOGICA_EN_PAUSA,
    LOGICA_EN_LOGIN,
    LOGICA_FIN_PARTIDA
} eLogicaEstado;

typedef struct {
    int numRonda;
    int cantPremios;
    int cantFantasmas;
    int cantVidasExtra;
    int cantVidasActual;
    long semillaRonda;
} tRonda;

typedef struct{
    tUbicacion ubic;
    char direccion;
    tEntidad* entidad;
} tMovimiento;

typedef struct {
    int columnas;
    int filas;
    int vidasInicio;
    int maxVidasExtra;
    int maxFantasmas;
    int maxPremios;
}tConfig;

typedef struct {
    long semillaMaestra;

    tEscenario escenario;
    tEntidad jugador;
    tEntidad *fantasmas;

    int puntajeTotal;

    tConfig config;
    tRonda ronda;
    eLogicaEstado estado;

    tCola movsJugador;
    tCola movimientos;
} tLogica;

int logica_inicializar(tLogica *logica);
void logica_destruir(tLogica *logica);
void logica_calc_min_res(tLogica *logica, unsigned *anchoRes, unsigned *altoRes);
void logica_procesar_turno(tLogica *logica, SDL_Keycode tecla);
int logica_procesar_movimientos(tLogica *logica);
int logica_iniciar_juego(tLogica *logica);
int logica_nueva_ronda(tLogica *logica);
void logica_fin_juego(tLogica *logica);
void logica_mostrar_historial_movs(tLogica *logica);
void logica_actualizar(tLogica *logica);












#endif // LOGICA_H_INCLUDED
