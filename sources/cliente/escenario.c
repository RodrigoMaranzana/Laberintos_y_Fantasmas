#include "../../include/cliente/escenario.h"
#include "../../include/comun/matriz.h"
#include "../../include/comun/retorno.h"
#include "../../include/comun/pila.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define MARGEN_VENTANA 64

static void _escenario_init_tiles(tTile *tiles);
static void _escenario_generar_laberinto(tEscenario *escenario);
static void _escenario_calcular_puerta(tEscenario *escenario, int pared, int *fila, int *columna, int *mascara);
static int _escenario_calcular_mascara(tEscenario *escenario, int columna, int fila);
static void _escenario_colocar_puertas(tEscenario *escenario);
static void _escenario_colocar_jugador(tEscenario *escenario, int fila, int columna, int pared);
static void _escenario_colocar_fantasmas(tEscenario *escenario);
static void _escenario_dibujar_entidad(SDL_Renderer *renderer, SDL_Texture **imagenes, tEntidad *entidad);


int escenario_crear(tEscenario *escenario, unsigned columnas, unsigned filas)
{
    escenario->tablero = (tCasilla**) matriz_crear((size_t) columnas, (size_t) filas, sizeof(tCasilla));
    if (!escenario->tablero) {

        puts("ERROR: No se pudo crear el escenario");
        return ERR_SIN_MEMORIA;
    }

    escenario->config.cantFantasmas = 4;

    escenario->fantasmas = (tEntidad*)malloc(sizeof(tEntidad) * escenario->config.cantFantasmas);
    if (!escenario->fantasmas) {

        puts("ERROR: No se pudo crear el escenario");
        return ERR_SIN_MEMORIA;
    }

    escenario->config.columnas = columnas;
    escenario->config.filas = filas;


    escenario->semilla = time(NULL);
    srand(escenario->semilla);

    escenario->tileSet = IMAGEN_TSET_CASTILLO;
    escenario->frame = 0;
    temporizador_inicializar(&escenario->temporFrame, 0.2f);
    temporizador_iniciar(&escenario->temporFrame);

    _escenario_init_tiles(escenario->tiles);

    return TODO_OK;
}

static void _escenario_init_tiles(tTile *tiles)
{
    int i;
    for (i = TILE_TECHO_0; i <= TILE_TECHO_15; i++) {
        tiles[i].tileTipo = TILE_TIPO_TECHO;
        tiles[i].tileID = i;
        tiles[i].coords.x = ((i - TILE_TECHO_0) % 4) * PIXELES_TILE;
        tiles[i].coords.y = ((i - TILE_TECHO_0) / 4) * PIXELES_TILE;
        tiles[i].coords.w = PIXELES_TILE;
        tiles[i].coords.h = PIXELES_TILE;
    }

    for (i = TILE_PISO_0; i <= TILE_PISO_15; i++) {
        tiles[i].tileTipo = TILE_TIPO_PISO;
        tiles[i].tileID = i;
        tiles[i].coords.x =  192 + (((i - TILE_PISO_0) % 4) * PIXELES_TILE);
        tiles[i].coords.y = ((i - TILE_PISO_0) / 4) * PIXELES_TILE;
        tiles[i].coords.w = PIXELES_TILE;
        tiles[i].coords.h = PIXELES_TILE;
    }

    for (i = TILE_PARED_0; i <= TILE_PARED_15; i++) {
        tiles[i].tileTipo = TILE_TIPO_PARED;
        tiles[i].tileID = i;
        tiles[i].coords.x = ((i - TILE_PARED_0) % 4) * PIXELES_TILE;
        tiles[i].coords.y = 192 + (((i - TILE_PARED_0) / 4) * PIXELES_TILE);
        tiles[i].coords.w = PIXELES_TILE;
        tiles[i].coords.h = PIXELES_TILE;
    }

    for (i = TILE_PARED_ANIM_0; i <= TILE_PARED_ANIM_3; i++) {
        tiles[i].tileTipo = TILE_TIPO_PARED;
        tiles[i].tileID = i;
        tiles[i].coords.x = 192;
        tiles[i].coords.y = 192 + ((i - TILE_PARED_ANIM_0) * PIXELES_TILE);
        tiles[i].coords.w = PIXELES_TILE;
        tiles[i].coords.h = PIXELES_TILE;
    }

    tiles[TILE_PUERTA_ENTRADA_0].tileTipo = TILE_TIPO_PUERTA_ENTRADA;
    tiles[TILE_PUERTA_ENTRADA_0].tileID = TILE_PUERTA_ENTRADA_0;
    tiles[TILE_PUERTA_ENTRADA_0].coords = (SDL_Rect) {.x = 0, .y = 384, .w = PIXELES_TILE, .h = 2 * PIXELES_TILE};

    tiles[TILE_PUERTA_ENTRADA_1].tileTipo = TILE_TIPO_PUERTA_ENTRADA;
    tiles[TILE_PUERTA_ENTRADA_1].tileID = TILE_PUERTA_ENTRADA_1;
    tiles[TILE_PUERTA_ENTRADA_1].coords = (SDL_Rect) {.x = 96, .y = 384, .w = PIXELES_TILE, .h = 2 * PIXELES_TILE};

    tiles[TILE_PUERTA_SALIDA_0].tileTipo = TILE_TIPO_PUERTA_SALIDA;
    tiles[TILE_PUERTA_SALIDA_0].tileID = TILE_PUERTA_SALIDA_0;
    tiles[TILE_PUERTA_SALIDA_0].coords = (SDL_Rect) {.x = 48, .y = 384, .w = PIXELES_TILE, .h = 2 * PIXELES_TILE};

    tiles[TILE_PUERTA_SALIDA_1].tileTipo = TILE_TIPO_PUERTA_SALIDA;
    tiles[TILE_PUERTA_SALIDA_1].tileID = TILE_PUERTA_SALIDA_1;
    tiles[TILE_PUERTA_SALIDA_1].coords = (SDL_Rect) {.x = 144, .y = 384, .w = PIXELES_TILE, .h = 2 * PIXELES_TILE};

}

