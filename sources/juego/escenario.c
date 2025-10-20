#include "../../include/juego/escenario.h"
#include "../../include/comun/matriz.h"
#include "../../include/comun/pila.h"
#include "../../include/comun/mensaje.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static void _escenario_init_tiles(tTile *tiles);
static int _escenario_generar_laberinto(tEscenario *escenario);
static void _escenario_postprocesar(tEscenario *escenario);
static void _escenario_calcular_puerta(tCasilla *puerta, tTile *tiles, int esEntrada, int pared, int cantFilas, int cantColumnas);
static void _escenario_colocar_puertas(tEscenario *escenario);
static void _escenario_despejar_puerta(tEscenario *escenario, tUbicacion ubicPuerta, int pared);
static void _escenario_liberar_casilla(tEscenario *escenario, int fila, int columna);
static int _escenario_colocar_extras(tEscenario *escenario, int cantPisos, int cantPremios, int cantVidas);

int escenario_crear(tEscenario *escenario, unsigned columnas, unsigned filas)
{
    escenario->tablero = (tCasilla**) matriz_crear((size_t) columnas, (size_t) filas, sizeof(tCasilla));
    if (!escenario->tablero) {

        mensaje_error("No se pudo crear el escenario");
        return ERR_SIN_MEMORIA;
    }

    escenario->cantColumnas = columnas;
    escenario->cantFilas = filas;

    _escenario_init_tiles(escenario->tiles);

    return ERR_TODO_OK;
}

eParedLimite escenario_ubic_es_pared_limite(const tEscenario *escenario, tUbicacion ubic)
{
    if (ubic.fila == 0) {
        return PARED_SUPERIOR;
    }

    if (ubic.fila == escenario->cantFilas - 1) {
        return PARED_INFERIOR;
    }

    if (ubic.columna == 0) {
        return PARED_IZQUIERDA;
    }

    if (ubic.columna == escenario->cantColumnas - 1) {
        return PARED_DERECHA;
    }

    return PARED_NINGUNA;
}

void escenario_generar(tEscenario *escenario, int cantPremios, int cantVidas)
{
    int fila, columna, mascara, cantPisos = 0;
    unsigned int semillaPared;
    int indicePared;

    escenario->tileSet = IMAGEN_TSET_CASTILLO; /// ESCALABLE CON rand() SI TENEMOS MAS DE UN TILESET
    escenario->frame = 0;

    for (fila = 0; fila < escenario->cantFilas; fila++) {

        for (columna = 0; columna < escenario->cantColumnas; columna++) {

            semillaPared = rand() + columna * 13 + fila * 7;

            if(fila != escenario->cantFilas - 1){

                indicePared = rand() % 4 ? TILE_PARED_0 + (semillaPared % 16) : TILE_PARED_ANIM_0 + (semillaPared % 4);
            }else{
                indicePared = TILE_PARED_0;
            }

            escenario->tablero[fila][columna].tile = &escenario->tiles[indicePared];
            escenario->tablero[fila][columna].entidad = NULL;
            escenario->tablero[fila][columna].fila = fila;
            escenario->tablero[fila][columna].columna = columna;
            escenario->tablero[fila][columna].transitable = 0;
            escenario->tablero[fila][columna].extra = EXTRA_NINGUNO;
        }
    }

    _escenario_generar_laberinto(escenario);
    _escenario_postprocesar(escenario);
    _escenario_colocar_puertas(escenario);

    for (fila = 0; fila < escenario->cantFilas; fila++) {

        for (columna = 0; columna < escenario->cantColumnas; columna++) {

            if (escenario->tablero[fila][columna].tile->tileTipo == TILE_TIPO_PISO) {

                mascara = escenario_calcular_mascara(escenario, columna, fila);
                escenario->tablero[fila][columna].tile = &escenario->tiles[escenario->tablero[fila][columna].tile->tileID + mascara];
                ++cantPisos;
            }
        }
    }

    _escenario_colocar_extras(escenario, cantPisos, cantPremios, cantVidas);

    temporizador_inicializar(&escenario->temporFrame, 0.2f);
    temporizador_iniciar(&escenario->temporFrame);
}

static int _escenario_colocar_extras(tEscenario *escenario, int cantPisos, int cantPremios, int cantVidas)
{
    int i, j, contador = 0, pisoActual = 0, fila, columna;
    SDL_Point *pisosDisponibles, coord;

    if (cantPisos < (cantPremios + cantVidas) || cantPisos == 0) {
        return ESC_ERR_PARAMS;
    }

    pisosDisponibles = malloc(sizeof(SDL_Point) * cantPisos);
    if (!pisosDisponibles) {
        return ESC_ERR_SIN_MEM;
    }

    for (fila = 0; fila < escenario->cantFilas; fila++) {

        for (columna = 0; columna < escenario->cantColumnas; columna++) {

            if (escenario->tablero[fila][columna].tile->tileTipo == TILE_TIPO_PISO) {
                pisosDisponibles[contador].y = fila;
                pisosDisponibles[contador].x = columna;
                contador++;
            }
        }
    }

    for (i = cantPisos - 1; i > 0; --i) {

        j = rand() % (i + 1);
        coord = pisosDisponibles[i];
        pisosDisponibles[i] = pisosDisponibles[j];
        pisosDisponibles[j] = coord;
    }

    for (pisoActual = 0; pisoActual < cantPremios + cantVidas; pisoActual++) {

        coord = pisosDisponibles[pisoActual];
        if (pisoActual < cantPremios) {
            escenario->tablero[coord.y][coord.x].extra = EXTRA_PREMIO;
        } else {
            escenario->tablero[coord.y][coord.x].extra = EXTRA_VIDA;
        }
    }

    free(pisosDisponibles);

    return ESC_TODO_OK;
}

