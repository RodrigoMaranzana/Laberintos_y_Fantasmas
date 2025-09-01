#include "../include/vector.h"
#include "../include/retornos.h"

#include <string.h>
#include <stdio.h>

//MACROS
#define CAP_INI             10
#define FACTOR_INC          2

int vector_crear(tVector* vector, size_t tamElem)
{
	vector->ce = 0;

	vector->vec = malloc(CAP_INI * tamElem);
	if(!vector->vec)
    {
        vector->cap = 0;
        vector->tamElem = 0;

        return ERR_SIN_MEMORIA;
    }

    vector->cap = CAP_INI;
    vector->tamElem = tamElem;

	return TODO_OK;
}

int vector_cargar_de_archivo(tVector* vector, const char* nombreArch, size_t tamElem)
{
    size_t tamArch, cantReg;

    FILE* arch = fopen(nombreArch, "rb");
    if(!arch)
    {
        return ERR_ARCHIVO;
    }

    fseek(arch, 0, SEEK_END);

    tamArch = ftell(arch);
    cantReg = tamArch / tamElem;

    rewind(arch);

    vector->vec = malloc(tamArch);
    if(!vector->vec)
    {
        fclose(arch);
        return ERR_SIN_MEMORIA;
    }
    vector->cap = cantReg;
    vector->ce = cantReg;
    vector->tamElem = tamElem;

    fread(vector->vec, tamElem, cantReg, arch);

    fclose(arch);

    return TODO_OK;
}

void vector_recorrer(tVector* vector, Accion accion, void* extra)
{
    void* ult = vector->vec + ((vector->ce - 1) * vector->tamElem);
    void* i;

    for(i = vector->vec; i <= ult; i+= vector->tamElem)
    {
        accion(i, extra);
    }
}

void vector_destruir(tVector* vector)
{
    free(vector->vec);
	vector->vec = NULL;
	vector->cap = 0;
	vector->ce = 0;
	vector->tamElem = 0;
}

int vector_ord_insertar(tVector *vector, void *elem, Cmp cmp, Actualizar actualizar)
{
    void *actual, *ult, *posIns;

    if(vector->ce == vector->cap){//vector LLENO

        size_t capNueva = vector->cap * FACTOR_INC;
        void* vecNuevo = realloc(vector->vec, capNueva * vector->tamElem);
        if(!vecNuevo){

            return ERR_SIN_MEMORIA;
        }

        vector->cap = capNueva;
        vector->vec = vecNuevo;
    }

    actual = vector->vec;
    ult = vector->vec + ((vector->ce - 1) * vector->tamElem);

    while(actual <= ult && cmp(elem, actual) > 0){

        actual += vector->tamElem;
    }

    if(actual <= ult && cmp(elem, actual) == 0){

        actualizar(actual, elem);
        return TODO_OK;
    }

    posIns = actual;

    for(actual = ult; actual >= posIns; actual -= vector->tamElem){

        memcpy(actual + vector->tamElem, actual, vector->tamElem);
    }

    //inserto el elemento
    memcpy(posIns, elem, vector->tamElem);

    vector->ce++;

    return TODO_OK;
}

int vector_ord_buscar(tVector* vector, void* elem, Cmp cmp)
{
    void* actual = vector->vec;
    void* ult = vector->vec + ((vector->ce - 1) * vector->tamElem);

    while(actual <= ult && cmp(elem, actual) > 0){

        actual += vector->tamElem;
    }

    if(actual <= ult && cmp(elem, actual) == 0){
        memcpy(elem, actual, vector->tamElem);
        return (actual - vector->vec) / vector->tamElem;

    }

    return -1;
}

void vector_it_crear(tVectorIterador* vectorIterador, tVector* vector)
{
    vectorIterador->vector = vector;
    vectorIterador->actual = vectorIterador->vector->vec;
    vectorIterador->esFin = 0;

}

void* vector_it_primero(tVectorIterador* vectorIterador)
{
    if(vectorIterador->vector->ce == 0){
       vectorIterador->esFin = 0;
       return NULL;
   }

   vectorIterador->actual = vectorIterador->vector->vec;
   vectorIterador->ult = vectorIterador->vector->vec + ((vectorIterador->vector->ce - 1) * vectorIterador->vector->tamElem);

   return vectorIterador->actual;

}

void* vector_it_siguiente(tVectorIterador* vectorIterador)
{
    vectorIterador->actual += vectorIterador->vector->tamElem;

    if(vectorIterador->actual > vectorIterador->ult){

        return NULL;
    }

    return vectorIterador->actual;
}

int es_fin_vector_it(tVectorIterador* vectorIterador)
{
    if(vectorIterador->actual <= vectorIterador->ult){

        return 0;
    }

    return 1;
}








