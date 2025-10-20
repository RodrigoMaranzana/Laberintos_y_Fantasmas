#include <stdio.h>

#include <string.h>
#include "../../include/comun/mensaje.h"

void mensaje_color(const char *color, const char *formato, ...)
{
    va_list argumentos;

    #ifdef MODO_DEBUG
        fprintf(stderr, "%s", color);
    #endif

    va_start(argumentos, formato);
    vfprintf(stderr, formato, argumentos);
    va_end(argumentos);

    #ifdef MODO_DEBUG
        fprintf(stderr, COLOR_RESET "\n");
    #else
        fprintf(stderr,"\n");
    #endif
}

void mensaje_info(const char *mensaje)
{
    #ifdef MODO_DEBUG
        fprintf(stdout, TEXTO_CIAN "[INFO] " COLOR_RESET "%s\n", mensaje);
    #else
        fprintf(stdout,"[INFO] %s\n", mensaje);
    #endif
}

void mensaje_todo_ok(const char *mensaje)
{
    #ifdef MODO_DEBUG
        fprintf(stderr, TEXTO_VERDE "[OK] " COLOR_RESET "%s\n", mensaje);
    #else
        fprintf(stderr,"[OK] %s\n", mensaje);
    #endif
}

void mensaje_advertencia(const char *mensaje)
{
    #ifdef MODO_DEBUG
        fprintf(stderr, TEXTO_AMARILLO "[ADVERTENCIA] " TEXTO_AMARILLO_B "%s" COLOR_RESET "\n", mensaje);
    #else
        fprintf(stderr,"[ADVERTENCIA] %s\n", mensaje);
    #endif
}

void mensaje_error(const char *mensaje)
{
    #ifdef MODO_DEBUG
        fprintf(stderr, PARPADEO TEXTO_ROJO "[ERROR] " COLOR_RESET TEXTO_ROJO_B "%s" COLOR_RESET "\n", mensaje);
    #else
        fprintf(stderr,"[ERROR] %s\n", mensaje);
    #endif
}

void mensaje_debug(const char *mensaje)
{
    #ifdef MODO_DEBUG
        fprintf(stderr, TEXTO_MAGENTA "[DEBUG] " TEXTO_MAGENTA_B "%s" COLOR_RESET "\n", mensaje);
    #else
        fprintf(stderr,"[DEBUG] %s\n", mensaje);
    #endif
}

void mensaje_titulo(const char *mensaje)
{
    printf(FONDO_BLANCO);
    for(int i = 0; i < strlen(mensaje) + 4; i++) {
        putchar('=');
    }
    putchar('\n');

    printf("  %s  \n", mensaje);
    for(int i = 0; i < strlen(mensaje) + 4; i++) {
        putchar('=');
    }

    printf("\n" COLOR_RESET);
}

void mensaje_subtitulo(const char *mensaje)
{
    printf(FONDO_BLANCO);
    printf("-- %s --\n", mensaje);
    printf(COLOR_RESET);
}
