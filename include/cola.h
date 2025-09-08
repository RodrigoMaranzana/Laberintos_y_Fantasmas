#ifndef COLA_H_INCLUDED
#define COLA_H_INCLUDED

#define TODO_OK     0
#define SIN_MEMORIA 1
#define COLA_VACIA  2
#define COLA_LLENA  3

typedef struct sNodo
{
    void *dato;
    unsigned tamDato;
    struct sNodo *sig;
}tNodo;

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
