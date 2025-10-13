#include <stdio.h>
#include <string.h>
#include "../../include/comun/mensaje.h"

void mensaje_info(const char *mensaje)
{
    fprintf(stderr, TEXTO_CIAN "[INFO] " COLOR_RESET "%s\n", mensaje);
}

void mensaje_todo_ok(const char *mensaje)
{
    fprintf(stderr, TEXTO_VERDE "[OK] " COLOR_RESET "%s\n", mensaje);
}

void mensaje_advertencia(const char *mensaje)
{
    fprintf(stderr, TEXTO_AMARILLO "[ADVERTENCIA] " TEXTO_AMARILLO_B "%s" COLOR_RESET "\n", mensaje);
}

void mensaje_error(const char *mensaje)
{
    fprintf(stderr, PARPADEO TEXTO_ROJO "[ERROR] " COLOR_RESET TEXTO_ROJO_B "%s" COLOR_RESET "\n", mensaje);
}

void mensaje_debug(const char *mensaje)
{
    fprintf(stderr, TEXTO_MAGENTA "[DEBUG] " TEXTO_MAGENTA_B "%s" COLOR_RESET "\n", mensaje);
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
