/* write_bytes.c
 *
 * Escribe bytes en el archivo indicado.
 *
 * Uso: ./write_bytes archivo num-bytes buffer-size sync
 *
 * Escribe la cantidad de bytes indicada en 'num-bytes' en el archivo
 * 'archivo', usando un buffer de tamaño 'buffer-size'. Si la opción 'sync'
 * es igual a 1, entonces las escrituras son sincrónicas.  
 *
 * Basado en el programa write_bytes.c de Michel Kerrisk.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
    size_t bufSize, numBytes, thisWrite, totWritten;
    char *buf;
    int fd, openFlags;

    if (argc != 5) {
        fprintf(stderr, "Uso: %s archivo numbytes bufsize sync\n", argv[0]);
        fprintf(stderr, "\tarchivo:  archivo donde se escribiran los bytes.\n");
        fprintf(stderr, "\tnumbytes: cantidad de bytes a esribir en el archivo.\n");
        fprintf(stderr, "\tbufsize:  tamaño del buffer a utilizar.\n");
        fprintf(stderr, "\tsync:     1 para hacer todas las escrituras sincrónicas.\n");
        exit(EXIT_FAILURE);
    }

    numBytes = atol(argv[2]);
    bufSize = atol(argv[3]);

    buf = malloc(bufSize);
    if (buf == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    openFlags = O_CREAT | O_WRONLY;

    if (atoi(argv[4]) == 1) {
        openFlags |= O_SYNC;
    }
    
    fd = open(argv[1], openFlags, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror(argv[1]);
        exit(EXIT_FAILURE);
    }

    for (totWritten = 0; totWritten < numBytes; totWritten += thisWrite) {
        thisWrite = bufSize > (numBytes - totWritten) ? (numBytes - totWritten) : bufSize;

        if (write(fd, buf, thisWrite) != thisWrite) {
            fprintf(stderr, "partial/failed write");
        }
    }

    if (close(fd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
