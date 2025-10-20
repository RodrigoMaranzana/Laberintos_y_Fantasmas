#ifndef TEXTO_H_INCLUDED
#define TEXTO_H_INCLUDED

#include <SDL.h>
#include <SDL_ttf.h>

SDL_Texture* texto_crear_textura(SDL_Renderer *renderer, TTF_Font *fuente, const char* texto, SDL_Color color);
int texto_obtener_tam(TTF_Font *fuente, const char *texto, SDL_Point *tamTexto);
#endif // TEXTO_H_INCLUDED
