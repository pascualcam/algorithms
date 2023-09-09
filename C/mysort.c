/* Author: Pascual E. Camacho
 * SUID: ecam
 * CS 107
 *
 * This file contains a modified sort function
 * that sorts files based on a set of user specified
 * filters:
 *  -l sort by increasing line length
 *  -n sort by numerical values
 *  -r sort reversed
 *  -u sort and discard duplicate lines
 * Function handles single filters or a combination of
 * them.
 * When -u flag is used, this function calls binsert,
 * function implemented in util.c
 */

#include <errno.h>
#include <error.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "samples/prototypes.h"

#define MAX_LINE_LEN 4096
#define MIN_NLINES 100

/* \description - sorts contents of file by specified instruction and
 *                can filter duplicates. Values can be printed in an 
 *                inverse faction.
 * \parameters - FILE *fp - pointer to file
 *               cmp_fn_t cmp - function to be used to compare of file elements
 *               bool uniq - flag that indicates if duplicates are to be filtered
 *               bool reverse - flag that indicates if values should be printed
 *                  in reverse order.
 * \return - void
 */

typedef int (*cmp_fn_t)(const void *p, const void *q);

// Function to compare strings lexicographically
int cmp_pstr(const void *p, const void *q) {
    return strcmp(*(char **)p, *(char **)q);
}

// Function to compare by string length
int cmp_pstr_len(const void *p, const void *q) {
    int len_first = strlen(*(char **)p);
    int len_second = strlen(*(char **)q);
    return len_first - len_second;
}

// Function to compare strings numerically
int cmp_pstr_numeric(const void *p, const void *q) {
    int first_num = atoi(*(char **)p);
    int second_num = atoi(*(char **)q);
    return first_num - second_num;
}

// Sort by filter
void sort_lines(FILE *fp, cmp_fn_t cmp, bool uniq, bool reverse) {
    // Initialize counter of lines read
    size_t lines_read = 0;
    // Factor to double memory after we have exhausted the current allocation
    size_t arr_size = MIN_NLINES;
    // Heap allocate space in array to store strings
    char **arr = malloc(MIN_NLINES * sizeof(char *));
    assert(arr);
    // Stack allocate the maximum possible length to store line
    char line[MAX_LINE_LEN];
    
    // Read line by line in file
    while (fgets(line, MAX_LINE_LEN, fp)) {
        // Check memory and reallocate if needed
        if (lines_read == arr_size) {
            arr_size += MIN_NLINES;  // double array size/memory
            arr = realloc(arr, arr_size * sizeof(char *));  // reallocate array
            assert(arr);
        }
        // Check flag to remove duplicate lines
        if (uniq) {
            // create a temporary line to check if duplicate
            char *temp = line;
            // get line from binsert program ()
            char **b_line = binsert(&temp, arr, &lines_read, sizeof(char *), cmp);
            
            // check if duplicate
            if (temp == *b_line) {
                // heap allocate a copy
                *b_line = strdup(line);
                assert(*b_line);
            }
        }
         else {
            // heap allocate a copy
            char *newl = strdup(line);
            assert(newl);
            // store new line in array
            arr[lines_read] = newl;
            lines_read++;  // increase counter 
        }
    }
    // Sort final array if uniq flag is off (uniq already sorted by binsert)
    if (!uniq) {
        // sort array
        qsort(arr, lines_read, sizeof(char *), cmp);
    }
    // Print array
    for (int i = 0; i < lines_read; i++) {
        // Check reverse flag to decide print order
        // Print in reverse order
        if (reverse) {
            int end_index = lines_read - 1;
            printf("%s", arr[end_index - i]);
            free(arr[end_index - i]);  // free memory
        } else {  // print in normal order
            printf("%s", arr[i]);
            free(arr[i]);  // free memory
        }
    }
    free(arr);  // free entire array
}

// ------- DO NOT EDIT ANY CODE BELOW THIS LINE (but do add comments!)  -------

int main(int argc, char *argv[]) {
    cmp_fn_t cmp = cmp_pstr;
    bool uniq = false;
    bool reverse = false;

    int opt = getopt(argc, argv, "lnru");
    while (opt != -1) {
        if (opt == 'l') {
            cmp = cmp_pstr_len;
        } else if (opt == 'n') {
            cmp = cmp_pstr_numeric;
        } else if (opt == 'r') {
            reverse = true;
        } else if (opt == 'u') {
            uniq = true;
        } else {
            return 1;
        }

        opt = getopt(argc, argv, "lnru");
    }

    FILE *fp = stdin;
    if (optind < argc) {
        fp = fopen(argv[optind], "r");
        if (fp == NULL) {
            error(1, 0, "cannot access %s", argv[optind]);
        }
    }
    sort_lines(fp, cmp, uniq, reverse);
    fclose(fp);
    return 0;
}
