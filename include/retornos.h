#ifndef ERR_H
#define ERR_H

typedef enum{
    // General
    TODO_OK,
    ERR_SIN_MEMORIA,

    // SDL
    ERR_SDL_INI,
    ERR_VENTANA,
    ERR_RENDERER,
    ERR_TEXTURA,

    // Archivos
    ERR_ARCHIVO
}eRetornos;

#endif //ERR_H
