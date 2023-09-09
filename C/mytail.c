/*  Own version of tail command in C.
 */

#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include "samples/prototypes.h"

#define MAX_NUM_LINES 100000
#define DEFAULT_NUM_LINES 10
#define NUMERIC_ARG_BASE 10

/*  Function prints the last n characters in a file (similar to mytail).
 *  It takes pointer to a file and a number of elements to read. If number of 
 *  elements in file is less than number of elements to be read (n), function
 *  prints entire file, line by line, enumerated by line number.
 */
void print_last_n(FILE *file_pointer, int n) {
    char *line = NULL;
    char *arr[n];  // array size n
    int index = 0;
    int counter = 0;  // # lines read
    int complete = 0;  // flag triggers when array is full of elements

    while ((line = read_line(file_pointer)) != NULL) {
        // If array is full of elements
        if (complete == 1) {
            free(arr[index]);  // free line at index in array
        } 
        arr[index] = line;  // add current line to arr
        index++;
        counter++;
        
        // Array full: reset index and trigger flag
        if (index == n) {
            index = 0;
            complete = 1;  // flag to complete
         }
    }
    // If lines read is less than -n-, print all lines
    // Note: this is inefficient, as two segments of code are almost equivalent,
    // but was implemented this way to prevent a late submission and to handle an
    // "edge case"
    if (counter < n) {
        for (int i = 0; i < counter; i++) {
            // If index at last elem, reset to start of array
            if (index == counter) {
                index = 0;
            }
            printf("%s\n", arr[index]);
            free(arr[index]);  // free memory
            index++;
        }
    // Else, print -n- lines in order
    } else {
        for (int i = 0; i < n; i ++) {
            printf("%s\n", arr[index]);
            free(arr[index]);  // free memory
            index++;
            if (index == n) {
                index = 0;  // reset index
            }
        } 
    }
}

// ------- DO NOT EDIT ANY CODE BELOW THIS LINE (but do add comments!)  -------

// convert arguments
int convert_arg(const char *str, int max) {
    char *end = NULL;
    long parsed_number = strtol(str, &end, NUMERIC_ARG_BASE);
    if (*end != '\0') {
        error(1, 0, "Invalid number '%s'", str);
    }
    if (parsed_number < 1 || parsed_number > max) {
        error(1, 0, "%s is not within the acceptable range [%d, %d]", str, 1, max);
    }
    return parsed_number;
}

// handle arguments and open file
int main(int argc, char *argv[]) {
    int num_lines = DEFAULT_NUM_LINES;

    if (argc > 1 && argv[1][0] == '-') {
        num_lines = convert_arg(argv[1] + 1, MAX_NUM_LINES);
        argv++;
        argc--;
    }

    FILE *file_pointer = NULL;
    if (argc == 1) {
        file_pointer = stdin;
    } else {
        file_pointer = fopen(argv[1], "r");
        if (file_pointer == NULL) {
            error(1, errno, "cannot access '%s'", argv[1]);
        }
    }

    print_last_n(file_pointer, num_lines);
    fclose(file_pointer);
    return 0;
}
