#include <stdio.h>
#include "../../include/cliente/graficos.h"

SDL_Texture* graficos_crear_textura(SDL_Renderer *renderer, int ancho, int alto, int modoAcceso, unsigned formato)
{
    SDL_Texture* textura = SDL_CreateTexture(renderer, formato, modoAcceso, ancho, alto);
    if (!textura) {

        printf("Error: %s", SDL_GetError());
        return NULL;
    }

    SDL_SetTextureBlendMode(textura, SDL_BLENDMODE_BLEND);

    return textura;
}

void graficos_dibujar_textura(SDL_Texture *textura, SDL_Renderer *renderer, SDL_Rect *rectFuente, SDL_Rect *rectDestino, tEfectoGrafico efecto)
{
    if (efecto) {
        efecto(textura);
    }

    if (SDL_RenderCopy(renderer, textura, rectFuente, rectDestino) != 0) {

        printf("Error: %s", SDL_GetError());
    }

    if (efecto) {
        SDL_SetTextureAlphaMod(textura, 255);
        SDL_SetTextureColorMod(textura, 255, 255, 255);
    }
}

void graficos_dibujar_borde(SDL_Renderer *renderer, SDL_Rect rect, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(renderer, &rect);
}

void graficos_dibujar_relleno(SDL_Renderer *renderer, SDL_Rect rect, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}
