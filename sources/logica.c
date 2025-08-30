#include "../include/logica.h"
#include "../include/retornos.h"
#include <stdlib.h>
#include <string.h>

/// MACROS TEMPORALES
#define PIXELES_X_TILE 16

void logica_calc_resolucion(unsigned cantColumnas, unsigned cantFilas, unsigned *anchoRes, unsigned *altoRes)
{
    *anchoRes =  cantColumnas * PIXELES_X_TILE;
    *altoRes =  cantFilas * PIXELES_X_TILE;
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

    /// TEMPORAL
    logica->jugador.ubic.columna = 2;
    logica->jugador.ubic.fila = 2;


    return TODO_OK;
}

void logica_destruir(tLogica *logica)
{
    if(logica->mapaTeclas){

        free(logica->mapaTeclas);
    }
}

int logica_actualizar(tLogica *logica, eAccion accion)
{
    switch(accion){

        case ACCION_ARRIBA:
            logica->jugador.ubic.fila--;
            break;
        case ACCION_ABAJO:
            logica->jugador.ubic.fila++;
            break;
        case ACCION_IZQUIERDA:
            logica->jugador.ubic.columna--;
            break;
        case ACCION_DERECHA:
            logica->jugador.ubic.columna++;
            break;
        default:
            break;
    }

    return TODO_OK;
}




