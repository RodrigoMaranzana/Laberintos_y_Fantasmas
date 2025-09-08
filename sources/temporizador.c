#include "../include/temporizador.h"
#include <SDL.h>
#include <stdlib.h>

void temporizador_inicializar(tTemporizador *tempor, float duracion)
{
    tempor->duracion = duracion;
    tempor->estado = TEMPOR_INACTIVO;
}

void temporizador_actualizar(tTemporizador* tempor)
{
    unsigned tiempoActual;
    float deltaTiempo;

    if (tempor->estado != TEMPOR_ACTIVO) {
        return;
    }

    tiempoActual = SDL_GetTicks();

    if (tempor->tiempoAnterior == 0) {
        tempor->tiempoAnterior = tiempoActual;
        return;
    }

    deltaTiempo = (tiempoActual - tempor->tiempoAnterior) / 1000.0f;
    tempor->transcurrido += deltaTiempo;
    tempor->tiempoAnterior = tiempoActual;

    if (tempor->transcurrido >= tempor->duracion) {
        tempor->transcurrido = tempor->duracion;
        tempor->estado = TEMPOR_FINALIZADO;
    }
}

eTemporEstado temporizador_estado(const tTemporizador* temporizador)
{
    return temporizador->estado;
}

void temporizador_iniciar(tTemporizador* temporizador)
{
    temporizador->estado = TEMPOR_ACTIVO;
    temporizador->transcurrido = 0.0f;
    temporizador->tiempoAnterior = SDL_GetTicks();
}

void temporizador_pausar(tTemporizador* temporizador)
{
    if (temporizador->estado == TEMPOR_ACTIVO) {
        temporizador->estado = TEMPOR_INACTIVO;
    }
}

void temporizador_reanudar(tTemporizador* temporizador)
{
    if (temporizador->estado == TEMPOR_INACTIVO) {
        temporizador->estado = TEMPOR_ACTIVO;
    }
}
