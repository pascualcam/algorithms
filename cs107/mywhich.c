/*  This file mimics the functionality of the C program 'which'.
 *  It can receive a specific exectuable file as input and will return
 *  the path to the location of that file. If no path is provided by user,
 *  the program uses the current environment path.
 */

#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "samples/prototypes.h"

#define SLASH "/"



/*  Function takes a directory and appends the name of an executable
 *  at the end.
 */
void appendToPath(char *dir, char *exe) {
    strcat(dir, SLASH);  // slash before executable name
    strcat(dir, exe);
}

int main(int argc, char *argv[], const char *envp[]) {
    // Get value of environment variable
    const char *searchpath = get_env_value(envp, "MYPATH");
    char dir[PATH_MAX];
    
    // If MYPATH is not an environment variable, use default PATH
    if (searchpath == NULL) {
        searchpath = get_env_value(envp, "PATH");
    }
    
    // If no input from the user, print all contents in path
    if (argc == 1) {
        const char *remaining = searchpath;
        printf("%s\n", "Directories in search path:"); 
        while (scan_token(&remaining, ":", dir, sizeof(dir))) {
            printf("%s\n", dir);
        }

    // Find location of user input
    } else {
        // Want to start i=1 (argv[1]), since argv[0] is location of this file
        for (int i = 1; i < argc; i++) {
            const char *remaining = searchpath;  // path gets reset for each argument

            // Loop through all paths in remaining (searchpath) 
            while (scan_token(&remaining, ":", dir, sizeof(dir))) {
                
                appendToPath(dir, argv[i]);  // append argument to path
                
                // Print path if argument exists, is readable, and access is permitted
                if (access(dir, (R_OK | X_OK)) == 0) {
                    printf("%s\n", dir);
                    break;  // move to next argument
                }
            }
        } 
    }
    return 0;
}
