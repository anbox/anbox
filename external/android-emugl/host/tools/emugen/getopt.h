#ifndef GETOPT_H
#define GETOPT_H

#ifdef __cplusplus
extern "C" {
#endif

extern int optind;
extern char* optarg;
extern int optopt;

int getopt(int argc, char* const argv[], const char* ostr);

#ifdef __cplusplus
}
#endif

#endif  // GETOPT_H
