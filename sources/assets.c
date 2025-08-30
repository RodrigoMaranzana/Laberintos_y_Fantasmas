#include <stdio.h>
#include "../include/retornos.h"
#include "../include/assets.h"




int assets_cargar_imagenes(SDL_Renderer *renderer, SDL_Texture **imgAssets)
{
    int i = 0;
    SDL_Texture **pImgAssets = imgAssets;
    char *pathImagenes[] = {
        "assets/img/fantasma.png",
        "assets/img/jugador.png",
    };
    char **pPathImagenes = pathImagenes;

    for(i = 0; i < IMAGEN_CANTIDAD; i++){

        *pImgAssets = IMG_LoadTexture(renderer, *pPathImagenes);
        if(!*pImgAssets){

            printf("Error cargando imagen %s: %s\n", *pPathImagenes, IMG_GetError());
            return ERR_ARCHIVO;
        }

        pImgAssets++;
        pPathImagenes++;
    }

    return TODO_OK;
}

int assets_cargar_sonidos(Mix_Chunk **sndAssets)
{
    int i = 0;
    Mix_Chunk **pSndAssets = sndAssets;
    char *pathSonidos[] = {
        "assets/snd/fantasma_01.wav",
        "assets/snd/fantasma_02.wav",
        "assets/snd/fantasma_03.wav",
        "assets/snd/jugador_caminar.mp3",
        "assets/snd/jugador_muerte_01.wav",
        "assets/snd/jugador_muerte_02.wav",
        "assets/snd/jugador_premio.wav",
        "assets/snd/jugador_vida.wav",
        "assets/snd/musica.mp3",
        "assets/snd/puerta_salida.wav",
    };
    char **pPathSonidos = pathSonidos;

    for(i = 0; i < SONIDO_CANTIDAD; i++){

        *pSndAssets = Mix_LoadWAV(*pPathSonidos);
        if(!*pSndAssets){

            printf("Error cargando sonido %s: %s\n", *pPathSonidos, Mix_GetError());
            return ERR_ARCHIVO;
        }

        pSndAssets++;
        pPathSonidos++;
    }

    return TODO_OK;
}
