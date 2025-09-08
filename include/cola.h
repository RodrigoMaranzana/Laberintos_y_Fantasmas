#ifndef COLA_H_INCLUDED
#define COLA_H_INCLUDED
<<<<<<< HEAD
#include "comun.h"
=======
>>>>>>> 7f2d18524b9558ce7a63f639d92cc2f793669acc

#define TODO_OK     0
#define SIN_MEMORIA 1
#define COLA_VACIA  2
#define COLA_LLENA  3

<<<<<<< HEAD
=======
typedef struct sNodo
{
    void *dato;
    unsigned tamDato;
    struct sNodo *sig;
}tNodo;

>>>>>>> 7f2d18524b9558ce7a63f639d92cc2f793669acc
typedef struct
{
    tNodo *pri;
    tNodo *ult;
}tCola;


void cola_crear(tCola *cola);
int cola_encolar(tCola *cola, const void *dato, unsigned tamDato);
int cola_desencolar(tCola *cola, void *dato, unsigned tamDato);
int cola_ver_primero(const tCola *cola, void *dato, unsigned tamDato);
int cola_vacia(const tCola *cola);
int cola_llena(const tCola *cola, unsigned tamDato);
void cola_vaciar(tCola *cola);

#endif // COLA_H_INCLUDED
