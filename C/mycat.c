#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include "samples/prototypes.h"

// ------- DO NOT EDIT ANY CODE IN THIS FILE -------

int main(int argc, char *argv[]) {
    FILE *file_pointer = NULL;

    if (argc == 1) {
        file_pointer = stdin;
    } else {
        file_pointer = fopen(argv[1], "r");
        if (file_pointer == NULL) {
            error(1, errno, "cannot access '%s'", argv[1]);
        }
    }

    char *line = NULL;
    int line_number = 1;
    while ((line = read_line(file_pointer)) != NULL) {
        if (*line != '\0') {
            printf("%6d  %s", line_number++, line);
        }
        printf("\n");
        free(line);
    }
    fclose(file_pointer);
    return 0;
}
