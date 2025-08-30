#ifndef MATRICES_H_INCLUDED
#define MATRICES_H_INCLUDED

#include <stdlib.h>
#include <string.h>


void** crearMatriz(size_t filas, size_t columnas, size_t tamElem);

void destruirMatriz(void** matriz, size_t filas);

void initMatriz(void** matriz ,size_t filas, size_t columnas, size_t tamElem);


#endif // MATRICES_H_INCLUDED
