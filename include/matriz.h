#ifndef MATRICES_H_INCLUDED
#define MATRICES_H_INCLUDED

#include <stdlib.h>
#include <string.h>


void** matriz_crear(size_t columnas, size_t filas, size_t tamElem);
void matriz_destruir(void** matriz, size_t filas);
void matriz_inicializar(void** matriz, void *dato, size_t columnas, size_t filas, size_t tamElem);


#endif // MATRICES_H_INCLUDED
