#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED

#include <SDL.h>

#define MENU_NADA_SELEC -1

#define MENU_ERR_TODO_OK 0
#define MENU_SIN_MEM 1

typedef enum {
    OPCION_DESHABILITADA,
    OPCION_HABILITADA,
    OPCION_OCULTA,
} eOpcionEstado;

typedef enum {
    MENU_VERTICAL,
    MENU_HORIZONTAL
} eMenuTipo;

typedef void (*tMenuFuncion)(void *datos);

typedef struct {
    tMenuFuncion funcion;
    void* datos;
} tMenuAccion;

typedef struct sMenuOpcion {
    int id;
    SDL_Texture *textura;
    tMenuAccion accion;
    SDL_Point tamTextura;
    eOpcionEstado estado;
} tMenuOpcion;

typedef struct {
    SDL_Renderer *renderer;
    tMenuOpcion *opciones;
    unsigned cantOpc;
    unsigned capOpc;
    int selecOpc;
    SDL_Point ubicacion;
    eMenuTipo menuTipo;
} tMenu;

tMenu* menu_crear(SDL_Renderer *renderer, unsigned capOpc, SDL_Point ubicacion, eMenuTipo menuTipo);
void menu_destruir(tMenu* menu);
int menu_agregar_opcion(tMenu *menu, int id, SDL_Texture *textura, unsigned tamAltura, tMenuAccion accion, eOpcionEstado estado);
void menu_siguiente_opcion(tMenu *menu);
void menu_anterior_opcion(tMenu *menu);
tMenuAccion menu_confirmar_opcion(tMenu *menu);
void menu_estado_opcion(tMenu *menu, int id, eOpcionEstado nuevoEstado);
void menu_dibujar(tMenu* menu);

#endif // MENU_H_INCLUDED