static void _escenario_generar_laberinto(tEscenario *escenario)
{
    tPila pila;
    tCasilla *casTope, *casAdya, *casElegida;
    int desplazamientos[] = { 0, 0, 2, -2 };
    int i, offsetY, offsetX, columAdya, filaAdya;
    tCasilla* vecCasAdya[4];
    int numCasAdya;

    pila_crear(&pila);
    casTope = &escenario->tablero[1][1];
    casTope->tile = &escenario->tiles[TILE_PISO_0];
    pila_apilar(&pila, &casTope, sizeof(tCasilla*));


    while (pila_vacia(&pila) == TODO_OK) {

        pila_tope(&pila, &casTope, sizeof(tCasilla*));

        for (numCasAdya = 0, i = 0; i < 4; i++) {

            offsetX = desplazamientos[i];
            offsetY = desplazamientos[(i + 2) % 4];
            columAdya = casTope->columna + offsetX;
            filaAdya = casTope->fila + offsetY;

            if ((columAdya >= 1) && (columAdya < escenario->config.columnas - 1) && (filaAdya >= 1) && (filaAdya < escenario->config.filas - 1)) {

                casAdya = &escenario->tablero[filaAdya][columAdya];
                if (casAdya->tile->tileTipo == TILE_TIPO_PARED) {

                    vecCasAdya[numCasAdya] = casAdya;
                    numCasAdya++;
                }
            }
        }

        if (numCasAdya > 0) {

            casElegida = vecCasAdya[rand() % numCasAdya];

            offsetX = casElegida->columna - casTope->columna;
            offsetY = casElegida->fila - casTope->fila;

            escenario->tablero[casTope->fila + (offsetY / 2)][casTope->columna + (offsetX / 2)].tile = &escenario->tiles[TILE_PISO_0];
            escenario->tablero[casTope->fila + (offsetY / 2)][casTope->columna + (offsetX / 2)].transitable = 1;
            casElegida->tile = &escenario->tiles[TILE_PISO_0];
            casElegida->transitable = 1;;

            pila_apilar(&pila, &casElegida, sizeof(tCasilla*));

        } else {

            pila_desapilar(&pila, &casTope, sizeof(tCasilla*));
        }
    }

    pila_vaciar(&pila);
}

static void _escenario_postprocesar(tEscenario *escenario)
{
    int fila, columna;

    for (fila = 1; fila < escenario->config.filas - 1; fila++) {

        for (columna = 1; columna < escenario->config.columnas - 1; columna++) {

            if (!(rand() % 7)) {

                escenario->tablero[fila][columna].tile = &escenario->tiles[TILE_PISO_0];
                escenario->tablero[fila][columna].transitable = 1;
            }
        }
    }
}

