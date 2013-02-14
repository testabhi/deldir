/*
 * deldir.c - Recursively delete a folder, even with a huge number of inodes
 * Copyright (C) 2013 Ayron Jungren
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License ONLY.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _XOPEN_SOURCE 700       /* for lstat */

#include <ctype.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_SIZE 8
#define USAGE_HEADER "Usage: %s [-h] [-q] [-s] [-v] [-y]\n"
#define USAGE_HELP "Try `%s -h' for more information.\n"
#define VERSION "0.2-git"

/* C99 and up supports the `restrict' keyword
 * (see https://en.wikipedia.org/wiki/Restrict) */
#if __STDC_VERSION__ >= 199901L
#define C99_TEXT "with C99 support"
#define _inline inline
#define _restrict restrict
#else
#define C99_TEXT "without C99 support"
#define _inline
#define _restrict
#endif

/* GCC and GCC-compatible compilers support __attribute__ so it needs to be
 * dummied out if the compiler is not GCC-compatible */
#ifndef __GNUC__
#define __attribute__(x)
#endif

/* Function prototypes */
bool confirm_deldir(const char * _restrict path);
bool deldir(const char * _restrict path, const char * _restrict starting_path, bool suppress_warning);
_inline int do_rmdir(const char * _restrict path);
_inline int do_unlink(const char * _restrict path);
char *get_current_path(void);
char *parse_arguments(int argc, char **argv);
void print_current_path(void);
void print_help(char *name) __attribute__((noreturn));
void print_usage(char *name) __attribute__((noreturn));
void print_version(void) __attribute__((noreturn));
int main(int argc, char **argv);

/* Global variables */
bool assume_yes = false, quiet = false, simulate = false;

bool confirm_deldir(const char * _restrict path) {
    char * _restrict buffer;
    int index;

    if(assume_yes) {
        fprintf(stderr, "WARNING: *ALL* files in `%s' will be deleted!\n"
                        "Continuing in five seconds... (Press Ctrl-C to cancel)\n", path);

        for(index = 0; index < 5; index++) {
            fprintf(stderr, ".\a");
            fflush(stderr);
            sleep(1);
        }

        fprintf(stderr, "\n");

        return true;
    }

    if(simulate)
        return true;

    buffer = malloc(BUFFER_SIZE);
    if(buffer == NULL) {
        perror("Could not allocate buffer");
        return false;
    }

    fprintf(stderr, "WARNING: *ALL* files in `%s' will be deleted! Continue?\n"
                    "Type \"yes\" and press ENTER to confirm, or anything else to cancel.\n> ", path);
    fflush(stdin);
    if(fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
        perror("Could not read from stdin");
        free(buffer);
        return false;
    }

    if(strcmp(buffer, "yes\n") == 0) {
        free(buffer);
        return true;
    }
    else {
        puts("Cancelling!");
        free(buffer);
        return false;
    }
}

bool deldir(const char * _restrict path, const char * _restrict starting_path, bool suppress_warning) {
    char * _restrict full_path = NULL;
    DIR * _restrict handle;
    struct dirent * _restrict entry;
    struct stat * _restrict file = NULL;

    if(chdir(path) != 0) {
        perror("Could not change directory");
        goto error;
    }

    file = malloc(sizeof(struct stat));
    if(file == NULL) {
        perror("Could not allocate memory");
        goto error;
    }

    full_path = get_current_path();
    if(full_path == NULL)
        goto error;

    if(!suppress_warning && !confirm_deldir(full_path))
        goto error;

    handle = opendir(full_path);
    if(handle == NULL) {
        perror("Could not open directory");
        goto error;
    }

    while((entry = readdir(handle))) {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        if(lstat(entry->d_name, file) != 0) {
            perror("Could not stat file");
            goto error;
        }

        if(S_ISDIR(file->st_mode)) {
            if(!quiet)
                printf("Entering folder `%s'\n", entry->d_name);
            if(!deldir(entry->d_name, full_path, true)) {
                /* There isn't an error to print with perror as the child deldir
                 * already called it. */
                goto error;
            }
            if(do_rmdir(entry->d_name) != 0) {
                perror("Could not delete directory");
                goto error;
            }
        }
        else {
            if(!quiet)
                printf("Deleting `%s'...", entry->d_name);
            if(do_unlink(entry->d_name) != 0) {
                if(!quiet)
                    puts(" FAILED");
                perror("Could not unlink file");
                goto error;
            }
            else if(!quiet)
                puts(" OK");
        }
    }

    closedir(handle);

    if(starting_path == NULL) {
        if(do_rmdir(full_path) != 0) {
            perror("Could not delete directory");
            goto error;
        }
    }
    else if(chdir(starting_path) != 0) {
        perror("Could not change directory");
        goto error;
    }

    free(file);
    free(full_path);
    return true;

    error:
    free(file);
    free(full_path);

    return false;
}

