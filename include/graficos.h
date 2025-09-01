#ifndef GRAFICOS_H_INCLUDED
#define GRAFICOS_H_INCLUDED

#include <SDL2/SDL.h>

typedef struct{
    SDL_Texture *textura;
    SDL_Rect rect;
}tTextura;

SDL_Texture* graficos_crear_textura(SDL_Renderer *renderer, int ancho, int alto, int modoAcceso, unsigned formato);
void graficos_dibujar_textura(SDL_Texture *textura, SDL_Renderer *renderer, SDL_Rect *rectFuente, SDL_Rect *rectDestino);

#endif // GRAFICOS_H_INCLUDED
