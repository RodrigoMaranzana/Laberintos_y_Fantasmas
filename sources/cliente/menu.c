#include "../../include/cliente/menu.h"
#include "../../include/comun/comun.h"
#include "../../include/cliente/graficos.h"
#include <stdlib.h>
#include <string.h>

#define PADDING_VERTICAL 8
#define PADDING_HORIZONTAL 16

tMenu* menu_crear(SDL_Renderer *renderer, unsigned capOpc, SDL_Point ubicacion, eMenuTipo menuTipo)
{
    tMenu* menu = (tMenu*) malloc(sizeof(tMenu));
    if (!menu) {

        return NULL;
    }

    menu->selecOpc = -1;

    menu->opciones = (tMenuOpcion*) malloc(capOpc * sizeof(tMenuOpcion));
    if (!menu->opciones) {

        free(menu);
        return NULL;
    }

    menu->capOpc = capOpc;
    menu->cantOpc = 0;
    menu->ubicacion = ubicacion;
    menu->menuTipo = menuTipo;
    menu->renderer = renderer;

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

            return MENU_SIN_MEM;
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

    if (menu->selecOpc == -1) {

        menu->selecOpc = pOpciones->id;
    }

    return MENU_ERR_TODO_OK;
}

void menu_siguiente_opcion(tMenu *menu)
{
    int i = 0;
    tMenuOpcion *pOpciones = menu->opciones, *pOpcionesUlt = (menu->opciones + menu->cantOpc - 1);

    if (menu->cantOpc == 0) {

        return;
    }

    while (pOpciones <= pOpcionesUlt && pOpciones->id != menu->selecOpc) {

        pOpciones++;
    }

    if (pOpciones > pOpcionesUlt) {

        pOpciones = menu->opciones;
    }

    do {

        pOpciones++;
        if (pOpciones > pOpcionesUlt) {
            pOpciones = menu->opciones;
        }
        i++;

    } while ((pOpciones->estado == OPCION_DESHABILITADA || pOpciones->estado == OPCION_OCULTA) && i < menu->cantOpc);

    menu->selecOpc = pOpciones->id;
}

void menu_anterior_opcion(tMenu *menu)
{
    int i = 0;
    tMenuOpcion *pOpciones = menu->opciones, *pOpcionesUlt = (menu->opciones + menu->cantOpc - 1);

    if (menu->cantOpc == 0) {

        return;
    }

    while (pOpciones <= pOpcionesUlt && pOpciones->id != menu->selecOpc) {

        pOpciones++;
    }

    if (pOpciones > pOpcionesUlt) {
        pOpciones = menu->opciones;
    }

    do {

        pOpciones--;
        if (pOpciones < menu->opciones) {
            pOpciones = pOpcionesUlt;
        }
        i++;

    } while ((pOpciones->estado == OPCION_DESHABILITADA || pOpciones->estado == OPCION_OCULTA) && i < menu->cantOpc);

    menu->selecOpc = pOpciones->id;
}

tMenuAccion menu_confirmar_opcion(tMenu *menu)
{
    tMenuOpcion *pOpciones = menu->opciones, *pOpcionesUlt = menu->opciones + menu->cantOpc - 1;
    tMenuAccion accionNula = {NULL, NULL};

    if (menu->cantOpc == 0) {

        return accionNula;
    }

    while (pOpciones <= pOpcionesUlt && pOpciones->id != menu->selecOpc) {

        pOpciones++;
    }

    if (pOpciones <= pOpcionesUlt) {

        return pOpciones->accion;
    }

    return accionNula;
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

void menu_dibujar(tMenu* menu)
{
    tMenuOpcion *pOpciones, *pOpcionesUlt = (menu->opciones + menu->cantOpc - 1);
    SDL_Rect rectDestino;

    rectDestino.x = menu->ubicacion.x;
    rectDestino.y = menu->ubicacion.y;

    for (pOpciones = menu->opciones; pOpciones <= pOpcionesUlt; pOpciones++) {

        if (pOpciones->estado != OPCION_OCULTA) {

            rectDestino.w = pOpciones->tamTextura.x;
            rectDestino.h = pOpciones->tamTextura.y;

            if (pOpciones->estado == OPCION_HABILITADA) {

                if (pOpciones->id == menu->selecOpc) {

                    SDL_SetTextureColorMod(pOpciones->textura, 255, 255, 0);
                } else {

                    SDL_SetTextureColorMod(pOpciones->textura, 255, 255, 255);
                }
            } else {

                SDL_SetTextureColorMod(pOpciones->textura, 128, 128, 128);
            }

            graficos_dibujar_textura(pOpciones->textura, menu->renderer, NULL, &rectDestino, NULL);

            if (menu->menuTipo == MENU_VERTICAL) {

                rectDestino.y += pOpciones->tamTextura.y + PADDING_VERTICAL;
            }else {

                rectDestino.x += pOpciones->tamTextura.x + PADDING_HORIZONTAL;
            }

        }
    }
}
















