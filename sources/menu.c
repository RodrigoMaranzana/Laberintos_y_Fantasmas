#include "../include/menu.h"
#include "../include/retorno.h"
#include "../include/graficos.h"
#include "../include/input.h"
#include <stdlib.h>
#include <string.h>

tMenu* menu_crear(unsigned capOpc, SDL_Point ubicacion)
{
    tMenu* menu = (tMenu*) malloc(sizeof(tMenu));
    if (!menu) {

        return NULL;
    }

    menu->selecOpc = 0;

    menu->opciones = (tMenuOpcion*) malloc(capOpc * sizeof(tMenuOpcion));
    if (!menu->opciones) {

        free(menu);
        return NULL;
    }

    menu->capOpc = capOpc;
    menu->cantOpc = 0;
    menu->ubicacion = ubicacion;

    return menu;
}

void menu_destruir(tMenu* menu)
{
    tMenuOpcion *pOpciones = menu->opciones, *pOpcionesUlt = (menu->opciones + menu->cantOpc - 1);

    while (pOpciones <= pOpcionesUlt) {

        SDL_DestroyTexture(pOpciones->textura);
        pOpciones++;
    }

    free(menu->opciones);
    free(menu);
}

int menu_agregar_opcion(tMenu *menu, int id, SDL_Texture *textura, unsigned tamAltura, tMenuAccion accion, eOpcionEstado estado)
{
    unsigned nuevaCapOpc;
    tMenuOpcion *pOpciones;
    int x, y;

    if (menu->cantOpc >= menu->capOpc) {

        nuevaCapOpc = menu->capOpc * 2;
        tMenuOpcion* nuevasOpciones = (tMenuOpcion*) realloc(menu->opciones, nuevaCapOpc * sizeof(tMenuOpcion));
        if (!nuevasOpciones) {

            return ERR_SIN_MEMORIA;
        }

        menu->opciones = nuevasOpciones;
        menu->capOpc = nuevaCapOpc;
    }

    pOpciones = menu->opciones;
    pOpciones += menu->cantOpc;

    SDL_QueryTexture(textura, NULL, NULL, &x, &y);

    pOpciones->tamTextura.x = (x / y) * tamAltura;
    pOpciones->tamTextura.y = tamAltura;

    pOpciones->textura = textura;

    pOpciones->id = id;
    pOpciones->estado = estado;
    pOpciones->accion = accion;
    menu->cantOpc++;

    return TODO_OK;
}

void menu_siguiente_opcion(tMenu *menu)
{
    int encontrado = 0;

    if (menu->cantOpc == 0) {

        return;
    }

    for (int i = 0; !encontrado && i < menu->cantOpc; i++) {

        menu->selecOpc++;
        if (menu->selecOpc >= menu->cantOpc) {

            menu->selecOpc = 0;
        }

        if (menu->opciones[menu->selecOpc].estado == OPCION_HABILITADA) {

            encontrado = 1;
        }
    }
}

void menu_anterior_opcion(tMenu *menu)
{
    int encontrado = 0;

    if (menu->cantOpc == 0) {

        return;
    }

    for (int i = 0; !encontrado && i < menu->cantOpc; i++) {

        menu->selecOpc--;
        if (menu->selecOpc < 0) {

            menu->selecOpc = menu->cantOpc - 1;
        }

        if (menu->opciones[menu->selecOpc].estado == OPCION_HABILITADA) {

            encontrado = 1;
        }
    }
}

tMenuAccion menu_confirmar_opcion(tMenu *menu)
{
    if (menu->cantOpc > 0 && menu->selecOpc < menu->cantOpc && menu->opciones[menu->selecOpc].estado == OPCION_HABILITADA) {

        return menu->opciones[menu->selecOpc].accion;
    }

    return (tMenuAccion) {
        NULL, NULL
    };
}

void menu_estado_opcion(tMenu *menu, int id, eOpcionEstado nuevoEstado)
{
    tMenuOpcion *pOpciones = menu->opciones, *pOpcionesUlt = (menu->opciones + menu->cantOpc - 1);

    while (pOpciones <= pOpcionesUlt && pOpciones->id != id) {

        pOpciones++;
    }

    if (pOpciones <= pOpcionesUlt && pOpciones->id == id) {

        pOpciones->estado = nuevoEstado;
    }
}

void menu_dibujar(SDL_Renderer *renderer, tMenu* menu)
{
    tMenuOpcion *pOpciones, *pOpcionesUlt = (menu->opciones + menu->cantOpc - 1);
    SDL_Rect rectDestino;
    int padding = 8, i = 0;

    rectDestino.x = menu->ubicacion.x;
    rectDestino.y = menu->ubicacion.y;

    for (pOpciones = menu->opciones; pOpciones <= pOpcionesUlt; pOpciones++, i++) {

        if (pOpciones->estado != OPCION_OCULTA) {

            rectDestino.w = pOpciones->tamTextura.x;
            rectDestino.h = pOpciones->tamTextura.y;

            if (pOpciones->estado == OPCION_HABILITADA) {

                if (i == menu->selecOpc) {

                    SDL_SetTextureColorMod(pOpciones->textura, 255, 255, 0);
                } else {

                    SDL_SetTextureColorMod(pOpciones->textura, 255, 255, 255);
                }
            } else {

                SDL_SetTextureColorMod(pOpciones->textura, 128, 128, 128);
            }

            graficos_dibujar_textura(pOpciones->textura, renderer, NULL, &rectDestino);
            rectDestino.y += pOpciones->tamTextura.y + padding;
        }
    }
}
















