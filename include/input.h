#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#include <SDL2/SDL_keycode.h>

typedef enum{
    ACCION_NINGUNA,
    ACCION_SALIR,
    ACCION_CONFIRMAR,
    ACCION_CANCELAR,
    ACCION_ARRIBA,
    ACCION_ABAJO,
    ACCION_IZQUIERDA,
    ACCION_DERECHA,
    ACCION_CANTIDAD
}eAccion;

typedef struct{
    SDL_Keycode tecla;
    int accion;
}tKeyMap;

eAccion input_procesar_tecla(SDL_Keycode sdlKeycode, const tKeyMap* mapaTeclas, int mapaCant);

#endif // INPUT_H_INCLUDED
