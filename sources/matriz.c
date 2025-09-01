#include "../include/matriz.h"
#include "../include/retorno.h"

void** matriz_crear(size_t columnas, size_t filas, size_t tamElem)
{
    void** fila;
    void** ultimaFila;
    void** matriz;
    int estado = TODO_OK;

    matriz = malloc(filas * sizeof(void*));
    if(!matriz){

        return NULL;
    }

    ultimaFila = matriz + filas - 1;

    for(fila = matriz; estado == TODO_OK && fila <= ultimaFila; fila++){

        *fila = malloc(columnas * tamElem);
        if(!*fila){

            matriz_destruir(matriz, fila - matriz);
            estado = ERR_SIN_MEMORIA;
        }
    }

    return matriz;
}

void matriz_destruir(void** matriz, size_t filas)
{
    void **fila;
    void **ultimaFila = matriz + filas - 1;

    for(fila = matriz; fila <= ultimaFila; fila++){

        free(*fila);
        *fila = NULL;
    }

    free(matriz);
    matriz = NULL;
}

void matriz_inicializar(void** matriz, void *dato, size_t columnas, size_t filas, size_t tamElem)
{
    size_t fila;

    for(fila = 0; fila < filas; fila++){

        memcpy(matriz[fila], dato, tamElem);
    }
}




