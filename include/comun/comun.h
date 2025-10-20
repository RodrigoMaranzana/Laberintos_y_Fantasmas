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
#define PUERTO_TERMINAL 23456
#define TAM_BUFFER 1024
#define TAM_COMANDO_MAX 17
#define TAM_USUARIO 16
#define TAM_DIRECTORIO 256
#define TAM_RUTA 256

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
    ERR_LINEA_LARGA,
    ERR_EN_ESPERA,
    ERR_OFFLINE,
    ERR_LOGICA,
} eRetorno;

int comun_crear_directorio(const char *directorio);



#endif // COMUN_H_INCLUDED
