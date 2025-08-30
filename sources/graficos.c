#include <stdio.h>
#include "..\include\graficos.h"

SDL_Texture* graficos_crear_textura(SDL_Renderer *renderer, int ancho, int alto, int modoAcceso, unsigned formato)
{
    SDL_Texture* textura = SDL_CreateTexture(renderer, formato, modoAcceso, ancho, alto);
    if(!textura){

        printf("Error: %s", SDL_GetError());
        return NULL;
    }

    SDL_SetTextureBlendMode(textura, SDL_BLENDMODE_BLEND);

    return textura;
}

void graficos_dibujar_textura(SDL_Texture *textura, SDL_Renderer *renderer, SDL_Rect *rectFuente, SDL_Rect *rectDestino)
{
    if(SDL_RenderCopy(renderer, textura, rectFuente, rectDestino) != 0){

        printf("Error: %s", SDL_GetError());
    }
}
