#ifndef MATRICES_H_INCLUDED
#define MATRICES_H_INCLUDED

#include <stdlib.h>
#include <string.h>


void** crear_matriz(size_t filas, size_t columnas, size_t tamElem);
void destruir_matriz(void** matriz, size_t filas);
void init_matriz(void** matriz ,size_t filas, size_t columnas, size_t tamElem);


#endif // MATRICES_H_INCLUDED
