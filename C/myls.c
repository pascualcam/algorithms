/* Author: Pascual E. Camacho
 * SUID: ecam
 * CS 107
 */

/*  This file contains the list function (ls) implemented
 *  in a simplified fashion. The function filters files 
 *  based on user input, therefore, user can use flags to 
 *  specify the function's behavior. Specifically, the user 
 *  can sort by name, sort by type, filter hidden files or 
 *  show all files. 
 *  Function dynamically allocates memory.
 */

#include <dirent.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <error.h>
#include <string.h>
#include <stdlib.h>

/*  \description - Simplified list function that filters and
 *                  sorts based on user specification. Dynamically
 *                  allocates and frees memory.
 *  \parameters - const char *dirpath - path to directory to list
 *                int filter - specifies type of filter to apply
 *                int order - specifies order type
 *  \return - void
 */

enum { SORT_BY_NAME, SORT_BY_TYPE };
enum { EXCLUDE_DOT, INCLUDE_DOT };

/* Define comparison functions
 */
typedef int (*cmp_fn_t)(const struct dirent **, const struct dirent **);
typedef int (*filter_fn_t)(const struct dirent *);

/* This fully implemented function returns whether the dirent pointed to by
 * the given pointer represents a directory.  (Note: on the myth filesystem,
 * the only file type information that is accurate is directory/not-directory
 * used here. Other type info in struct dirent is not reliable).
 */
bool is_dir(const struct dirent *dirptr) {
    return dirptr->d_type == DT_DIR;
}

/*  Function to filter hidden files
 */
int filterDot(const struct dirent *entry) {
    const char dot = '.';
    //return dot != entry->d_name[0];
    if (entry->d_name[0] == dot) {
        return 0; 
    }
    return 1;
}

/*  Comparison function to sort by name, lexicograpically
 */
int cmpByName(const struct dirent **first, const struct dirent **second) {
    return strcmp((*first)->d_name, (*second)->d_name);
}

/*  Comparison function to sorty by type (directory or not)
 */
int cmpByType(const struct dirent **first, const struct dirent **second) {
    if (is_dir(*first) == is_dir(*second)) {
        return cmpByName(first, second);
    } else if (is_dir(*first)){
        return -1;
    } else {
        return 1;
    }
}

/* List function implemented as specified in description.
 */
void ls(const char *dirpath, int filter, int order) {
    struct dirent **namelist;
    cmp_fn_t cmp = cmpByName;
    filter_fn_t fltr = filterDot;
    int n;
     
    // check flag for filter
    if (filter == INCLUDE_DOT) {
        fltr = NULL;
    }
    // check flag for comparison function
    if (order == SORT_BY_TYPE) {
        cmp = cmpByType;
    }
    // scan directory
    n = scandir(dirpath, &namelist, fltr, cmp);
    // check for return value error
    if (n == -1) {
        error(0, 0, "cannot access %s\n", dirpath);
        return;
    }
    // print list
    for (int i = 0; i < n; i++) {
        printf("%s", namelist[i]->d_name);
        // if dictionary, add trailing slash
        if (is_dir(namelist[i])) {
            printf("/");
        }
        // move to next line
        printf("\n");
        // free struct dirent at i
        free(namelist[i]);
    }
    // free struct dirent list
    free(namelist);
}

// ------- DO NOT EDIT ANY CODE BELOW THIS LINE (but do add comments!)  -------
    
int main(int argc, char *argv[]) {
    int order = SORT_BY_NAME;
    int filter = EXCLUDE_DOT;

    int opt = getopt(argc, argv, "az");
    // Define filter and ordering to apply
    while (opt != -1) {
        if (opt == 'a') {
            filter = INCLUDE_DOT;
        } else if (opt == 'z') {
            order = SORT_BY_TYPE;
        } else {
            return 1;
        }

        opt = getopt(argc, argv, "az");
    }
    
    if (optind < argc - 1) {
        for (int i = optind; i < argc; i++) {
            printf("%s:\n", argv[i]);
            ls(argv[i], filter, order);
            printf("\n");
        }
    } else {
        ls(optind == argc - 1 ? argv[optind] : ".", filter, order);
    }
    
    return 0;
}
