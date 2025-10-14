#include "../../include/cliente/archivo.h"
#include "../../include/comun/comun.h"

#include <string.h>
#include <ctype.h>

#define MASC_FILAS              0b000001
#define MASC_COLUMNAS           0b000010
#define MASC_VIDAS_INI          0b000100
#define MASC_MAX_NUM_FANTASMAS  0b001000
#define MASC_MAX_NUM_PREMIOS    0b010000
#define MASC_MAX_VIDAS_EXTRA    0b100000
#define CANT_PARAM              0b111111

static int _archivo_parsear_linea_conf(char *buffer, tParam *param);

int archivo_leer_conf(FILE* arch, tConf *conf)
{
    int ret;
    char paramValidos = 0;
    char buffer[TAM_LINEA];

    while(fgets(buffer, TAM_LINEA, arch))
    {
        tParam paramAux;

        ret = _archivo_parsear_linea_conf(buffer, &paramAux);
        if(ret == ERR_LINEA_LARGA)
        {
            return ERR_CONF;
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
        return ERR_CONF;
    }

    return ERR_TODO_OK;
}

int archivo_escribir_conf(FILE* arch, const tConf *conf)
{
    int ret;

    ret = fprintf(arch, "FILAS : %d\n", conf->filas);
    ret = fprintf(arch, "COLUMNAS : %d\n",  conf->columnas);
    ret = fprintf(arch, "VIDAS_INICIO : %d\n",  conf->vidas_inicio);
    ret = fprintf(arch, "MAXIMO_NUMERO_FANTASMAS : %d\n",  conf->max_num_fantasmas);
    ret = fprintf(arch, "MAXIMO_NUMERO_PREMIOS : %d\n",  conf->max_num_premios);
    ret = fprintf(arch, "MAXIMO_VIDAS_EXTRA : %d\n",  conf->max_vidas_extra);

    return ret >= 0 ? ERR_TODO_OK : ERR_ARCHIVO;
}

int archivo_escribir_escenario(tEscenario *escenario, int numRonda, long semillaRonda)
{
    int fila, columna, c;
    char nombreArch[TAM_NOMBRE_ARCH];

    sprintf(nombreArch, "Ronda-%d_%ld.txt", numRonda, semillaRonda);
    FILE *arch = fopen(nombreArch, "w");
    if (!arch){
        return ERR_ARCHIVO;
    }

    for (fila = 0; fila < escenario->cantFilas; fila++)
    {
        for (columna = 0; columna < escenario->cantColumnas; columna++)
        {
            if (escenario->tablero[fila][columna].tile->tileTipo == TILE_TIPO_PARED) {
                c = '#';

            }else if (escenario->tablero[fila][columna].tile->tileTipo == TILE_TIPO_PUERTA_ENTRADA) {
                c = 'E';

            }else if (escenario->tablero[fila][columna].tile->tileTipo == TILE_TIPO_PUERTA_SALIDA) {
                c = 'S';

            }else if (escenario->tablero[fila][columna].entidad
                     && escenario->tablero[fila][columna].entidad->tipo != ENTIDAD_JUGADOR) {
                c = 'F';


            }else if (escenario->tablero[fila][columna].extra) {
                if (escenario->tablero[fila][columna].extra == EXTRA_VIDA) {
                    c = 'V';
                }else{
                    c = 'P';
                }

            }else {
                c = '.';
            }

            fputc(c,arch);
        }

         fputc('\n',arch);

    }

    fclose(arch);
    return ERR_TODO_OK;
}

/*************************
    FUNCIONES ESTATICAS
*************************/

static int _archivo_parsear_linea_conf(char *buffer, tParam *param)
{
    char* cursor = strchr(buffer, '\n');
    if(!cursor)
    {
        return ERR_LINEA_LARGA;
    }
    *cursor = '\0';

    cursor = strrchr(buffer, ':');
    if (!cursor) {

        return ERR_LINEA_LARGA;
    }

    while(*cursor != '\0' && !ES_DIGITO(*cursor))
    {
        cursor++;
    }

    if(*cursor == '\0')
    {
        return ERR_LINEA_LARGA;
    }

    sscanf(cursor, "%d", &param->valor);/// REVISAR CURSOR / CURSOR + 1
    *cursor = '\0';

    strncpy(param->nombre, buffer, TAM_NOMBRE);
    param->nombre[TAM_NOMBRE] = '\0';

    cursor = param->nombre;

    while(*cursor && (ES_LETRA(*cursor) || *cursor == '_')) {

        *cursor = toupper(*cursor);
        cursor++;
    }

    *cursor = '\0';

    return ERR_TODO_OK;
}
