#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

#include <stdlib.h>


typedef struct{
    void* vec;
    int ce;
    size_t cap;
    size_t tamElem;
}tVector;

typedef struct{
    tVector* vector;
    int esFin;
    void* actual;
    void* ult;
}tVectorIterador;

//PUNTEROS A FUNCION
typedef int (*Cmp)(const void* e1, const void* e2);
typedef void (*Actualizar)(void* actualizado, const void* actualizador);
typedef void (*Accion)(void* elem, void* extra);


//FUNCIONES PRIMITIVAS
int vector_crear(tVector* vector, size_t tamElem);
int vector_cargar_de_archivo(tVector* vector, const char* nombreArch, size_t tamElem);
void vector_recorrer(tVector* vector, Accion accion, void* extra);
void vector_destruir(tVector* vector);
int vector_ord_insertar(tVector* vector, void* elem, Cmp cmp, Actualizar actualizar);
int vector_ord_buscar(tVector* vector, void* elem, Cmp cmp);
void vector_it_crear(tVectorIterador* vectorIterador, tVector* vector);
void* vector_it_primero(tVectorIterador* vectorIterador);
void* vector_it_siguiente(tVectorIterador* vectorIterador);
int es_fin_vector_it(tVectorIterador* vectorIterador);


#endif // VECTOR_H_INCLUDED
