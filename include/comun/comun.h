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
#define TAM_USUARIO 17
//Solicitudes
#define SOLICITUD_INI_SESION        "INICIAR_SESION"
#define SOLICITUD_INS_PARTIDA       "INSERTAR_PARTIDA"
#define SOLICITUD_OBT_PARTIDA       "OBTENER_PARTIDAS"
//Respuestas
#define RESPUESTA_OK                "OK"
#define RESPUESTA_ERROR             "ERROR"
#define RESPUESTA_SOLIC_INVALIDA    "SOLICITUD_INVALIDA"
#define RESPUESTA_PARTIDA           "PARTIDA"
#define RESPUESTA_FIN               "FIN"
/// Ejemplos
//INICIAR_SESION|MANUEL\n
//INSERTAR_PARTIDA|11|2|67\n
//PARTIDA|11|2|67\n
//FIN\n

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
    ERR_FUENTE,
    ERR_ARCH_FUENTE,
    ERR_ARCH_IMAGEN,
    ERR_ARCH_SONIDO,
} eRetorno;


#endif // COMUN_H_INCLUDED
