#include "../../include/comun/cola.h"
#include "../../include/comun/comun.h"

#include <stdlib.h>
#include <string.h>

void cola_crear(tCola *cola)
{
    cola->pri = NULL;
    cola->ult = NULL;
}

int cola_encolar(tCola *cola, const void *dato, unsigned tamDato)
{
    tNodo *nodoNue = (tNodo*)malloc(sizeof(tNodo));
    if(!nodoNue)
    {
        return COLA_SIN_MEM;
    }

    nodoNue->dato = malloc(tamDato);
    if(!nodoNue->dato)
    {
        free(nodoNue);
        return COLA_SIN_MEM;
    }

    memcpy(nodoNue->dato, dato, tamDato);
    nodoNue->tamDato = tamDato;
    nodoNue->sig = NULL;

    if(cola->ult)
    {
        //El siguiente elemento de la cola es el nuevo nodo
        cola->ult->sig = nodoNue;
    }
    else{
        //si la cola esta vacia, el nuevo nodo es el primero
        cola->pri = nodoNue;
    }

    cola->ult = nodoNue;

    return COLA_TODO_OK;
}

int cola_desencolar(tCola *cola, void *dato, unsigned tamDato)
{
    tNodo *nodoElim = cola->pri;
    if(nodoElim == NULL)
    {
        return COLA_VACIA;
    }

    cola->pri = nodoElim->sig;
    memcpy(dato, nodoElim->dato, MIN(nodoElim->tamDato, tamDato));
    free(nodoElim->dato);
    free(nodoElim);

    if(cola->pri == NULL)
    {
        //No hay mas elementos en la cola
        cola->ult = NULL;
    }

    return COLA_TODO_OK;
}

int cola_ver_primero(const tCola *cola, void *dato, unsigned tamDato)
{
    if(cola->pri == NULL)
    {
        return COLA_VACIA;
    }

    memcpy(dato, cola->pri->dato, MIN(cola->pri->tamDato, tamDato));

    return COLA_TODO_OK;
}

int cola_vacia(const tCola *cola)
{
    return cola->pri == NULL? COLA_VACIA : COLA_TODO_OK;
}

int cola_llena(const tCola *cola, unsigned tamDato)
{
    tNodo *nodoAux = (tNodo*)malloc(sizeof(tNodo));
    void  *dato = malloc(tamDato);

    free(nodoAux);
    free(dato);

    return nodoAux == NULL || dato == NULL? COLA_LLENA : COLA_TODO_OK;
}

void cola_vaciar(tCola *cola)
{
    while(cola->pri)
    {
        tNodo *nodoElim = cola->pri;
        cola->pri = nodoElim->sig;
        free(nodoElim->dato);
        free(nodoElim);
    }
    cola->ult = NULL;
}
