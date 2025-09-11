#ifndef COMUN_H_INCLUDED
#define COMUN_H_INCLUDED

//MACRO FUNCIONES
#define ES_DIGITO(x)((x) >= '0' && (x) <= '9')
#define ES_LETRA(x)(((x) >= 'a' && (x) <= 'z') || ((x) >= 'A' && (x) <= 'Z'))

//MACROS
#define PIXELES_TILE 48
#define MARGEN_VENTANA 64

#define IP_SERVIDOR "127.0.0.1"
#define PUERTO 12345
#define TAM_BUFFER 1024

typedef enum {
    TODO_OK,
    ERR_SIN_MEMORIA,
    ERR_SDL_INI,
    ERR_VENTANA,
    ERR_RENDERER,
    ERR_TEXTURA,
    ERR_MENU,
    ERR_ARCHIVO,
    ERR_CONF,
    ERR_ARCH_FUENTE,
    ERR_ARCH_IMAGEN,
    ERR_ARCH_SONIDO,
} eRetorno;


#endif // COMUN_H_INCLUDED
