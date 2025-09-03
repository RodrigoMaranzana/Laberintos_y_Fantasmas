#include "../include/logica.h"
#include "../include/retorno.h"
#include "../include/matriz.h"
#include "../include/menu.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PIXELES_TILE 48

static void _logica_procesar_turno(tLogica *logica, SDL_Keycode tecla);

void logica_calc_resolucion(unsigned cantColumnas, unsigned cantFilas, unsigned *anchoRes, unsigned *altoRes)
{
    /// DEBE OBTENERSE DESDE EL ARCHIVO DE TEXTO CONF
    *anchoRes =  cantColumnas * PIXELES_TILE;
    *altoRes =  cantFilas * PIXELES_TILE;
}

int logica_inicializar(tLogica *logica)
{
    escenario_crear(&logica->escenario, CANT_COLUMNAS, CANT_FILAS);
    escenario_generar(&logica->escenario);

    logica->estado = LOGICA_EN_ESPERA;

    return TODO_OK;
}

void logica_destruir(tLogica *logica)
{
    escenario_destruir(&logica->escenario);
}

int logica_actualizar(tLogica *logica, SDL_Keycode tecla)
{
    _logica_procesar_turno(logica, tecla);

    return TODO_OK;
}

static void _logica_procesar_turno(tLogica *logica, SDL_Keycode tecla)
{
    tUbicacion nuevaUbic = logica->escenario.jugador.ubic;

    switch(tecla){

        case SDLK_UP:
            nuevaUbic.fila--;
            break;
        case SDLK_DOWN:
            nuevaUbic.fila++;
            break;
        case SDLK_LEFT:
            nuevaUbic.columna--;
            break;
        case SDLK_RIGHT:
            nuevaUbic.columna++;
            break;
        default:
            break;
    }

    if(logica->escenario.tablero[nuevaUbic.fila][nuevaUbic.columna].tile.imagen != IMAGEN_PARED_LAD_GRIS){

        logica->escenario.tablero[nuevaUbic.fila][nuevaUbic.columna].entidad = logica->escenario.tablero[logica->escenario.jugador.ubic.fila][logica->escenario.jugador.ubic.columna].entidad;
        logica->escenario.tablero[logica->escenario.jugador.ubic.fila][logica->escenario.jugador.ubic.columna].entidad = NULL;

        logica->escenario.jugador.ubic.columna = nuevaUbic.columna;
        logica->escenario.jugador.ubic.fila = nuevaUbic.fila;
    }

}




