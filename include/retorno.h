#ifndef ERR_H
#define ERR_H

typedef enum {
    // General
    TODO_OK,
    ERR_SIN_MEMORIA,

    // SDL
    ERR_SDL_INI,
    ERR_VENTANA,
    ERR_RENDERER,
    ERR_TEXTURA,

    // Menu
    ERR_MENU,

    // Archivos
    ERR_ARCHIVO,

    // Assets
    ERR_ARCH_FUENTE,
    ERR_ARCH_IMAGEN,
    ERR_ARCH_SONIDO,

    // Pila
    ERR_PILA_VACIA,
    ERR_PILA_LLENA
} eRetorno;

#endif //ERR_H
