#include "../include/matrices.h"


void** crearMatriz(size_t filas, size_t columnas, size_t tamElem)
{
    void** fila;
    void** ultimaFila;
    void** matriz = malloc(filas * sizeof(void*));

    if(!matriz){
        return NULL;
    }

    ultimaFila = matriz + filas - 1;

    for(fila = matriz; fila <= ultimaFila; fila++)
    {

        *fila = malloc(columnas * tamElem);

        if(!*fila)
        {
            destruirMatriz(matriz, fila - matriz);
            return NULL;
        }
    }

    return matriz;
}

void destruirMatriz(void** matriz, size_t filas)
{
    void **fila;
    void **ultimaFila = matriz + filas - 1;


    for(fila = matriz; fila <= ultimaFila; fila++)
    {
        free(*fila);
        *fila = NULL;
    }

    free(matriz);
    matriz = NULL;
}

void initMatriz(void** matriz ,size_t filas, size_t columnas, size_t tamElem)
{
    size_t fila;

    for(fila = 0; fila < filas; fila++)
    {
        memset(matriz[fila], 0, columnas * tamElem);
    }
}
