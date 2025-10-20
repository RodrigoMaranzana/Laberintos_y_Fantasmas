#include "../../include/comun/comun.h"
#include "../../include/comun/mensaje.h"
#include <windows.h>
#include <stdio.h>

int comun_crear_directorio(const char *directorio)
{
    if (CreateDirectory(directorio, NULL)) {

        mensaje_color(TEXTO_RESET, "Directorio %s creado correctamente.", directorio);
    } else {

        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            mensaje_color(TEXTO_MAGENTA, "[DEBUG] " TEXTO_MAGENTA_B "Ya existe el directorio %s.", directorio);
        } else {
            printf(TEXTO_ROJO PARPADEO "[ERROR] " TEXTO_RESET "No se pudo crear el directorio %s.", directorio);
            return ERR_ARCHIVO;
        }
    }

    return ERR_TODO_OK;
}
