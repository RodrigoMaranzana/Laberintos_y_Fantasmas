#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/comun/comun.h"
#include "../../include/comun/cola.h"
#include "../../include/comun/cliente.h"
#include "../../include/comun/mensaje.h"

#define ES_BLANCO(c) ((c) == ' ' || (c) == '\t' || (c) == '\r' || (c) == '\v' || (c) == '\f' || (c) == '\n')

typedef enum {
    AYUDA,
    BDATOS,
    SALIR,
    COMANDOS_CANT,
    DESCONOCIDO2
} eComando;

typedef enum {
    TERMINAL_TODO_OK,
    TERMINAL_ERROR_SINTAXIS,
    TERMINAL_FIN_SECUENCIA
} eErrTerminal;

typedef enum {
    MODO_TERMINAL,
    MODO_BDATOS
} eModoTerminal;

typedef enum {
    // Comandos
    CREAR, INSERTAR, ACTUALIZAR, SELECCIONAR,
    // Operadores
    IGUAL, MAYOR, MENOR, DISTINTO, TOP,
    // Tipos
    ENTERO, TEXTO,
    // Conector
    DONDE,
    // Restricciones
    PK, AI, IS,
    // Control
    SIMBOLOS_CANT, DESCONOCIDO,
} eSimbolo;

typedef struct {
    const char* buffer;
    const char* color;
} tColorSimbolo;

typedef struct {
    const char *cursor;
    int finSec;
} tSecuencia;

typedef int (*tParsear)(tSecuencia *secuencia, void *salida, unsigned tamSalida);

static int _terminal_parsear_texto(tSecuencia *secuencia, void *salida, unsigned tamSalida);
static int _terminal_parsear(tSecuencia *secuencia, tParsear parsear, void *salida, unsigned tamSalida);
void terminal_actualizar_llave(const char* linea, int *llave);
void terminal_imprimir_sintaxis(const char* linea);
static void _terminal_crear_secuencia(tSecuencia *secuencia, const char *buffer);
static const char* _terminal_simbolo_color(const char* simbolo);
static int _terminal_cmp_color_simbolo(const void* a, const void* b);

int main()
{
    int corriendo = 1, llave = 0;
    SOCKET sock;
    char conectado = SESION_OFFLINE, bufferLinea[1024], *cursor ;
    eModoTerminal modo = MODO_TERMINAL;
    tCola colaRespuestas, colaLineas;

    mensaje_titulo("LyfDB - Terminal del Servicio");

    if (cliente_inicializar() != 0) {
        mensaje_error("No se pudo inicializar Winsock.");
        return 1;
    }

    cola_crear(&colaRespuestas);
    cola_crear(&colaLineas);

    while (corriendo) {

        if (modo == MODO_BDATOS && llave > 0) {
            printf(TEXTO_AMARILLO "... " TEXTO_RESET);
        } else if (modo == MODO_BDATOS) {
            printf(TEXTO_VERDE "%s%s%s", "LyfDB", TEXTO_RESET, "> ");
        } else {
            printf(TEXTO_VERDE "%s%s%s", "terminal", TEXTO_RESET, "> ");
        }

        fgets(bufferLinea, sizeof(bufferLinea), stdin);
        cursor = strchr(bufferLinea, '\n');
        if (!cursor) {
            return -1;
        }
        *cursor = '\0';

        if (modo == MODO_BDATOS) {

            strncat(bufferLinea, "\n", 2);

            cola_encolar(&colaLineas, bufferLinea, strlen(bufferLinea) + 1);
            printf("\x1b[1A\r\x1b[2K");

            if (llave == 0) {
                printf(TEXTO_VERDE "LyfDB" TEXTO_RESET "> ");
            } else {
                printf(TEXTO_AMARILLO "... " TEXTO_RESET);
            }

            terminal_imprimir_sintaxis(bufferLinea);

            terminal_actualizar_llave(bufferLinea, &llave);

            if (cola_vacia(&colaLineas) != COLA_VACIA && llave == 0) {

                while (cola_desencolar(&colaLineas, bufferLinea, sizeof(bufferLinea)) != COLA_VACIA) {

                    cursor = strchr(bufferLinea, '{');
                    if (cursor) {
                        strcpy(cursor, cursor+1);
                    }
                    cursor = strchr(bufferLinea, '}');
                    if (cursor) {
                        *cursor = '\0';
                    }
                    cursor = strrchr(bufferLinea, '\n');
                    if (cursor) {
                        *cursor = '\0';
                    }

                    if (strlen(bufferLinea) > 0) {

                        if (cliente_enviar_solicitud(sock, bufferLinea) == CE_ERR_SOCKET) {
                            conectado = SESION_OFFLINE;
                            mensaje_advertencia("Socket desconectado.");
                            modo = MODO_TERMINAL;
                        } else {

                            int retorno = cliente_recibir_respuesta(sock, &colaRespuestas);
                            if (retorno == CE_TODO_OK || retorno == CE_DATOS) {

                                char buffer[TAM_BUFFER];
                                puts("");
                                if (cola_desencolar(&colaRespuestas, buffer, sizeof(buffer)) != COLA_VACIA) {
                                    printf(TEXTO_CIAN "[RESPUESTA]\n%s" ,buffer);
                                    *buffer = '\0';
                                }

                                if (retorno == CE_DATOS) {

                                    int i = 1;
                                    while (cola_desencolar(&colaRespuestas, buffer, sizeof(buffer)) != COLA_VACIA) {
                                        printf(TEXTO_MAGENTA "  %d. " TEXTO_AMARILLO "%s", i++, buffer);
                                    }
                                }
                                puts("");
                            }
                        }
                    }
                }

                cola_vaciar(&colaLineas);
                cola_vaciar(&colaRespuestas);
            }

        } else {

            if (strcmp(bufferLinea, "BDATOS") == 0) {

                if (conectado == SESION_OFFLINE) {

                    sock = cliente_conectar_servidor(IP_SERVIDOR, PUERTO_TERMINAL);
                    if (sock == INVALID_SOCKET) {
                        mensaje_error("No hay comunicacion con el servidor.");
                        conectado = SESION_OFFLINE;
                    }else{
                        mensaje_todo_ok("Conectado al servidor.");
                        conectado = SESION_ONLINE;
                        modo = MODO_BDATOS;
                    }
                }
            } else if (strcmp(bufferLinea, "SALIR") == 0) {

                corriendo = 0;
            }
        }
    }

    cliente_destruir_conexion(sock);
    WSACleanup();
    return 0;
}

