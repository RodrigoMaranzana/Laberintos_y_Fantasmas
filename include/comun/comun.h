#ifndef COMUN_H_INCLUDED
#define COMUN_H_INCLUDED

///MACRO FUNCIONES
#define ES_DIGITO(x)((x) >= '0' && (x) <= '9')
#define ES_LETRA(x)(((x) >= 'a' && (x) <= 'z') || ((x) >= 'A' && (x) <= 'Z'))
#define MIN(a,b)( (a) < (b) ? (a) : (b) )
#define MAX(a,b)( (a) > (b) ? (a) : (b) )

///MACROS
// General
#define PIXELES_TILE 48
#define MARGEN_VENTANA 128
#define PADDING_MARGEN 128
//Servidor
#define IP_SERVIDOR "127.0.0.1"
#define PUERTO 12345
#define TAM_BUFFER 1024
#define TAM_COMANDO_MAX 17
#define TAM_USUARIO 16

#define COLOR_ROJO         "\x1b[31m"
#define COLOR_VERDE        "\x1b[32m"
#define COLOR_AMARILLO     "\x1b[33m"
#define COLOR_AZUL         "\x1b[34m"
#define COLOR_MAGENTA      "\x1b[35m"
#define COLOR_CIAN         "\x1b[36m"
#define COLOR_BLANCO       "\x1b[37m"
#define COLOR_RESET        "\x1b[0m"
#define FONDO_NEGRO        "\x1b[40m"
#define FONDO_ROJO         "\x1b[30m\x1b[41m"
#define FONDO_VERDE        "\x1b[30m\x1b[42m"
#define FONDO_AMARILLO     "\x1b[30m\x1b[43m"
#define FONDO_AZUL         "\x1b[30m\x1b[44m"
#define FONDO_MAGENTA      "\x1b[30m\x1b[45m"
#define FONDO_CIAN         "\x1b[30m\x1b[46m"
#define FONDO_BLANCO       "\x1b[30m\x1b[47m"

typedef enum {
    ERR_TODO_OK,
    ERR_SIN_MEMORIA,
    ERR_SDL_INI,
    ERR_VENTANA,
    ERR_RENDERER,
    ERR_TEXTURA,
    ERR_MENU,
    ERR_ARCHIVO,
    ERR_CONF,
    ERR_FUENTE,
    ERR_ARCH_FUENTE,
    ERR_ARCH_IMAGEN,
    ERR_ARCH_SONIDO,
} eRetorno;


#endif // COMUN_H_INCLUDED
