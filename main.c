#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>

#include "vec.h"

#define TOOL_NAME "du-clone"
#define OPTSTRING "x"

/* Possible uses:
 * No arguments: default options in current directory
 * No path: use current directory
 * Arguments: -x eventually (stop traversing when a new FS is detected)
 */


 /* TODO:
  * Implement hashmap to avoid overcounting hard links
  * Figure out nice way to print block size
  * Support more options (-x, -h, etc)
  * Make shell-based tests (pipe du and clone output to files, sort them, diff them)
  * more?
 */

int traverseDirectory(char* target_directory); // TODO add option flags

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
    // printf("Target directory: %s\n", target_directory);
    return traverseDirectory(target_directory); // todo add option flags
}

struct file_data {
    char path[PATH_MAX];
    DIR* dir; // directory iterator: lets us resume where we left off
    unsigned long long size; // CRUCIAL: these are 512 BYTE blocks for name, rename later
    // These 2 values uniquely identify the file in the FS
    // dev_t st_dev;
    // ino_t st_ino;
};

// For hashmap, probably hash st_dev XOR st_ino

DECL_VECTOR(struct file_data, file_data_vec)

// TODO: add error checking on all vector operations, and return error codes
int traverseDirectory(char* target_directory) // TODO add option flags
{
    /*
     * The general strategy here is to do a postorder traversal of the directory,
     * using an explicit stack for recursion to avoid stack overflows.
     * The `readdir()` function is crucial here: it keeps track of which child is next,
     * so we don't lose progress after pushing/popping from the stack.
     * All the relevant data is stored in a `file_data` struct in the `file_data_vec` stack.
     * See vec.h for the relevant macro.
    */

    file_data_vec s;
    file_data_vec_init(&s, NULL);

    // init hashmap here (use khashl)

    struct file_data root_data;
    strncpy(root_data.path, target_directory, PATH_MAX - 1);
    root_data.path[PATH_MAX - 1] = '\0'; // strncpy may not null-terminate
    root_data.size = 0; 
    root_data.dir = NULL;

    file_data_vec_push(&s, root_data);
    
    // Everything past this point COULD be moved into its own function: only dependence is the vec.
    struct file_data* parent_data = NULL;

    while (s.len > 0) {
        parent_data = file_data_vec_get(&s, s.len - 1);
        struct stat stat_buf = {0};
        if (!parent_data->dir) {
            // printf("opening %s\n", root_data->path);
            parent_data->dir = opendir(parent_data->path);
        }
        if (!parent_data->dir) {
            fprintf(stderr, "Failed opening directory %s\n", parent_data->path);
            free(parent_data);
            file_data_vec_pop(&s);
            continue;
        }

        // readdir() moves through `dir` like an iterator
        // this is nice, since we don't have to keep track of where we are
        struct dirent* ent;
        struct file_data child_data = {0};
        bool found_subdir = false;
        while ((ent = readdir(parent_data->dir))) {
            // skip over . and .. directories
            if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) {
                continue;
            }

            snprintf(child_data.path, PATH_MAX - 1, "%s/%s", parent_data->path, ent->d_name);
            lstat(child_data.path, &stat_buf);
            
            // TODO add (st_dev, st_ino) to hashset to avoid overcounting hardlinks
            if (S_ISREG(stat_buf.st_mode)) {
                // If regular file, add size immediately. Files are leaf nodes, never any children
                parent_data->size += stat_buf.st_blocks;
                printf("%llu\t%s\n", (unsigned long long)stat_buf.st_blocks, child_data.path);
            } else {
                // Otherwise, it's a directory, and we'll add size when we pop it
                if (file_data_vec_push(&s, child_data)) {
                    printf("Error: Could not reallocate vector\n");
                    return -1;
                }
                found_subdir = true;
                break; // postorder: read subdirectory first
            }
        }
        if (!found_subdir) {
            // We're now ready to add the size of the dir itself and pop it from stack
            lstat(parent_data->path, &stat_buf);
            parent_data->size += stat_buf.st_blocks;
            printf("%llu\t%s\n", parent_data->size, parent_data->path);
            // now add its size to the new top of the stack (its parent) if it exists
            // i.e., if this is not the root node
            if (file_data_vec_pop(&s)) {
                printf("Error: Tried to pop from empty vector\n");
                return -1;
            }
            if (s.len > 0) {
                file_data_vec_get(&s, s.len - 1)->size += parent_data->size;
            }
        }
    }
    file_data_vec_free(&s);
    return 0;
}
