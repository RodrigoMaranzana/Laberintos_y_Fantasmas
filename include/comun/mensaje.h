#ifndef MENSAJE_H_INCLUDED
#define MENSAJE_H_INCLUDED
#include <stdarg.h>

#define TEXTO_ROJO           "\x1b[31m"
#define TEXTO_ROJO_B         "\x1b[1;31m"
#define TEXTO_VERDE          "\x1b[32m"
#define TEXTO_AMARILLO       "\x1b[33m"
#define TEXTO_AMARILLO_B     "\x1b[1;33m"
#define TEXTO_AZUL           "\x1b[34m"
#define TEXTO_MAGENTA        "\x1b[35m"
#define TEXTO_MAGENTA_B      "\x1b[1;35m"
#define TEXTO_CIAN           "\x1b[36m"
#define TEXTO_BLANCO         "\x1b[37m"
#define TEXTO_RESET          "\x1b[0m"
#define FONDO_NEGRO          "\x1b[40m"
#define FONDO_ROJO           "\x1b[30m\x1b[41m"
#define FONDO_VERDE          "\x1b[30m\x1b[42m"
#define FONDO_AMARILLO       "\x1b[30m\x1b[43m"
#define FONDO_CIAN           "\x1b[30m\x1b[46m"
#define FONDO_BLANCO         "\x1b[30m\x1b[47m"
#define FONDO_AZUL           "\x1b[30m\x1b[44m"
#define FONDO_MAGENTA        "\x1b[30m\x1b[45m"
#define PARPADEO             "\x1b[5m"
#define COLOR_RESET          "\x1b[0m"

void mensaje_color(const char *color, const char *formato, ...);
void mensaje_info(const char *mensaje);
void mensaje_todo_ok(const char *mensaje);
void mensaje_advertencia(const char *mensaje);
void mensaje_error(const char *mensaje);
void mensaje_debug(const char *mensaje);
void mensaje_titulo(const char *mensaje);

#endif // MENSAJE_H_INCLUDED
