#include <stdio.h>
#include "../../include/comun/comun.h"
#include "../../include/comun/mensaje.h"
#include "../../include/cliente/assets.h"

int assets_cargar_imagenes(SDL_Renderer *renderer, SDL_Texture **imagenes)
{
    int i = 0;
    SDL_Texture **pImagenes = imagenes;
    char *paths[IMAGEN_CANTIDAD] = {
        [IMAGEN_JUGADOR]        = "assets/img/jugador.png",
        [IMAGEN_FANTASMA_01]    = "assets/img/fantasma_01.png",
        [IMAGEN_FANTASMA_02]    = "assets/img/fantasma_02.png",
        [IMAGEN_FANTASMA_03]    = "assets/img/fantasma_03.png",
        [IMAGEN_FANTASMA_04]    = "assets/img/fantasma_04.png",
        [IMAGEN_PREMIO]         = "assets/img/premio.png",
        [IMAGEN_VIDA]           = "assets/img/vida.png",
        [IMAGEN_TSET_CASTILLO]  = "assets/img/tileset_castillo.png",
        [IMAGEN_ICO_VIDAS]      = "assets/img/contador_vidas.png",
        [IMAGEN_ICO_PREMIOS]    = "assets/img/contador_premios.png",
    };
    char **pPaths = paths;

    for (i = 0; i < IMAGEN_CANTIDAD; i++) {

        *pImagenes = IMG_LoadTexture(renderer, *pPaths);
        if (!*pImagenes) {
            mensaje_error("No se pudo cargar la imagen:");
            mensaje_color(TEXTO_ROJO_B, "%s\n%s", *pPaths, IMG_GetError());
            return ERR_ARCH_IMAGEN;
        }

        pImagenes++;
        pPaths++;
    }

    return ERR_TODO_OK;
}

int assets_cargar_sonidos(Mix_Chunk **sonidos)
{
    int i = 0;
    Mix_Chunk **pSonidos = sonidos;
    char *paths[SONIDO_CANTIDAD] = {
        //[SONIDO_FANTASMA_01]      = "assets/snd/fantasma_01.wav",
        //[SONIDO_FANTASMA_02]      = "assets/snd/fantasma_02.wav",
        [SONIDO_FANTASMA_03]      = "assets/snd/fantasma_03.wav",
        [SONIDO_JUGADOR_MOV]  = "assets/snd/jugador_mov.wav",
        //[SONIDO_JUGADOR_MUERTE_01] = "assets/snd/jugador_muerte_01.wav",
        //[SONIDO_JUGADOR_MUERTE_02] = "assets/snd/jugador_muerte_02.wav",
        //[SONIDO_JUGADOR_PREMIO]   = "assets/snd/jugador_premio.wav",
        //[SONIDO_JUGADOR_VIDA]     = "assets/snd/jugador_vida.wav",
        [SONIDO_MENU_CONFIRMAR]   = "assets/snd/menu_confirmar.wav",
        [SONIDO_MENU_MOVIMIENTO]  = "assets/snd/menu_movimiento.wav",
        //[SONIDO_MUSICA]           = "assets/snd/musica.mp3",
        //[SONIDO_PUERTA_SALIDA]    = "assets/snd/puerta_salida.wav",
    };
    char **pPaths = paths;

    for (i = 0; i < SONIDO_CANTIDAD; i++) {

        *pSonidos = Mix_LoadWAV(*pPaths);
        if (!*pSonidos) {
            mensaje_error("No se pudo cargar el sonido:");
            mensaje_color(TEXTO_ROJO_B, "%s\n%s", *pPaths, Mix_GetError());
            return ERR_ARCH_SONIDO;
        }

        pSonidos++;
        pPaths++;
    }

    return ERR_TODO_OK;
}

int assets_cargar_fuente(TTF_Font **fuente, int tamFuente)
{
    *fuente = TTF_OpenFont("assets/fnt/fuente.ttf", tamFuente);
    if (!*fuente) {
        mensaje_error("No se pudo cargar la fuente:");
        mensaje_color(TEXTO_ROJO_B, "%s\n%s", "assets/fnt/fuente.ttf", TTF_GetError());
        return ERR_ARCH_FUENTE;
    }

    return ERR_TODO_OK;
}

void assets_destuir_imagenes(SDL_Texture **imagenes)
{
    SDL_Texture **pImagenes = imagenes, **pImagenesUlt = (imagenes + IMAGEN_CANTIDAD - 1);

    while (pImagenes <= pImagenesUlt) {
        SDL_DestroyTexture(*pImagenes);
        *pImagenes = NULL;
        pImagenes++;
    }
}

void assets_destuir_sonidos(Mix_Chunk **sonidos)
{
    Mix_Chunk **pSonidos = sonidos, **pSonidosUlt = (sonidos + SONIDO_CANTIDAD - 1);

    while (pSonidos <= pSonidosUlt) {
        Mix_FreeChunk(*pSonidos);
        *pSonidos = NULL;
        pSonidos++;
    }
}

void assets_destruir_fuente(TTF_Font *fuente)
{
    TTF_CloseFont(fuente);
    fuente = NULL;
}
