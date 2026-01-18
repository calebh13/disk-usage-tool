#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include "stack.h"

#define TOOL_NAME "du-clone"
#define OPTSTRING "x"

/* Possible uses:
 * No arguments: default options in current directory
 * No path: use current directory
 * Arguments: -x eventually (stop traversing when a new FS is detected)
 */

void traverseDirectory(char* target_directory); // TODO add option flags

int main(int argc, char* argv[])
{
    char* target_directory = ".";
    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1) {
        switch(c) {
            case 'x':
                // Handle this later: like du
                printf("-x option detected\n");
                break;
            default:
                fprintf(stderr, "Usage: %s [OPTION] [DIRECTORY]", TOOL_NAME);
                fprintf(stderr, "Arguments:\n-x\n\tskip directories on different file systems");
                exit(EXIT_FAILURE);
                break;
        }
    }

    // If there are 3 arguments, optind will be at 2 when about to parse dirname
    // We're just going to ignore everything afterwards
    if (optind < argc) {
        target_directory = argv[optind++];
        if (optind < argc) {
            fprintf(stderr, "%s: error: too many arguments\n", TOOL_NAME);
            exit(EXIT_FAILURE);
        }
    }

    // Now we actually do the traversal!
    printf("Target directory: %s\n", target_directory);

    // General idea: we're going to append the `direntry` pointers to a stack so we can traverse with DFS
    // We do this with `opendir` on the root, then `readdir` repeatedly until we get a null pointer.
    // Then as we traverse, add the result info to another stack, keeping track of usage (like du, usage is the sum of its children + itself)
    // How do we want to store the info? TBD.

    traverseDirectory(target_directory); // todo add option flags
    
    return 0;
}

struct dir_data {
    char path[PATH_MAX];
    unsigned long size; // in KB
};

void traverseDirectory(char* target_directory) // TODO add option flags
{
    /* We'll be iterating on path strings since they need to be displayed anyway
     * Need to make a custom dir_entry struct that stores OUR relevant metadata
     * I.e.: path string, size
     *
     * Opendir function works on paths.
     * General idea is to iterate through the `dirent`s with readdir(), and:
     * - If directory, push it to the stack
     * - If file, add its size to the parent's size
     * Also, this does mean we'll have to construct a string each time :(
     * Luckily we know the size ahead of time: PATH_MAX = 4096 bytes on Linux
     */

    struct stack s;
    stack_alloc(&s);
    struct dir_data* root_data = malloc(sizeof(struct dir_data));
    strncpy(root_data->path, target_directory, PATH_MAX);
    root_data->size = 0; // I think we should add this later?

    stack_push(&s, (void*)root_data);

    while (!stack_empty(&s)) {
        root_data = stack_pop(&s);

        DIR* dir = opendir(root_data->path);
        if (!dir) {
            fprintf(stderr, "Failed opening directory %s\n", root_data->path);
            free(root_data);
            continue;
        }

        // readdir() moves through `dir` like an iterator
        // this is nice, since we don't have to keep track of where we are
        struct dirent* ent;
        while ((ent = readdir(dir))) {
            // skip over . and .. directories
            if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) {
                continue;
            }

            // Now we build the full path to `lstat` it
            // TODO: do we malloc this, so we can add it to the results stack?
            char path[PATH_MAX]; // from <linux/limits.h>
            snprintf(path, PATH_MAX, "%s%s", root_data->path, ent->d_name);

        }
    }

    // How does du do it?
    // I think they print when they free something

}
