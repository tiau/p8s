#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include "engine.h"
#include "ai.h"

#ifndef P8_H
#define P8_H

/* main() return codes */
#define SUCCESS 0
#define BADARGS 1
#define BADPNUM 2
#define BADGNUM 3

/* These are global so our signal handler can get at them */
static size_t nplayers = 0, ngames = 1, offset = 0;
static size_t* victories = 0;

int main(int argc, char* argv[]);

#endif /* P8_H */
