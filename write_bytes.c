/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2018.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 13 */

/* write_bytes.c

   Write bytes to a file. (A simple program for file I/O benchmarking.)

   Usage: write_bytes file num-bytes buf-size

   Writes 'num-bytes' bytes to 'file', using a buffer size of 'buf-size'
   for each write().

   If compiled with -DUSE_O_SYNC, open the file with the O_SYNC flag,
   so that all data and metadata changes are flushed to the disk.

   If compiled with -DUSE_FDATASYNC, perform an fdatasync() after each write,
   so that data--and possibly metadata--changes are flushed to the disk.

   If compiled with -DUSE_FSYNC, perform an fsync() after each write, so that
   data and metadata are flushed to the disk.
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

    if (argc != 4) {
        fprintf(stderr, "Uso: %s file num-bytes buf-size\n", argv[0]);
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

#if defined(USE_O_SYNC) && defined(O_SYNC)
    openFlags |= O_SYNC;
#endif

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

#ifdef USE_FSYNC
        if (fsync(fd)) {
            perror("fsync");
            exit(EXIT_FAILURE);
        }
#endif
#ifdef USE_FDATASYNC
        if (fdatasync(fd)) {
            perror("fdatasync");
            exit(EXIT_FAILURE);
        }
#endif
    }

    if (close(fd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