void escenario_generar(tEscenario *escenario)
{
    int fila, columna, mascara;
    unsigned int semillaPared;
    int indicePared;

    for (fila = 0; fila < escenario->config.filas; fila++) {

        for (columna = 0; columna < escenario->config.columnas; columna++) {

            semillaPared = escenario->semilla + columna * 13 + fila * 7;

            if(fila != escenario->config.filas - 1){

                indicePared = rand() % 4 ? TILE_PARED_0 + (semillaPared % 16) : TILE_PARED_ANIM_0 + (semillaPared % 4);
            }else{
                indicePared = TILE_PARED_0;
            }

            escenario->tablero[fila][columna].tile = &escenario->tiles[indicePared];
            escenario->tablero[fila][columna].entidad = NULL;
            escenario->tablero[fila][columna].fila = fila;
            escenario->tablero[fila][columna].columna = columna;
            escenario->tablero[fila][columna].transitable = 0;
        }
    }

    _escenario_generar_laberinto(escenario);
    _escenario_postprocesar(escenario);
    _escenario_colocar_puertas(escenario);
    _escenario_colocar_fantasmas(escenario);

    for (fila = 0; fila < escenario->config.filas; fila++) {

        for (columna = 0; columna < escenario->config.columnas; columna++) {

            if (escenario->tablero[fila][columna].tile->tileTipo == TILE_TIPO_PISO) {

                mascara = _escenario_calcular_mascara(escenario, columna, fila);

                escenario->tablero[fila][columna].tile = &escenario->tiles[escenario->tablero[fila][columna].tile->tileID + mascara];
            }
        }
    }
}

static int _escenario_calcular_mascara(tEscenario *escenario, int columna, int fila)
{
    int mascara = 0;

    if (fila > 0 && escenario->tablero[fila - 1][columna].tile->tileTipo == TILE_TIPO_PARED) {

        mascara += 1;
    }

    if (columna < escenario->config.columnas - 1 && escenario->tablero[fila][columna + 1].tile->tileTipo == TILE_TIPO_PARED) {

        mascara += 2;
    }

    if (fila < escenario->config.filas - 1 && escenario->tablero[fila + 1][columna].tile->tileTipo == TILE_TIPO_PARED) {

        mascara += 4;
    }

    if (columna > 0 && escenario->tablero[fila][columna - 1].tile->tileTipo == TILE_TIPO_PARED) {

        mascara += 8;
    }

    return mascara;
}

static void _escenario_colocar_puertas(tEscenario *escenario)
{
    int paredEntrada, paredSalida;
    int fila, columna, mascara;

    paredEntrada = rand() % 4;
    _escenario_calcular_puerta(escenario, paredEntrada, &fila, &columna, &mascara);
    escenario->tablero[fila][columna].tile = &escenario->tiles[TILE_PUERTA_ENTRADA_0 + mascara];

    _escenario_colocar_jugador(escenario, fila, columna, paredEntrada);

    do {
        paredSalida = rand() % 4;
    } while (paredSalida == paredEntrada);

    _escenario_calcular_puerta(escenario, paredSalida, &fila, &columna, &mascara);
    escenario->tablero[fila][columna].tile = &escenario->tiles[TILE_PUERTA_SALIDA_0 + mascara];
    escenario->tablero[fila][columna].transitable = 1;
}

static void _escenario_calcular_puerta(tEscenario *escenario, int pared, int *fila, int *columna, int *mascara)
{
    *mascara = 0;

    switch (pared) {
        case 0: /// Pared superior
            *fila = 0;
            *columna = (rand() % (escenario->config.columnas - 2)) + 1;
            break;
        case 1: /// Pared inferior
            *fila = escenario->config.filas - 1;
            *columna = (rand() % (escenario->config.columnas - 2)) + 1;
            break;
        case 2: /// Pared izquierda
            *columna = 0;
            *fila = (rand() % (escenario->config.filas - 2)) + 1;
            *mascara = 1;
            break;
        case 3: /// Pared derecha
            *columna = escenario->config.columnas - 1;
            *fila = (rand() % (escenario->config.filas - 2)) + 1;
            *mascara = 1;
            break;
        default:
            break;
    }
}

static void _escenario_colocar_jugador(tEscenario *escenario, int fila, int columna, int pared)
{
    switch (pared) {
        case 0:
            escenario->jugador.orientacion = MIRANDO_ABAJO;
            fila++;
            break;
        case 1:
            escenario->jugador.orientacion = MIRANDO_ARRIBA;
            fila--;
            break;
        case 2:
            escenario->jugador.orientacion = MIRANDO_DERECHA;
            columna++;
            break;
        case 3:
            escenario->jugador.orientacion = MIRANDO_IZQUIERDA;
            columna--;
            break;
    }

    escenario->jugador.ubic.columna = columna;
    escenario->jugador.ubic.fila = fila;
    escenario->jugador.imagen = IMAGEN_JUGADOR;
    escenario->jugador.frame = 0;

    temporizador_inicializar(&escenario->jugador.temporFrame, 0.25f);
    temporizador_iniciar(&escenario->jugador.temporFrame);

    escenario->tablero[fila][columna].tile = &escenario->tiles[TILE_PISO_0];
    escenario->tablero[fila][columna].entidad = &escenario->jugador;
    escenario->tablero[fila][columna].transitable = 1;
}

