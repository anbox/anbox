#include "getopt.h"

#include <stdio.h>
#include <string.h>

#define  _getprogname() nargv[0]

int opterr = 1;
int optind = 1;
int optopt = 0;
char* optarg;

int getopt(int argc, char* const argv[], const char* ostr) {
    static const char kEmpty[] = "";
    static const char* place = kEmpty;
    if (!*place) {
        if (optind >= argc)
            return -1;

        const char* arg = argv[optind];
        if (arg[0] != '-') {
            // Not an option.
            return -1;
        }
        if (arg[1] == '-' && !arg[2]) {
            // '--' -> end of options.
            return -1;
        }
        if (!arg[1]) {
            // Single '-', If the program wants it, treat it as an option.
            // Otherwise, it's the end of options.
            if (!strchr(ostr, '-')) {
                return -1;
            }
            optopt = '-';
            place = arg + 1;
        } else {
            optopt = arg[1];
            place = arg + 2;
        }
    };

    char* oindex = strchr(ostr, optopt);
    if (!oindex) {
        // Unsupported option.
        (void)fprintf(stderr, "%s: illegal option -- %c\n", argv[0]);
        return '?';
    }
    if (oindex[1] != ':') {
        // No argument needed.
        optarg = NULL;
        if (!*place)
            optind++;
        return optopt;
    }

    // This option needs an argument. Either after the option character,
    // or the argument that follows.
    if (*place) {
        optarg = (char *)place;
    } else if (argc > ++optind) {
        optarg = (char *)argv[optind];
    } else if (oindex[2] == ':') {
        // Optional argument is missing.
        place = kEmpty;
        optarg = NULL;
        return optopt;
    } else {
        // Missing argument.
        place = kEmpty;
        (void)fprintf(stderr, "%s: option requires an argument --%c\n",
                      argv[0], optopt);
        return ':';
    }
    return optopt;
}
