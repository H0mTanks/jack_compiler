#if _WIN32
#include "dirent.h"
#else
#include <dirent.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>
#include <errno.h>
#include "types.h"
#include "common.c"
#include "lex.c"


Internal void tests() {
    common_tests();
}

Internal int is_dir_error() {
    switch (errno) {
        case EACCES:
        case EBADF:
        case EMFILE:
        case ENFILE:
        case ENOENT:
        case ENOMEM:
        case ENOTDIR: {
            return errno;
        }
        default: {
            return 0;
        }
    }
}

int main(int argc, char* argv[]) {
    printf("Starting compiler\n");

    const char* path = NULL;
    if (argc == 2) {
        path = argv[1];
    }
    else if (argc > 2) {
        fatal("Too many arguments supplied");
    }
    else {
        fatal("One argument expected");
    }

    tests();

    DIR* dir = opendir(path);

    if (dir) {
        struct dirent* de = readdir(dir);
        size_t pathlen = strlen(path);
        bool found_valid_jackfile = false;
        for (; de; de = readdir(dir)) {
            const char* ext = get_extension(de->d_name);
            bool is_valid_jackfile = check_jack_extension(ext);

            if (!is_valid_jackfile) {
                continue;
            }

            found_valid_jackfile = true;

            size_t filelen = strlen(de->d_name);
            size_t filepath_len = pathlen + 1 + filelen;
            char* filepath = xcalloc(filepath_len + 1, sizeof(char));
            strcpy(filepath, path);
            if (path[pathlen - 1] != '/') {
                strcat(filepath, "/");
            }
            strcat(filepath, de->d_name);
            printf("filename: %s\n", filepath);

            stream = read_file(filepath);
            printf("%s\n", stream);
        }

        if (!found_valid_jackfile) {
            printf("No .jack file found in directory\n");
        }

        closedir(dir);
    }
    else {
        if (is_dir_error() != ENOTDIR) {
            perror("Error");
            exit(1);
        }

        const char* ext = get_extension(path);
        bool is_valid_jackfile = check_jack_extension(ext);

        if (!is_valid_jackfile) {
            fatal("File is not a .jack file");
        }

        stream = read_file(path);
        printf("%s", stream);
    }
}