static void _escenario_colocar_fantasmas(tEscenario *escenario)
{
    int columna, fila;
    tEntidad *pFantasma, *pFantasmaUlt = escenario->fantasmas + (escenario->config.cantFantasmas - 1);

    for (pFantasma = escenario->fantasmas; pFantasma <= pFantasmaUlt; pFantasma++) {

        do {

            columna = rand() % escenario->config.columnas;
            fila = rand() % escenario->config.filas;

        } while(escenario->tablero[fila][columna].tile->tileTipo != TILE_TIPO_PISO || escenario->tablero[fila][columna].entidad);

        pFantasma->orientacion = rand() % 4;
        pFantasma->frame = rand() % 4;
        pFantasma->ubic.columna = columna;
        pFantasma->ubic.fila = fila;
        pFantasma->imagen = IMAGEN_FANTASMA_01 + ((pFantasmaUlt - pFantasma) % 4) ;

        temporizador_inicializar(&pFantasma->temporFrame, 0.25f + ((float)(rand() % 101)) / 1000.0f);
        temporizador_iniciar(&pFantasma->temporFrame);

        escenario->tablero[fila][columna].entidad = pFantasma;
    }
}

void escenario_destruir(tEscenario *escenario)
{
    if (escenario->tablero) {

        matriz_destruir((void**) escenario->tablero, escenario->config.filas);
    }

    free(escenario->fantasmas);
}


void escenario_dibujar(SDL_Renderer *renderer, tEscenario *escenario, SDL_Texture **imagenes)
{
    int columna, fila, mascara;
    eTileTipo tileTipo;
    SDL_Rect rectTileDst;

    for (fila = 0; fila < escenario->config.filas; fila++) {

        for (columna = 0; columna < escenario->config.columnas; columna++) {

            tileTipo = escenario->tablero[fila][columna].tile->tileTipo;

            rectTileDst.x = columna * PIXELES_TILE + MARGEN_VENTANA / 2;
            rectTileDst.y = fila * PIXELES_TILE + (MARGEN_VENTANA / 2) + 8;
            rectTileDst.w = escenario->tablero[fila][columna].tile->coords.w;
            rectTileDst.h = escenario->tablero[fila][columna].tile->coords.h;

            if (tileTipo != TILE_TIPO_PUERTA_ENTRADA && tileTipo != TILE_TIPO_PUERTA_SALIDA) {

                SDL_Rect rectTileSrc = escenario->tablero[fila][columna].tile->coords;

                if (escenario->tablero[fila][columna].tile->tileID >= TILE_PARED_ANIM_0
                    && escenario->tablero[fila][columna].tile->tileID <= TILE_PARED_ANIM_3) {

                    rectTileSrc.x += escenario->frame * PIXELES_TILE;
                }

                graficos_dibujar_textura(*(imagenes + escenario->tileSet), renderer, &rectTileSrc, &rectTileDst);

                if (tileTipo == TILE_TIPO_PARED) {

                    mascara = _escenario_calcular_mascara(escenario, columna, fila);

                    rectTileDst.y -= PIXELES_TILE;

                    graficos_dibujar_textura(*(imagenes + escenario->tileSet), renderer, &escenario->tiles[TILE_TECHO_0 + mascara].coords, &rectTileDst);
                }
            }

            if (tileTipo == TILE_TIPO_PUERTA_ENTRADA || tileTipo == TILE_TIPO_PUERTA_SALIDA) {

                rectTileDst.h = 2 * PIXELES_TILE;
                rectTileDst.y -= PIXELES_TILE;

                graficos_dibujar_textura(*(imagenes + escenario->tileSet), renderer, &escenario->tablero[fila][columna].tile->coords, &rectTileDst);
            }

            if (escenario->tablero[fila][columna].entidad) {

                _escenario_dibujar_entidad(renderer, imagenes, escenario->tablero[fila][columna].entidad);
            }
        }
    }
}

static void _escenario_dibujar_entidad(SDL_Renderer *renderer, SDL_Texture **imagenes, tEntidad *entidad)
{
    SDL_Rect rectEntidadDst, rectEntidadSrc;

    rectEntidadSrc.x = entidad->frame * PIXELES_TILE;
    rectEntidadSrc.y = entidad->orientacion * PIXELES_TILE;
    rectEntidadSrc.w = PIXELES_TILE;
    rectEntidadSrc.h = PIXELES_TILE;

    rectEntidadDst.x = entidad->ubic.columna * PIXELES_TILE + MARGEN_VENTANA / 2;
    rectEntidadDst.y = entidad->ubic.fila * PIXELES_TILE + MARGEN_VENTANA / 2 + 8;
    rectEntidadDst.w = PIXELES_TILE;
    rectEntidadDst.h = PIXELES_TILE;

    graficos_dibujar_textura(*(imagenes + entidad->imagen), renderer, &rectEntidadSrc, &rectEntidadDst);
}


