#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED

#include <SDL2/SDL.h>

#define MENU_NADA_SELEC -1


typedef void (*tMenuFuncion)(void *datos);

typedef enum{
    MENU_NINGUNO,
    MENU_SIGUIENTE,
    MENU_ANTERIOR,
    MENU_CONFIRMAR,
}eMenuComando;

typedef struct {
    tMenuFuncion funcion;
    void* datos;
}tMenuAccion;

typedef struct{
    SDL_Texture *textura;
    tMenuAccion accion;
    SDL_Point tamTextura;
}tMenuOpcion;

typedef struct{
    tMenuOpcion* opciones;
    unsigned cantOpc;
    unsigned capOpc;
    int selecOpc;
    SDL_Point ubicacion;
}tMenu;

tMenu* menu_crear(unsigned capOpc, SDL_Point ubicacion);
void menu_destruir(tMenu* menu);
int menu_agregar_opcion(tMenu *menu, SDL_Texture *textura, unsigned tamAltura, tMenuAccion accion);
tMenuAccion menu_procesar_comando(tMenu* menu, eMenuComando comando);
void menu_dibujar(SDL_Renderer *renderer, tMenu* menu);

#endif // MENU_H_INCLUDED
