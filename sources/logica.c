#include "../include/logica.h"
#include "../include/retorno.h"
#include "../include/matriz.h"
#include "../include/menu.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PIXELES_TILE 48

static void _logica_procesar_turno(tLogica *logica, eAccion accion);

void logica_calc_resolucion(unsigned cantColumnas, unsigned cantFilas, unsigned *anchoRes, unsigned *altoRes)
{
    /// DEBE OBTENERSE DESDE EL ARCHIVO DE TEXTO CONF
    *anchoRes =  cantColumnas * PIXELES_TILE;
    *altoRes =  cantFilas * PIXELES_TILE;
}

int logica_inicializar(tLogica *logica)
{
    tKeyMap keyMapLocal[ACCION_CANTIDAD] = {
        {SDLK_ESCAPE, ACCION_SALIR},
        {SDLK_RETURN, ACCION_CONFIRMAR},
        {SDLK_BACKSPACE, ACCION_CANCELAR},
        {SDLK_UP, ACCION_ARRIBA},
        {SDLK_DOWN, ACCION_ABAJO},
        {SDLK_LEFT, ACCION_IZQUIERDA},
        {SDLK_RIGHT, ACCION_DERECHA},
    };

    logica->mapaCant = ACCION_CANTIDAD;
    logica->mapaTeclas = malloc(logica->mapaCant * sizeof(tKeyMap));
    if(!logica->mapaTeclas){

        return ERR_SIN_MEMORIA;
    }

    memcpy(logica->mapaTeclas, keyMapLocal, sizeof(keyMapLocal));

    escenario_crear(&logica->escenario, 16, 16);
    escenario_generar(&logica->escenario);

    logica->estado = LOGICA_EN_ESPERA;

    return TODO_OK;
}

void logica_destruir(tLogica *logica)
{
    if(logica->mapaTeclas){

        free(logica->mapaTeclas);
    }

    escenario_destruir(&logica->escenario);
}

int logica_actualizar(tLogica *logica, eAccion accion)
{
    _logica_procesar_turno(logica, accion);

    return TODO_OK;
}

static void _logica_procesar_turno(tLogica *logica, eAccion accion)
{
    tUbicacion nuevaUbic = logica->escenario.jugador.ubic;

    switch(accion){

        case ACCION_ARRIBA:
            nuevaUbic.fila--;
            break;
        case ACCION_ABAJO:
            nuevaUbic.fila++;
            break;
        case ACCION_IZQUIERDA:
            nuevaUbic.columna--;
            break;
        case ACCION_DERECHA:
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




