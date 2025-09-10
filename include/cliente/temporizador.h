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
} tTemporizador;

void temporizador_inicializar(tTemporizador *tempor, float duracion);
void temporizador_actualizar(tTemporizador* tempor);
eTemporEstado temporizador_estado(const tTemporizador* tempor);
void temporizador_iniciar(tTemporizador* tempor);
void temporizador_pausar(tTemporizador* tempor);
void temporizador_reanudar(tTemporizador* tempor);

#endif // TEMPORIZADOR_H_INCLUDED