_inline int do_rmdir(const char * _restrict path) {
    if(simulate) {
        printf("rmdir(\"%s\") in ", path);
        print_current_path();
        return 0;
    }
    else
        return rmdir(path);
}

_inline int do_unlink(const char * _restrict path) {
    if(simulate) {
        printf("unlink(\"%s\") in ", path);
        print_current_path();
        return 0;
    }
    else
        return unlink(path);
}

char *get_current_path(void) {
    char *path;

    path = getcwd(NULL, 0);
    if(path == NULL) {
        perror("Could not get current path");
        return NULL;
    }

    return path;
}

char *parse_arguments(int argc, char **argv) {
    int option;

    while((option = getopt(argc, argv, "hqsvy")) != -1) {
        switch(option) {
            case 'h':
                print_help(argv[0]);
                break;
            case 'q':
                quiet = true;
                break;
            case 's':
                quiet = true;
                simulate = true;
                break;
            case 'v':
                print_version();
                break;
            case 'y':
                assume_yes = true;
                break;
            default:
                fprintf(stderr, USAGE_HELP, argv[0]);
                return NULL;
        }
    }

    if(optind == argc-1)
        return argv[optind];
    else
        print_usage(argv[0]);

#ifndef __GNUC__
    /* Make the compiler happy (not needed on GCC-compatible compilers as
     * the noreturn attribute on print_usage will prevent the warning) */
    return NULL;
#endif
}

void print_current_path(void) {
    char *path = get_current_path();

    if(path == NULL)
        return;

    puts(path);
    free(path);
}

void print_help(char *name) {
    fprintf(stderr, USAGE_HEADER, name);
    /* These strings are split up because C90 only requires compilers to support
     * 509 character long strings */
    fputs("Recursively delete a folder, even with a huge number of inodes\n"
          "Should work even when `rm' and other UNIX tools do not.\n"
          "\n"
          "  -h                         print this help and exit\n"
          "  -q                         do not print files and folders as they are deleted\n"
          "  -s                         simulate deleting files and folders, do not actually delete\n"
          "  -v                         print the version of deldir and exit\n"
          "  -y                         assume the confirmation to delete is answered with `yes'\n"
          , stderr);
    fputs("deldir home page: <http://0x3b.com/projects/deldir>\n"
          "Report deldir bugs at <http://0x3b.com/projects/deldir/issues>\n"
          "deldir is licensed under the GNU General Public License (GPL) version 3 ONLY.\n"
          , stderr);
    fprintf(stderr, "Run `%s -v' for more information about licensing.\n", name);
    exit(EXIT_FAILURE);
}

void print_usage(char *name) {
    fprintf(stderr, USAGE_HEADER, name);
    fprintf(stderr, USAGE_HELP, name);
    exit(EXIT_FAILURE);
}

void print_version(void) {
    fputs("deldir " VERSION "\n"
          "Built " C99_TEXT "\n"
          "Copyright (C) 2013 Ayron Jungren\n"
          "License GPLv3: GNU GPL version 3 ONLY <https://gnu.org/licenses/gpl.html>.\n"
          "This is free software: you are free to change and redistribute it.\n"
          "There is NO WARRANTY, to the extent permitted by law.\n"
          , stderr);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    char *path = parse_arguments(argc, argv);

    if(path == NULL)
        return EXIT_FAILURE;

    if(deldir(path, NULL, false))
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}

