/*  This file implements a similar version of unic in C.
 *  The major advantage of this function is that it filters out all duplicates.
 */
#include <errno.h>
#include <error.h>
#include <stdio.h>
#include "samples/prototypes.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/* initial estimate of number of uniq lines
 * resize-as-you-go, add in increments of 100
 */
#define ESTIMATE 100

// Define a struct to store information about a line in the file read
// struct stores the string itself and a count of how many times the 
// string appears in the file.
typedef struct entry {
    char *string;
    int count; 
} entry;


// Function prints the number of times n lines occur in a file
void print_uniq_lines(FILE *file_pointer) {
    // Initialize line and array (malloc'd) of entries
    char *line = NULL;
    entry *arr = malloc(ESTIMATE * sizeof(entry));  // array of pointers
    assert(arr);

    // Initialize variables to keep track of process
    int arrSize = ESTIMATE;
    int uniqLineArr = 0;  // number of unique lines in array (ie # of struct elems)
    int flag = 0;  // flag to check when a line already exists in array

    while((line = read_line(file_pointer)) != NULL) {
        // Dynamic memory allocation if array is full
        if (uniqLineArr == arrSize) {
            arrSize += ESTIMATE;
            arr = realloc(arr, (arrSize)*sizeof(entry));
            assert(arr);
        }
        // If line is in array, increase counter
        for (int i = 0; i < uniqLineArr; i++) {
            if (strcmp(line, arr[i].string) == 0) {
                arr[i].count += 1;
                free(line);
                flag = 1;  // trigger flag
                break;
            }
        }
        // Add line to array
        if (flag != 1) {
            arr[uniqLineArr].string = line;  // add entry element to array
            arr[uniqLineArr].count = 1;
            uniqLineArr++;
        }
        flag = 0;  // reset flag
    }
    // Print array elements 
    for (int i = 0; i < uniqLineArr; i++) {
        printf("%7d %s\n", arr[i].count, arr[i].string);
        free(arr[i].string);  // free line
    }
    free(arr);  // free entire array
}


// ------- DO NOT EDIT ANY CODE BELOW THIS LINE (but do add comments!)  -------

// open file and process arguments
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

    print_uniq_lines(file_pointer);
    fclose(file_pointer);
    return 0;
}