void terminal_imprimir_sintaxis(const char* linea)
{
    char buffer[TAM_BUFFER];
    const char *cursor = linea;
    tSecuencia secuencia;

    _terminal_crear_secuencia(&secuencia, linea);

    while ((_terminal_parsear(&secuencia, _terminal_parsear_texto, buffer, sizeof(buffer))) == TERMINAL_TODO_OK) {

        const char* cursorActual = secuencia.cursor - strlen(buffer);
        for (const char* p = cursor; p < cursorActual; ++p) {
            putchar(*p);
        }

        printf("%s%s", _terminal_simbolo_color(buffer), buffer);

        cursor = secuencia.cursor;
    }

    if (*cursor) {
        printf("%s", cursor);
    }
}

static const char* _terminal_simbolo_color(const char* simbolo)
{
    const tColorSimbolo colores[] = {
        {"ACTUALIZAR", TEXTO_AMARILLO},
        {"AI",         TEXTO_MAGENTA_B},
        {"CREAR",      TEXTO_AMARILLO},
        {"DISTINTO",   TEXTO_CIAN},
        {"DONDE",      TEXTO_MAGENTA},
        {"ENTERO",     TEXTO_MAGENTA},
        {"IGUAL",      TEXTO_CIAN},
        {"INSERTAR",   TEXTO_AMARILLO},
        {"IS",         TEXTO_MAGENTA_B},
        {"MAYOR",      TEXTO_CIAN},
        {"MENOR",      TEXTO_CIAN},
        {"PK",         TEXTO_MAGENTA_B},
        {"SELECCIONAR",TEXTO_AMARILLO},
        {"TEXTO",      TEXTO_MAGENTA},
        {"TOP",        TEXTO_CIAN}
    };

    tColorSimbolo *color = bsearch(simbolo, colores, sizeof(colores) / sizeof(*colores), sizeof(*colores), _terminal_cmp_color_simbolo);

    if (color != NULL) {
        return color->color;
    } else {
        return TEXTO_RESET;
    }
}

static int _terminal_cmp_color_simbolo(const void* a, const void* b) {

    const char *buffer = (const char*)a;
    const tColorSimbolo *color = (const tColorSimbolo*)b;

    return strcmp(buffer, color->buffer);
}


void terminal_actualizar_llave(const char* linea, int *llave) {

    for (int i = 0; linea[i] != '\0'; ++i) {

        switch (linea[i]) {
            case '{': ++(*llave);
                break;
            case '}': --(*llave);
                break;
        }
    }
}

static void _terminal_crear_secuencia(tSecuencia *secuencia, const char *buffer)
{
    secuencia->cursor = buffer;
    secuencia->finSec = 0;
}

static int _terminal_parsear(tSecuencia *secuencia, tParsear parsear, void *salida, unsigned tamSalida)
{
    while (*secuencia->cursor && ES_BLANCO(*secuencia->cursor)) {
        secuencia->cursor++;
    }

    if (!*secuencia->cursor || secuencia->finSec) {
       return TERMINAL_FIN_SECUENCIA;
    }

    return parsear(secuencia, salida, tamSalida);
}

static int _terminal_parsear_texto(tSecuencia *secuencia, void *salida, unsigned tamSalida)
{
    char *texto = (char*)salida;
    char *cursor = texto;
    const char* delimitadores = " \t\n\r(),";

    if (*secuencia->cursor == '(' || *secuencia->cursor == ')' || *secuencia->cursor == ',') {
        if (tamSalida > 1) {
            *cursor++ = *secuencia->cursor++;
        }
    } else {

        while (*secuencia->cursor && !strchr(delimitadores, *secuencia->cursor) && (cursor - texto) < (tamSalida - 1)) {
            *cursor++ = *secuencia->cursor++;
        }
    }

    *cursor = '\0';

    return texto == cursor ? TERMINAL_FIN_SECUENCIA : TERMINAL_TODO_OK;
}
