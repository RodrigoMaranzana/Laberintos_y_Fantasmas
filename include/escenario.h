#ifndef ESCENARIO_H_INCLUDED
#define ESCENARIO_H_INCLUDED

#include <SDL.h>
#include "../include/entidad.h"
#include "../include/assets.h"
#include "../include/graficos.h"

typedef struct{
    eImagenes imagen;
    SDL_Rect rectTile;
    ///Puntero a funcion
}tTile;

typedef struct{
    tTile tile;
    tEntidad *entidad;
}tCasilla;

typedef struct{
    tCasilla **tablero;
    tEntidad jugador;
    tEntidad *fantasmas;
    unsigned columnas;
    unsigned filas;
}tEscenario;


int escenario_crear(tEscenario *escenario, unsigned columnas, unsigned filas);
void escenario_generar(tEscenario *escenario);
void escenario_destruir(tEscenario *escenario);
void escenario_dibujar(SDL_Renderer *renderer, tEscenario *escenario, SDL_Texture **imagenes);

#endif // ESCENARIO_H_INCLUDED
