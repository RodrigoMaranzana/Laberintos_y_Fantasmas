#ifndef ESCENARIO_H_INCLUDED
#define ESCENARIO_H_INCLUDED

#include <SDL.h>
#include "../../include/cliente/assets.h"
#include "../../include/cliente/temporizador.h"
#include "../../include/comun/comun.h"

#define CAPAS_X_CASILLA 2
#define NO_TRANSITABLE 0
#define TRANSITABLE 1


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

typedef enum {
    EXTRA_NINGUNO,
    EXTRA_VIDA,
    EXTRA_PREMIO,
} eExtra;

typedef enum {
    PARED_NINGUNA,
    PARED_SUPERIOR,
    PARED_INFERIOR,
    PARED_IZQUIERDA,
    PARED_DERECHA
} eParedLimite;

typedef enum {
    ENTIDAD_JUGADOR,
    ENTIDAD_FANTASMA_AMARILLO,
    ENTIDAD_FANTASMA_AZUL,
    ENTIDAD_FANTASMA_ROSA,
    ENTIDAD_FANTASMA_ROJO,
} eEntidadTipo;

typedef enum {
    ENTIDAD_CON_VIDA,
    ENTIDAD_SIN_VIDA,
    ENTIDAD_ATURDIDA,
    ENTIDAD_POTENCIADA,
} eEntidadEstado;

typedef enum {
    MIRANDO_ABAJO,
    MIRANDO_IZQUIERDA,
    MIRANDO_DERECHA,
    MIRANDO_ARRIBA,
} eOrientacion;

typedef struct {
    eTileTipo tileTipo;
    eTileID tileID;
    SDL_Rect coords;
} tTile;

typedef struct {
    int columna;
    int fila;
} tUbicacion;

typedef struct {
    eImagen imagen;
    eEntidadTipo tipo;
    tUbicacion ubic;
    tUbicacion ubicAnterior;
    eOrientacion orientacion;
    tTempor temporFrame;
    tTempor temporEstado;
    eEntidadEstado estado;
    char frame;
} tEntidad;

typedef struct {
    int columna;
    int fila;
    tTile *tile;
    tEntidad *entidad;
    eExtra extra;
    char transitable;
    char visitada;
    tUbicacion anteriorBFS;
} tCasilla;

typedef struct {
    tCasilla **tablero;
    int cantColumnas;
    int cantFilas;
    tUbicacion ubicPEntrada;
    tUbicacion ubicPSalida;
    tUbicacion puntoReaparicion;
    eImagen tileSet;
    tTile tiles[TILE_CANTIDAD];
    tTempor temporFrame;
    char frame;
} tEscenario;

int escenario_crear(tEscenario *escenario, unsigned columnas, unsigned filas);
void escenario_generar(tEscenario *escenario);
void escenario_destruir(tEscenario *escenario);
int escenario_calcular_mascara(tEscenario *escenario, int columna, int fila);
eParedLimite escenario_ubic_es_pared_limite(tEscenario *escenario, tUbicacion ubic);


#endif // ESCENARIO_H_INCLUDED
