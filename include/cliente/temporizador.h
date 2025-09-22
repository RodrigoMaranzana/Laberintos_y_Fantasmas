#ifndef TEMPORIZADOR_H_INCLUDED
#define TEMPORIZADOR_H_INCLUDED

typedef enum {
    TEMPOR_INACTIVO,
    TEMPOR_ACTIVO,
    TEMPOR_FINALIZADO,
}eTemporEstado;

typedef struct {
    float duracion;
    float tiempoAnterior;
    float transcurrido;
    eTemporEstado estado;
} tTempor;

void temporizador_inicializar(tTempor *tempor, float duracion);
void temporizador_actualizar(tTempor* tempor);
eTemporEstado temporizador_estado(const tTempor* tempor);
void temporizador_iniciar(tTempor* tempor);
void temporizador_pausar(tTempor* tempor);
void temporizador_reanudar(tTempor* tempor);

#endif // TEMPORIZADOR_H_INCLUDED
