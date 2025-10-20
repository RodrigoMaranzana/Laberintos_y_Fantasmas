#include <stdio.h>
#include "../../include/juego/graficos.h"
#include "../../include/comun/mensaje.h"

SDL_Texture* graficos_crear_textura(SDL_Renderer *renderer, int ancho, int alto, int modoAcceso, unsigned formato)
{
    SDL_Texture* textura = SDL_CreateTexture(renderer, formato, modoAcceso, ancho, alto);
    if (!textura) {

        mensaje_color(TEXTO_ROJO, "[ERROR] %s", SDL_GetError());
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
        mensaje_color(TEXTO_ROJO, "[ERROR] %s", SDL_GetError());
    }

    if (efecto) {
        SDL_SetTextureAlphaMod(textura, 255);
        SDL_SetTextureColorMod(textura, 255, 255, 255);
        SDL_SetTextureBlendMode(textura, SDL_BLENDMODE_BLEND);
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

void graficos_dibujar_mosaico(SDL_Renderer *renderer, SDL_Texture *textura, SDL_Rect *rectFuente, float escala)
{
    int inicioX, inicioY, tamRendererX, tamRendererY;
    SDL_Rect rectDestino;
    int offset = (int)(SDL_GetTicks() * 0.02f);

    SDL_GetRendererOutputSize(renderer, &tamRendererX, &tamRendererY);

    rectDestino.w = (int)(rectFuente->w * escala);
    rectDestino.h = (int)(rectFuente->h * escala);

    inicioX = - (offset % rectDestino.w);
    inicioY = - (offset % rectDestino.h);

    for (int y = inicioY; y < tamRendererY; y += rectDestino.h) {

        for (int x = inicioX; x < tamRendererX; x += rectDestino.w) {

            rectDestino.x = x;
            rectDestino.y = y;
            SDL_RenderCopy(renderer, textura, rectFuente, &rectDestino);
        }
    }
}
