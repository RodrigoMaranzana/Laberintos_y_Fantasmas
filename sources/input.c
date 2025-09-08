#include "../include/input.h"

int input_tecla_valida(SDL_Keycode sdlKeycode)
{
    switch (sdlKeycode) {

        case SDLK_ESCAPE:
        case SDLK_RETURN:
        case SDLK_BACKSPACE:
        case SDLK_UP:
        case SDLK_DOWN:
        case SDLK_LEFT:
        case SDLK_RIGHT:
            return 1;
        default:
            break;
    }

    return 0;
}
