# Laberintos y Fantasmas

TP Algoritmos y Estructuras de Datos - Grupo Pixelados

####**Convenciones de Nombres**

|                    | **Formato**                | **Ejemplos**                                                        |
| ------------------ | -------------------------- | ------------------------------------------------------------------- |
| Variable           | `pascalCase`               | `contadorFrames`, `indiceActual`                                    |
| Typedef            | `pascalCase` (prefijo `t`) | `tNodo`, `tPila`, `tJugador`                                        |
| Struct             | `pascalCase` (prefijo `s`) | `sNodo`                                                             |
| Enum               | `pascalCase` (prefijo `e`) | `eRetornos`, `eMotorEstado`                                         |
| Elementos del Enum | `SCREAMING_SNAKE_CASE``    | `SPRITE_JUGADOR`                                                    |
| Macro              | `SCREAMING_SNAKE_CASE`     | `CANT_PIXELES`, `TAM_VEC`                                           |
| Funcion            | `snake_case`               | `inicializar_motor()`, `movimiento_es_valido()`, `destruir_juego()` |
| Funcion Static     | `snake_case `(prefijo `_`) | `_puede_avanzar()`, `_calcular_trayectoria()`                       |

#### **Librerias de terceros utilizadas****

-  [SDL  2.32.8](https://github.com/libsdl-org/SDL/releases/tag/release-2.32.8): **Simple DirectMedia Layer**.

-  [SDL2_image-2.8.8](https://github.com/libsdl-org/SDL_image/releases/tag/release-2.8.8): Decodificador para formatos de imagen populares para **Simple DirectMedia Layer**.

-  [SDL Mixer 2.8.1](https://github.com/libsdl-org/SDL_mixer/releases/tag/release-2.8.1): Mezclador de audio para **Simple DirectMedia Layer**. 

##### Pasos para la configuración del entorno:

1. Descomprimir `SDL2 2.32.8` en la carpeta `libraries` ubicada en la raiz del proyecto.

2. Descomprimir `SDL2_image 2.8.8` en una carpeta de su preferencia, y copiar la carpeta `x86_64-w64-mingw32` dentro de `SDL2-2.32.8`.

3. Descomprimir `SDL2_mixer 2.8.1` en una carpeta de su preferencia, y copiar la carpeta `x86_64-w64-mingw32` dentro de `SDL2-2.32.8`.

4. En Code::Blocks, click derecho sobre el nombre del proyecto, estando a la izquierda seleccionado debug:
   
   1. Acceda al tab `linker settings`: En `Link libraries` agregue `SDL2` , `SDL2_image` y `SDL2_mixer`. En `Other linker options` agregue -`lmingw32 -lSDL2main -lSDL2`. 
   
   2. Acceda al tab `Search Directories`: En el tab `Compiler` agregue `libraries\SDL2-2.32.8\x86_64-w64-mingw32\include`. Luego, en el tab `Linker` agregue `libraries\SDL2-2.32.8\x86_64-w64-mingw32\lib`

5. Pulse `OK`.

6. Copie los archivos `SDL2.dll`, `SDL2_image.dll` y `SDL2_mixer.dll` desde la carpeta `libraries\SDL2-2.32.8\x86_64-w64-mingw32\bin` a la carpeta `bin\Debug` en la raiz del proyecto.
