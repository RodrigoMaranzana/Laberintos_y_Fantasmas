#include "../../include/cliente/archivo.h"
#include "../../include/comun/retorno.h"
#include "../../include/comun/comun.h"

#include <string.h>
#include <ctype.h>

static int _archivo_parsear_linea_conf(char *buffer, tParam *param);


int archivo_leer_conf(FILE* arch, tConf *conf)
{
    int ret;
    tParam paramAux;
    char paramValidos = 0;
    char buffer[TAM_LINEA];

    while(fgets(buffer, TAM_LINEA, arch))
    {
        ret = _archivo_parsear_linea_conf(buffer, &paramAux);

        if(ret != TODO_OK)
        {
            return ERR_ARCHIVO;
        }

        if(strcmp(paramAux.nombre, "FILAS") == 0)
        {
            conf->filas = paramAux.valor;
            paramValidos |= MASC_FILAS;

        }else if(strcmp(paramAux.nombre, "COLUMNAS") == 0)
        {
            conf->columnas = paramAux.valor;
            paramValidos |= MASC_COLUMNAS;

        }else if(strcmp(paramAux.nombre, "VIDAS_INICIO") == 0)
        {
            conf->vidas_inicio = paramAux.valor;
            paramValidos |= MASC_VIDAS_INI;

        }else if(strcmp(paramAux.nombre, "MAXIMO_NUMERO_FANTASMAS") == 0)
        {
            conf->max_num_fantasmas = paramAux.valor;
            paramValidos |= MASC_MAX_NUM_FANTASMAS;

        }else if(strcmp(paramAux.nombre, "MAXIMO_NUMERO_PREMIOS") == 0)
        {
            conf->max_num_premios = paramAux.valor;
            paramValidos |= MASC_MAX_NUM_PREMIOS;

        }else if(strcmp(paramAux.nombre, "MAXIMO_VIDAS_EXTRA") == 0)
        {
            conf->max_vidas_extra = paramAux.valor;
            paramValidos |= MASC_MAX_VIDAS_EXTRA;
        }

    }

    if(paramValidos != CANT_PARAM)
    {
        return ERR_ARCHIVO;
    }


    return TODO_OK;
}



static int _archivo_parsear_linea_conf(char *buffer, tParam *param)
{
    char* pAux;
    char* cursor = strchr(buffer, '\n');
    if(!cursor)
    {
        return ERR_LINEA_LARGA;
    }
    *cursor = '\0';

    cursor = strrchr(buffer, ':');
    pAux = cursor;

    while(*pAux != '\0' || !ES_DIGITO(*pAux))
    {
        pAux++;
    }

    if(*pAux == '\0')
    {
        return ERR_ARCHIVO;
    }

    sscanf(pAux, "%d", &param->valor);

    *cursor = '\0';

    strncpy(param->nombre, buffer, TAM_NOMBRE);
    param->nombre[TAM_NOMBRE] = '\0';

    pAux = param->nombre;

    while(*pAux)
    {
        if(ES_LETRA(*pAux))
        {
            *pAux = toupper(*pAux);
            pAux++;

        }else
        {
            *pAux = '\0';
        }
    }

    return TODO_OK;
}

