#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#define TOOL_NAME "du-clone"
#define OPTSTRING "x"

/* Possible uses:
 * No arguments: default options in current directory
 * No path: use current directory
 * Arguments: -x eventually (stop traversing when a new FS is detected)
 */

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

    return 0;
}
