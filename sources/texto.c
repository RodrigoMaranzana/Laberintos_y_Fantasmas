#include "../include/texto.h"
#include <stdio.h>

SDL_Texture* texto_crear_textura(SDL_Renderer *renderer, TTF_Font *fuente, const char* texto, SDL_Color color)
{
    SDL_Texture *textura;
    SDL_Surface *superficie;

    superficie = TTF_RenderText_Blended(fuente, texto, color);
    if (!superficie) {
        printf("ERROR: %s\n", TTF_GetError());
        return NULL;
    }

    textura = SDL_CreateTextureFromSurface(renderer, superficie);
    if (!textura) {

        printf("ERROR: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_SetTextureBlendMode(textura, SDL_BLENDMODE_BLEND);

    SDL_FreeSurface(superficie);
    return textura;
}
