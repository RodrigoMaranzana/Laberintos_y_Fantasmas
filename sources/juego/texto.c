#include "../../include/juego/texto.h"
#include "../../include/comun/mensaje.h"
#include "../../include/comun/comun.h"
#include <stdio.h>

SDL_Texture* texto_crear_textura(SDL_Renderer *renderer, TTF_Font *fuente, const char* texto, SDL_Color color)
{
    SDL_Texture *textura;
    SDL_Surface *superficie;

    superficie = TTF_RenderText_Blended(fuente, texto, color);
    if (!superficie) {
        mensaje_color(TEXTO_ROJO, "[ERROR] %s\n", TTF_GetError());
        return NULL;
    }

    textura = SDL_CreateTextureFromSurface(renderer, superficie);
    if (!textura) {
        mensaje_color(TEXTO_ROJO, "[ERROR] %s\n", SDL_GetError());
        return NULL;
    }

    SDL_SetTextureBlendMode(textura, SDL_BLENDMODE_BLEND);

    SDL_FreeSurface(superficie);
    return textura;
}

int texto_obtener_tam(TTF_Font *fuente, const char *texto, SDL_Point *tamTexto){

    int ret = ERR_TODO_OK;

    if (TTF_SizeText(fuente, texto, &tamTexto->x, &tamTexto->y) == -1) {
        ret = ERR_FUENTE;
    }

    return ret;
}
