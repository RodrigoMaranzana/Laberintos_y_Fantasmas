#include "../include/menu.h"
#include "../include/retorno.h"
#include "../include/graficos.h"
#include "../include/input.h"
#include <stdlib.h>
#include <string.h>

tMenu* menu_crear(unsigned capOpc, SDL_Point ubicacion)
{
    tMenu* menu = (tMenu*)malloc(sizeof(tMenu));
    if(!menu){

        return NULL;
    }

    menu->selecOpc = 0;

    menu->opciones = (tMenuOpcion*)malloc(capOpc * sizeof(tMenuOpcion));
    if(!menu->opciones){

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

    while(pOpciones <= pOpcionesUlt){

        SDL_DestroyTexture(pOpciones->textura);
        pOpciones++;
    }

    free(menu->opciones);
    free(menu);
}

int menu_agregar_opcion(tMenu *menu, SDL_Texture *textura, unsigned tamAltura, tMenuAccion accion)
{
    unsigned nuevaCapOpc;
    tMenuOpcion *pOpciones;
    int x, y;

    if(menu->cantOpc >= menu->capOpc){

        nuevaCapOpc = menu->capOpc * 2;
        tMenuOpcion* nuevasOpciones = (tMenuOpcion*)realloc(menu->opciones, nuevaCapOpc * sizeof(tMenuOpcion));
        if(!nuevasOpciones){

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

    pOpciones->accion = accion;
    menu->cantOpc++;

    return TODO_OK;
}

tMenuAccion menu_procesar_comando(tMenu* menu, eMenuComando comando)
{
    switch(comando){

        case MENU_ANTERIOR:
            menu->selecOpc--;
            if(menu->selecOpc < 0){

                menu->selecOpc = menu->cantOpc - 1;
            }
            break;
        case MENU_SIGUIENTE:
            menu->selecOpc++;
            if(menu->selecOpc >= menu->cantOpc){

                menu->selecOpc = 0;
            }
            break;
        case MENU_CONFIRMAR:
            return menu->opciones[menu->selecOpc].accion;;
            break;
        default:
            break;
    }

    return (tMenuAccion){NULL, NULL};
}

void menu_dibujar(SDL_Renderer *renderer, tMenu* menu)
{
    tMenuOpcion *pOpciones, *pOpcionesUlt = (menu->opciones + menu->cantOpc - 1);
    SDL_Rect rectDestino;
    int padding = 8, i = 0;

    rectDestino.x = menu->ubicacion.x;
    rectDestino.y = menu->ubicacion.y;

    for(pOpciones = menu->opciones; pOpciones <= pOpcionesUlt; pOpciones++, i++){

        rectDestino.w = pOpciones->tamTextura.x;
        rectDestino.h = pOpciones->tamTextura.y;

        if(i == menu->selecOpc){

            SDL_SetTextureColorMod(pOpciones->textura, 255, 255, 0);
        }else{

            SDL_SetTextureColorMod(pOpciones->textura, 255, 255, 255);
        }

        graficos_dibujar_textura(pOpciones->textura, renderer, NULL, &rectDestino);

        rectDestino.y += pOpciones->tamTextura.y + padding;
    }
}













