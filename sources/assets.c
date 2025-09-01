#include <stdio.h>
#include "../include/retorno.h"
#include "../include/assets.h"

int assets_cargar_imagenes(SDL_Renderer *renderer, SDL_Texture **imagenes)
{
    int i = 0;
    SDL_Texture **pImagenes = imagenes;
    char *paths[IMAGEN_CANTIDAD] = {
        [IMAGEN_FANTASMA]       = "assets/img/fantasma.png",
        [IMAGEN_JUGADOR]        = "assets/img/jugador.png",
        [IMAGEN_PARED_LAD_GRIS] = "assets/img/pared_tileset_ladrillo_gris.png",
        [IMAGEN_PISO]           = "assets/img/piso.png",
        [IMAGEN_PREMIO]         = "assets/img/test.png",
        [IMAGEN_PUERTA_ENTRADA] = "assets/img/test.png",
        [IMAGEN_PUERTA_SALIDA]  = "assets/img/test.png",
        [IMAGEN_VIDA]           = "assets/img/test.png",
    };
    char **pPaths = paths;

    for(i = 0; i < IMAGEN_CANTIDAD; i++){

        *pImagenes = IMG_LoadTexture(renderer, *pPaths);
        if(!*pImagenes){

            printf("Error cargando imagen %s: %s\n", *pPaths, IMG_GetError());
            return ERR_ARCH_IMAGEN;
        }

        pImagenes++;
        pPaths++;
    }

    return TODO_OK;
}

int assets_cargar_sonidos(Mix_Chunk **sonidos)
{
    int i = 0;
    Mix_Chunk **pSonidos = sonidos;
    char *paths[SONIDO_CANTIDAD] = {
        [SONIDO_FANTASMA_01]      = "assets/snd/fantasma_01.wav",
        [SONIDO_FANTASMA_02]      = "assets/snd/fantasma_02.wav",
        [SONIDO_FANTASMA_03]      = "assets/snd/fantasma_03.wav",
        [SONIDO_JUGADOR_CAMINAR]  = "assets/snd/jugador_caminar.mp3",
        [SONIDO_JUGADOR_MUERTE_01]= "assets/snd/jugador_muerte_01.wav",
        [SONIDO_JUGADOR_MUERTE_02]= "assets/snd/jugador_muerte_02.wav",
        [SONIDO_JUGADOR_PREMIO]   = "assets/snd/jugador_premio.wav",
        [SONIDO_JUGADOR_VIDA]     = "assets/snd/jugador_vida.wav",
        [SONIDO_MUSICA]           = "assets/snd/musica.mp3",
        [SONIDO_PUERTA_SALIDA]    = "assets/snd/puerta_salida.wav",
    };
    char **pPaths = paths;

    for(i = 0; i < SONIDO_CANTIDAD; i++){

        *pSonidos = Mix_LoadWAV(*pPaths);
        if(!*pSonidos){

            printf("Error cargando sonido %s: %s\n", *pPaths, Mix_GetError());
            return ERR_ARCH_SONIDO;
        }

        pSonidos++;
        pPaths++;
    }

    return TODO_OK;
}

int assets_cargar_fuente(TTF_Font **fuente, int tamFuente)
{
    *fuente = TTF_OpenFont("assets/fnt/fuente.ttf", tamFuente);
    if(!*fuente){

        printf("ERROR: %s\n", TTF_GetError());
        return ERR_ARCH_FUENTE;
    }

    return TODO_OK;
}


void assets_destuir_imagenes(SDL_Texture **imagenes)
{
    SDL_Texture **pImagenes = imagenes, **pImagenesUlt = (imagenes + IMAGEN_CANTIDAD - 1);

    while(pImagenes <= pImagenesUlt){

        SDL_DestroyTexture(*pImagenes);
        *pImagenes = NULL;
        pImagenes++;
    }
}

void assets_destuir_sonidos(Mix_Chunk **sonidos)
{
    Mix_Chunk **pSonidos = sonidos, **pSonidosUlt = (sonidos + SONIDO_CANTIDAD - 1);

    while(pSonidos <= pSonidosUlt){

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

