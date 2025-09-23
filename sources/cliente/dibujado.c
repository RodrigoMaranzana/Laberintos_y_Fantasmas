#include "../../include/cliente/dibujado.h"
#include "../../include/cliente/temporizador.h"

#include <math.h>

static void _dibujado_jugador_aturdido(SDL_Texture *textura);
static void _dibujado_fantasma_aturdido(SDL_Texture *textura);
static void _dibujado_entidad(SDL_Renderer *renderer, SDL_Texture **imagenes, tEntidad *entidad);
static void _dibujado_jugador_potenciado(SDL_Texture *textura);

void dibujado_escenario(SDL_Renderer *renderer, tEscenario *escenario, SDL_Texture **imagenes)
{
    int columna, fila, mascara;
    eTileTipo tileTipo;
    SDL_Rect rectTileDst;

    temporizador_actualizar(&escenario->temporFrame);
    if (temporizador_estado(&escenario->temporFrame) == TEMPOR_FINALIZADO) {

        escenario->frame = (escenario->frame + 1) % 4;
        temporizador_iniciar(&escenario->temporFrame);
    }

    for (fila = 0; fila < escenario->cantFilas; fila++) {

        for (columna = 0; columna < escenario->cantColumnas; columna++) {

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

                graficos_dibujar_textura(*(imagenes + escenario->tileSet), renderer, &rectTileSrc, &rectTileDst, NULL);

                if (tileTipo == TILE_TIPO_PARED) {

                    mascara = escenario_calcular_mascara(escenario, columna, fila);

                    rectTileDst.y -= PIXELES_TILE;

                    graficos_dibujar_textura(*(imagenes + escenario->tileSet), renderer, &escenario->tiles[TILE_TECHO_0 + mascara].coords, &rectTileDst, NULL);
                }
            }

            if (tileTipo == TILE_TIPO_PUERTA_ENTRADA || tileTipo == TILE_TIPO_PUERTA_SALIDA) {

                rectTileDst.h = 2 * PIXELES_TILE;
                rectTileDst.y -= PIXELES_TILE;

                graficos_dibujar_textura(*(imagenes + escenario->tileSet), renderer, &escenario->tablero[fila][columna].tile->coords, &rectTileDst, NULL);
            }

            switch (escenario->tablero[fila][columna].extra) {

                case EXTRA_PREMIO: {
                    graficos_dibujar_textura(*(imagenes + IMAGEN_PREMIO), renderer, NULL, &rectTileDst, NULL);
                    break;
                }
                case EXTRA_VIDA: {
                    graficos_dibujar_textura(*(imagenes + IMAGEN_VIDA), renderer, NULL, &rectTileDst, NULL);
                    break;
                }
                default:
                    break;
            }

            if (escenario->tablero[fila][columna].entidad) {

                _dibujado_entidad(renderer, imagenes, escenario->tablero[fila][columna].entidad);
            }
        }
    }
}


/*************************
    FUNCIONES ESTATICAS
*************************/

static void _dibujado_jugador_aturdido(SDL_Texture *textura)
{
    SDL_SetTextureColorMod(textura, 255, 100, 100);
}

static void _dibujado_jugador_potenciado(SDL_Texture *textura)
{
    char r, g, b;
    unsigned tick = SDL_GetTicks();
    float tiempo = tick * 0.005f;

    r = (char)(128 + (255 * (sin(tiempo + 1) + 1) / 2));
    g = (char)(128 + (255 * (sin(tiempo + 2) + 1) / 2));
    b = (char)(128 + (255 * (sin(tiempo + 4) + 1) / 2));

    SDL_SetTextureColorMod(textura, r, g, b);
    SDL_SetTextureBlendMode(textura, SDL_BLENDMODE_ADD);
}

static void _dibujado_fantasma_aturdido(SDL_Texture *textura)
{
    SDL_SetTextureColorMod(textura, 100, 100, 255);
}

static void _dibujado_entidad(SDL_Renderer *renderer, SDL_Texture **imagenes, tEntidad *entidad)
{
    SDL_Rect rectEntidadDst, rectEntidadSrc;
    SDL_Texture *texturaEntidad = *(imagenes + entidad->imagen);
    tEfectoGrafico efecto = NULL;

    if (entidad->estado == ENTIDAD_ATURDIDA) {

        if ((SDL_GetTicks() / 128) % 2 == 0) {

            efecto = entidad->tipo == ENTIDAD_JUGADOR ? _dibujado_jugador_aturdido : _dibujado_fantasma_aturdido;
        }
    } else if (entidad->estado == ENTIDAD_POTENCIADA) {

        if ((SDL_GetTicks() / 128) % 2 == 0) {

            efecto = entidad->tipo == ENTIDAD_JUGADOR ? _dibujado_jugador_potenciado : _dibujado_fantasma_aturdido;
        }

    } else if (entidad->estado == ENTIDAD_SIN_VIDA) {

        return;
    }

    rectEntidadSrc.x = entidad->frame * PIXELES_TILE;
    rectEntidadSrc.y = entidad->orientacion * PIXELES_TILE;
    rectEntidadSrc.w = PIXELES_TILE;
    rectEntidadSrc.h = PIXELES_TILE;

    rectEntidadDst.x = entidad->ubic.columna * PIXELES_TILE + MARGEN_VENTANA / 2;
    rectEntidadDst.y = entidad->ubic.fila * PIXELES_TILE + MARGEN_VENTANA / 2 + 8;
    rectEntidadDst.w = PIXELES_TILE;
    rectEntidadDst.h = PIXELES_TILE;

    graficos_dibujar_textura(texturaEntidad, renderer, &rectEntidadSrc, &rectEntidadDst, efecto);
}
