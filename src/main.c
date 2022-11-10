#if _WIN32
#include "vendor/dirent.h"
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


Internal void tests(void) {
    common_tests();
    lex_tests();
    printf("tests complete\n");
}

Internal int is_dir_error(void) {
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

    //tests();

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

            char* filepath = NULL;
            BUF_PRINTF(filepath, path);
            if (path[pathlen - 1] != '/') {
                BUF_PRINTF(filepath, "/");
            }

            char* out_filepath = xcalloc(pathlen + 1 + (ext - de->d_name) + 1 + strlen("xml") + 1, sizeof(char));
            strcpy(out_filepath, filepath);

            BUF_PRINTF(filepath, de->d_name);
            // printf("filepath: %s\n", filepath);

            const char* filestream = read_file(filepath);
            printf("%s\n", filestream);

            lex(filestream);

            //TODO: change output filename to `filenameT.xml`
            strncat(out_filepath, de->d_name, ext - de->d_name - 1);
            strcat(out_filepath, "TT.");
            strcat(out_filepath, "xml");
            printf("filename: %s\n", out_filepath);

            write_file(out_filepath, file_buf, BUF_LEN(file_buf));

            free(out_filepath);
            BUF_FREE(filepath);
            BUF_FREE(file_buf);
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

        const char* filestream = read_file(path);
        printf("%s", filestream);
        lex(filestream);

        char* out_filepath = xcalloc(ext - path + 1 + strlen("xml") + 1, sizeof(char));
        strncpy(out_filepath, path, ext - path - 1);
        strcat(out_filepath, "TT.");
        strcat(out_filepath, "xml");
        printf("filename: %s\n", out_filepath);
        write_file(out_filepath, file_buf, BUF_LEN(file_buf));

        free(out_filepath);
        BUF_FREE(file_buf);
    }
}