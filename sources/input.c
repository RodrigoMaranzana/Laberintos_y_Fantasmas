#include "../include/input.h"

eAccion input_procesar_tecla(SDL_Keycode sdlKeycode, const tKeyMap* mapaTeclas, int mapaCant)
{
    int i = 0;
    eAccion accion = -1;

    while(i < mapaCant && accion == -1){

        if(mapaTeclas[i].tecla == sdlKeycode){

            accion = (eAccion)mapaTeclas[i].accion;
        }

        i++;
    }

    return accion;
}