int escenario_calcular_mascara(tEscenario *escenario, int columna, int fila)
{
    int mascara = 0;

    if (fila > 0 && escenario->tablero[fila - 1][columna].tile->tileTipo == TILE_TIPO_PARED) mascara += 1;

    if (columna < escenario->cantColumnas - 1 && escenario->tablero[fila][columna + 1].tile->tileTipo == TILE_TIPO_PARED) mascara += 2;

    if (fila < escenario->cantFilas - 1 && escenario->tablero[fila + 1][columna].tile->tileTipo == TILE_TIPO_PARED) mascara += 4;

    if (columna > 0 && escenario->tablero[fila][columna - 1].tile->tileTipo == TILE_TIPO_PARED) mascara += 8;

    return mascara;
}

void escenario_destruir(tEscenario *escenario)
{
    if (escenario->tablero) {
        matriz_destruir((void**) escenario->tablero, escenario->cantFilas);
    }

    /// REVISAR QUE NO HAYA NADA MAS QUE DESTRUIR/LIBERAR
}


/*************************
    FUNCIONES ESTATICAS
*************************/

static int _escenario_generar_laberinto(tEscenario *escenario)
{
    tPila pila;
    tCasilla *casTope, *casAdya, *casElegida;
    int desplazamientos[] = { 0, 0, 2, -2 };
    int i, offsetY, offsetX, columAdya, filaAdya;
    tCasilla* vecCasAdya[4];
    int numCasAdya, contadorPisos = 1;

    pila_crear(&pila);
    casTope = &escenario->tablero[1][1];
    casTope->tile = &escenario->tiles[TILE_PISO_0];
    casTope->transitable = 1;
    pila_apilar(&pila, &casTope, sizeof(tCasilla*));

    while (pila_vacia(&pila) == ERR_TODO_OK) {

        pila_tope(&pila, &casTope, sizeof(tCasilla*));

        for (numCasAdya = 0, i = 0; i < 4; i++) {

            offsetX = desplazamientos[i];
            offsetY = desplazamientos[(i + 2) % 4];
            columAdya = casTope->columna + offsetX;
            filaAdya = casTope->fila + offsetY;

            if ((columAdya >= 1) && (columAdya < escenario->cantColumnas - 1) && (filaAdya >= 1) && (filaAdya < escenario->cantFilas - 1)) {

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
            casElegida->transitable = 1;
            ++contadorPisos;

            pila_apilar(&pila, &casElegida, sizeof(tCasilla*));

        } else {

            pila_desapilar(&pila, &casTope, sizeof(tCasilla*));
        }
    }

    pila_vaciar(&pila);

    return contadorPisos;
}

static void _escenario_postprocesar(tEscenario *escenario)
{
    int fila, columna;

    for (fila = 1; fila < escenario->cantFilas - 1; fila++) {

        for (columna = 1; columna < escenario->cantColumnas - 1; columna++) {

            if (!(rand() % 7)) {
                escenario->tablero[fila][columna].tile = &escenario->tiles[TILE_PISO_0];
                escenario->tablero[fila][columna].transitable = 1;
            }
        }
    }

    if (escenario->cantFilas % 2 == 0) {

        for (columna = 1; columna < escenario->cantColumnas - 1; columna++) {

            if ((rand() % 6) < 5) {
                escenario->tablero[escenario->cantFilas - 2][columna].tile = &escenario->tiles[TILE_PISO_0];
                escenario->tablero[escenario->cantFilas - 2][columna].transitable = 1;
            }
        }
    }

    if (escenario->cantColumnas % 2 == 0) {

        for (fila = 1; fila < escenario->cantFilas - 1; fila++) {

            if ((rand() % 6) < 5) {
                escenario->tablero[fila][escenario->cantColumnas - 2].tile = &escenario->tiles[TILE_PISO_0];
                escenario->tablero[fila][escenario->cantColumnas - 2].transitable = 1;
            }
        }
    }
}

static void _escenario_colocar_puertas(tEscenario *escenario)
{
    tCasilla puerta = {0};
    int pared;
    int paredesPosibles[4] = {PARED_SUPERIOR, PARED_INFERIOR, PARED_IZQUIERDA, PARED_DERECHA};

    pared = paredesPosibles[rand() % 4];
    _escenario_calcular_puerta(&puerta, escenario->tiles, 1, pared, escenario->cantFilas, escenario->cantColumnas);
    escenario->tablero[puerta.fila][puerta.columna] = puerta;

    escenario->ubicPEntrada.columna = puerta.columna;
    escenario->ubicPEntrada.fila = puerta.fila;

    _escenario_despejar_puerta(escenario, escenario->ubicPEntrada, pared);

    if (pared == PARED_SUPERIOR){
        pared = PARED_INFERIOR;
    } else if (pared == PARED_INFERIOR) {
        pared = PARED_SUPERIOR;
    } else if (pared == PARED_IZQUIERDA) {
        pared = PARED_DERECHA;
    } else {
        pared = PARED_IZQUIERDA;
    }

    _escenario_calcular_puerta(&puerta, escenario->tiles, 0, pared, escenario->cantFilas, escenario->cantColumnas);
    escenario->tablero[puerta.fila][puerta.columna] = puerta;

    escenario->ubicPSalida.columna = puerta.columna;
    escenario->ubicPSalida.fila = puerta.fila;

    _escenario_despejar_puerta(escenario, escenario->ubicPSalida, pared);
}

static void _escenario_calcular_puerta(tCasilla *puerta, tTile *tiles, int esEntrada, int pared, int cantFilas, int cantColumnas)
{
    tTile *pTile = tiles;

    switch (pared) {
        case PARED_SUPERIOR:
            puerta->fila = 0;
            puerta->columna = (rand() % (cantColumnas - 2)) + 1;
            break;
        case PARED_INFERIOR:
            puerta->fila = cantFilas - 1;
            puerta->columna = (rand() % (cantColumnas - 2)) + 1;
            break;
        case PARED_IZQUIERDA:
            puerta->columna = 0;
            puerta->fila = (rand() % (cantFilas - 2)) + 1;
            break;
        case PARED_DERECHA:
            puerta->columna = cantColumnas - 1;
            puerta->fila = (rand() % (cantFilas - 2)) + 1;
            break;
        default:
            break;
    }

    if (pared == PARED_SUPERIOR || pared == PARED_INFERIOR) {
        puerta->tile = esEntrada ? (pTile + TILE_PUERTA_ENTRADA_0) : (pTile + TILE_PUERTA_SALIDA_0);
    }else {
        puerta->tile = esEntrada ? (pTile + TILE_PUERTA_ENTRADA_1) : (pTile + TILE_PUERTA_SALIDA_1);
    }

    puerta->entidad = NULL;
    puerta->transitable = esEntrada ? 0 : 1;
}

static void _escenario_despejar_puerta(tEscenario *escenario, tUbicacion ubicPuerta, int pared)
{
    int fila, columna;
    tUbicacion ubicInterior = ubicPuerta;

    switch (pared) {
        case PARED_SUPERIOR:
            ++ubicInterior.fila;
            break;
        case PARED_INFERIOR:
            --ubicInterior.fila;
            break;
        case PARED_IZQUIERDA:
            ++ubicInterior.columna;
            break;
        case PARED_DERECHA:
            --ubicInterior.columna;
            break;
    }

    for (fila = ubicInterior.fila - 1; fila <= ubicInterior.fila + 1; ++fila) {

        for (columna = ubicInterior.columna - 1; columna <= ubicInterior.columna + 1; ++columna) {

            _escenario_liberar_casilla(escenario, fila, columna);
        }
    }
}

static void _escenario_liberar_casilla(tEscenario *escenario, int fila, int columna)
{
    if (fila <= 0 || fila >= escenario->cantFilas - 1 || columna <= 0 || columna >= escenario->cantColumnas - 1) {
        return;
    }

    escenario->tablero[fila][columna].tile = &escenario->tiles[TILE_PISO_0];
    escenario->tablero[fila][columna].transitable = 1;
    escenario->tablero[fila][columna].entidad = NULL;
    escenario->tablero[fila][columna].extra = EXTRA_NINGUNO;
}

static void _escenario_init_tiles(tTile *tiles)
{
    int i;
    for (i = TILE_TECHO_0; i <= TILE_TECHO_15; ++i) {
        tiles[i].tileTipo = TILE_TIPO_TECHO;
        tiles[i].tileID = i;
        tiles[i].coords.x = ((i - TILE_TECHO_0) % 4) * PIXELES_TILE;
        tiles[i].coords.y = ((i - TILE_TECHO_0) / 4) * PIXELES_TILE;
        tiles[i].coords.w = PIXELES_TILE;
        tiles[i].coords.h = PIXELES_TILE;
    }

    for (i = TILE_PISO_0; i <= TILE_PISO_15; ++i) {
        tiles[i].tileTipo = TILE_TIPO_PISO;
        tiles[i].tileID = i;
        tiles[i].coords.x =  192 + (((i - TILE_PISO_0) % 4) * PIXELES_TILE);
        tiles[i].coords.y = ((i - TILE_PISO_0) / 4) * PIXELES_TILE;
        tiles[i].coords.w = PIXELES_TILE;
        tiles[i].coords.h = PIXELES_TILE;
    }

    for (i = TILE_PARED_0; i <= TILE_PARED_15; ++i) {
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
