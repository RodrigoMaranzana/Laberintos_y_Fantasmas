#ifndef ESCENARIO_H_INCLUDED
#define ESCENARIO_H_INCLUDED

#include <SDL.h>
#include "../include/entidad.h"
#include "../include/assets.h"
#include "../include/graficos.h"
#include "../include/temporizador.h"

#define CAPAS_X_CASILLA 2
#define NO_TRANSITABLE 0
#define TRANSITABLE 1

#define MIN(a,b)( (a) < (b) ? (a) : (b) )
#define MAX(a,b)( (a) > (b) ? (a) : (b) )

typedef void (*tile_funcion)(void *datos);

typedef enum {
    TILE_TECHO_0,
    TILE_TECHO_1,
    TILE_TECHO_2,
    TILE_TECHO_3,
    TILE_TECHO_4,
    TILE_TECHO_5,
    TILE_TECHO_6,
    TILE_TECHO_7,
    TILE_TECHO_8,
    TILE_TECHO_9,
    TILE_TECHO_10,
    TILE_TECHO_11,
    TILE_TECHO_12,
    TILE_TECHO_13,
    TILE_TECHO_14,
    TILE_TECHO_15,

    TILE_PISO_0,
    TILE_PISO_1,
    TILE_PISO_2,
    TILE_PISO_3,
    TILE_PISO_4,
    TILE_PISO_5,
    TILE_PISO_6,
    TILE_PISO_7,
    TILE_PISO_8,
    TILE_PISO_9,
    TILE_PISO_10,
    TILE_PISO_11,
    TILE_PISO_12,
    TILE_PISO_13,
    TILE_PISO_14,
    TILE_PISO_15,

    TILE_PARED_0,
    TILE_PARED_1,
    TILE_PARED_2,
    TILE_PARED_3,
    TILE_PARED_4,
    TILE_PARED_5,
    TILE_PARED_6,
    TILE_PARED_7,
    TILE_PARED_8,
    TILE_PARED_9,
    TILE_PARED_10,
    TILE_PARED_11,
    TILE_PARED_12,
    TILE_PARED_13,
    TILE_PARED_14,
    TILE_PARED_15,

    TILE_PARED_ANIM_0,
    TILE_PARED_ANIM_1,
    TILE_PARED_ANIM_2,
    TILE_PARED_ANIM_3,

    TILE_PUERTA_ENTRADA_0,
    TILE_PUERTA_ENTRADA_1,
    TILE_PUERTA_SALIDA_0,
    TILE_PUERTA_SALIDA_1,

    TILE_CANTIDAD,
} eTileID;

typedef enum {
    TILE_TIPO_TECHO,
    TILE_TIPO_PISO,
    TILE_TIPO_PARED,
    TILE_TIPO_PUERTA_ENTRADA,
    TILE_TIPO_PUERTA_SALIDA,
    TILE_TIPO_CANTIDAD,
} eTileTipo;

typedef struct {
    eTileTipo tileTipo;
    eTileID tileID;
    SDL_Rect coords;
} tTile;

typedef struct {
    int columna;
    int fila;
    tTile *tile;
    tEntidad *entidad;
    char transitable;
    char visitada;
} tCasilla;

typedef struct {
    int columnas;
    int filas;
    int cantVidasInicial;
    int cantVidasNivel;
    int cantFantasmas;
    int cantPremios;
}tConfig;

typedef struct {
    tConfig config;
    tCasilla **tablero;
    tEntidad jugador;
    tEntidad *fantasmas;
    eImagen tileSet;
    tTile tiles[TILE_CANTIDAD];
    long semilla;
    tTemporizador temporFrame;
    char frame;
} tEscenario;


int escenario_crear(tEscenario *escenario, unsigned columnas, unsigned filas);
void escenario_generar(tEscenario *escenario);
void escenario_destruir(tEscenario *escenario);
void escenario_dibujar(SDL_Renderer *renderer, tEscenario *escenario, SDL_Texture **imagenes);

#endif // ESCENARIO_H_INCLUDED
