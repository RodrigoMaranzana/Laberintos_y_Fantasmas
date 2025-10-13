#ifndef VENTANA_H_INCLUDED
#define VENTANA_H_INCLUDED

#include <SDL.h>

#define TAM_NOMBRE_VENTANA 25

typedef enum {
    VENTANA_BD_ERR_TODO_OK,
} eVentanaErr;

typedef struct sVentana tVentana;

typedef int (*tVentanaCrearDatos)(void *datos);
typedef void (*tVentanaActualizar)(SDL_Event *event, void *datos);
typedef void (*tVentanaDibujar)(void *datos);
typedef void (*tVentanaDestruirDatos)(void *datos);

typedef struct {
    tVentanaCrearDatos crearDatos;
    tVentanaActualizar actualizar;
    tVentanaDibujar dibujar;
    tVentanaDestruirDatos destruirDatos;
    void *datos;
} tVentanaAccion;

tVentana* ventana_crear(SDL_Renderer *renderer, tVentanaAccion accion, SDL_Rect dimensiones, SDL_Color cFondo, char conSombra);
void ventana_destruir(tVentana *ventana);
void ventana_abrir(tVentana *ventana);
void ventana_cerrar(tVentana *ventana);
void ventana_actualizar(tVentana *ventana, SDL_Event *event);
void ventana_dibujar(tVentana *ventana);


#endif // VENTANA_H_INCLUDED
