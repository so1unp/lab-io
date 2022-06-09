# Laboratorio 8 - E/S

## Ejercicio 1: buffering
El programa `write_bytes.c` escribe la cantidad de bytes indicada en un archivo. Por ejemplo, el siguiente comando escribe 100 Mb en el archivo `tmp.txt`, usando un buffer de 4096 bytes:
```
$ bin/write_bytes tmp.txt 104857600 4096 0
```

Responder:
1. Obtener un promedio de tiempo de ejecución del programa, usando el comando `time`, al crear un archivo de 100 Mb, con tamaños de buffer de 256, 1024, 4096 y 8192 bytes. Presentar una tabla que indique el tiempo total, de usuario, de sistema y uso de la CPU. Realizar 10 ejecuciones para obtener el promedio. Explicar los resultados.

2. Comparar la ejecución del comando al crear un archivo de 10 MiB con y sin sincronización automática con el disco. Explicar los resultados.

## Ejercicio 2: Interrupción del teclado

1. Describir el proceso de E/S en xv6 cuando el usuario presiona una tecla del teclado. ¿Se utilizan comandos específicos de E/S o se utiliza E/S mapeada en memoria?

2. Cuando se presiona una tecla, ¿El caracter es inmediatamente enviado al proceso que este leyendo de la entrada estándar? Justificar.

## Ejercicio 3: Comunicación por la pantalla.
Para simular la salida por un monitor conectado a la tarjeta de vídeo en la PC, hay que ejecutar _xv6_ utilizando el comando `make -qemu` (sin el `-nox`).

La función `cgaputc()` escribe el carácter que recibe como parámetro en la memoria de video en modo texto de un adaptador de vídeo CGA. El archivo `cga.pdf` es el manual de referencia de este adaptador. 

Se utilizan los siguientes datos:
- `CRTPORT` es un `#define` con el valor `0x3D4`, que es la **dirección** del puerto de comandos de la controladora de vídeo en el espacio de direcciones de E/S.
- La variable `*crt` es un puntero a la **memoria de video en modo texto**, mapeada en la dirección `0xB8000`. La memoria de vídeo es un arreglo de 4000 bytes que representa una grilla de 80 columnas y 25 filas. Cada carácter requiere 2 bytes, el primero indica el color de la fuente y fondo, mientras que el segundo es el código ASCII.
- La posición del cursor es almacenada por la controladora de vídeo en los registros _Cursor Location High Register_ y _Cursor Location Low Register_ (ver archivo `cga.txt`).

La función `cgaputc()` realiza las siguientes acciones:
1. Obtiene la posición del cursor y lo almacena en la variable `pos`.
3. Si el caracter a mostrar es un `\n` o un `BACKSPACE` actualiza la variable `pos`, que almancena la posición del cursor.
2. En cambio, si el carácter es imprimible, lo escribe en la memoria de vídeo en la posición correspondiente. 
4. Comprueba que la nueva posición del cursor este dentro de los límites de la pantalla.
5. Realiza el _scroll_ (desplazamiento) de la pantalla si corresponde.
6. Actualiza el cursor hardware con la nueva posición.

Responder:
1. Agregar comentarios en el código de la función `cgaputc()` que indiquen si una operación de E/S es mapeada en memoria o utilizar registros especiales de E/S. 
2. Modificar `cgaput.c` para cambiar el color con que el que se imprimen los caracteres por pantalla. Probar también cambiar el color de fondo. Ver la sección *Alphanumeric Modes* (pag. 10) en la documentación CGA (recordar que Intel usa *little-endian*). 

---

¡Fin del Laboratorio!
