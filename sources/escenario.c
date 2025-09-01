#include "../include/escenario.h"
#include "../include/matriz.h"
#include "../include/retorno.h"
#include <stdio.h>

static int _escenario_calcular_pared(tEscenario *escenario, int columna, int fila);
static int _escenario_es_pared(tEscenario *escenario, int columna, int fila);

int escenario_crear(tEscenario *escenario, unsigned columnas, unsigned filas)
{
    escenario->tablero = (tCasilla**)matriz_crear((size_t)columnas, (size_t)filas, sizeof(tCasilla));
    if(!escenario->tablero){

        puts("ERROR: No se pudo crear el escenario");
        return ERR_SIN_MEMORIA;
    }

    escenario->columnas = columnas;
    escenario->filas = filas;

    return TODO_OK;
}

void escenario_generar(tEscenario *escenario)
{
    eImagenes imagen;
    unsigned fila, columna;
    int mascara;

    for(fila = 0; fila < escenario->filas; fila++){

        for(columna = 0; columna < escenario->columnas; columna++){

            ///TEST
            imagen = (fila == 0 || columna == 0 || fila == escenario->filas - 1 || columna == escenario->columnas - 1) ? IMAGEN_PARED_LAD_GRIS : IMAGEN_PISO;

            if((fila == 2 && columna == 2) || (fila == 5 && columna == 5) || (fila == 5 && columna == 6) || (fila == 5 && columna == 7) || (fila == 4 && columna == 7)
               || (fila == 6 && columna == 6) || (fila == 7 && columna == 6) || (fila == 7 && columna == 7) || (fila == 8 && columna == 7)
                || (fila == 8 && columna == 6) || (fila == 8 && columna == 7) || (fila == 8 && columna == 8)){

                imagen = IMAGEN_PARED_LAD_GRIS;
            }

            escenario->tablero[fila][columna].tile.imagen = imagen;

            if(imagen == IMAGEN_PARED_LAD_GRIS){

                escenario->tablero[fila][columna].tile.rectTile = (SDL_Rect){.x = 0, .y = 0, .w = 48, .h = 64};
            }else{

                escenario->tablero[fila][columna].tile.rectTile = (SDL_Rect){.x = 0, .y = 0, .w = 48, .h = 48};
            }
            ///

            escenario->tablero[fila][columna].entidad = NULL;
        }
    }

    for(fila = 0; fila < escenario->filas; fila++){

        for(columna = 0; columna < escenario->columnas; columna++){

            if(escenario->tablero[fila][columna].tile.imagen == IMAGEN_PARED_LAD_GRIS){

                mascara = _escenario_calcular_pared(escenario, columna, fila);

                escenario->tablero[fila][columna].tile.rectTile.x = (mascara % 4) * 48;
                escenario->tablero[fila][columna].tile.rectTile.y = (mascara / 4) * 64;
                escenario->tablero[fila][columna].tile.rectTile.w = 48;
                escenario->tablero[fila][columna].tile.rectTile.h = 64;

            }else{

                escenario->tablero[fila][columna].tile.rectTile = (SDL_Rect){.x = 0, .y = 0, .w = 48, .h = 48};
            }
        }
    }


    ///TEST
    escenario->jugador.ubic.columna = 1;
    escenario->jugador.ubic.fila = 1;
    escenario->jugador.imagen = IMAGEN_FANTASMA;
    escenario->tablero[1][1].entidad = &escenario->jugador;
    ///
}

static int _escenario_calcular_pared(tEscenario *escenario, int columna, int fila)
{
    int mascara = 0;

    if(_escenario_es_pared(escenario, columna, fila - 1)){

        mascara += 1;
    }

    if(_escenario_es_pared(escenario, columna + 1, fila)){

        mascara += 2;
    }

    if(_escenario_es_pared(escenario, columna, fila + 1)){

        mascara += 4;
    }

    if(_escenario_es_pared(escenario, columna - 1, fila)){

        mascara += 8;
    }

    return mascara;
}

static int _escenario_es_pared(tEscenario *escenario, int columna, int fila)
{
    if(columna < 0 || fila < 0 || columna >= escenario->columnas || fila >= escenario->filas){

        return 0;
    }

    return escenario->tablero[fila][columna].tile.imagen == IMAGEN_PARED_LAD_GRIS ? 1 : 0;
}

void escenario_destruir(tEscenario *escenario)
{
    if(escenario->tablero){

        matriz_destruir((void**)escenario->tablero, escenario->filas);
    }
}

void escenario_dibujar(SDL_Renderer *renderer, tEscenario *escenario, SDL_Texture **imagenes)
{
    int columna, fila;
    eImagenes imgTile;
    SDL_Rect rectTile, rectEntidad = {.w = 48, .h = 48};

    rectTile.w = 48;

    for(fila = 0; fila < escenario->filas; fila++){

        for(columna = 0; columna < escenario->columnas; columna++){

            imgTile = escenario->tablero[fila][columna].tile.imagen;

            rectTile.x = columna * 48;
            rectTile.y = fila * 48;

            if(imgTile == IMAGEN_PARED_LAD_GRIS){

                rectTile.h = 64;
                rectTile.y -= 16;
            }else{
                rectTile.h = 48;
            }

            graficos_dibujar_textura(*(imagenes + imgTile), renderer, &escenario->tablero[fila][columna].tile.rectTile, &rectTile);

            if(escenario->tablero[fila][columna].entidad){

                rectEntidad.x = columna * 48;
                rectEntidad.y = fila * 48;

                graficos_dibujar_textura(*(imagenes + escenario->tablero[fila][columna].entidad->imagen), renderer, NULL, &rectEntidad);
            }
        }
    }
}



