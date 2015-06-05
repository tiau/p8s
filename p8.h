#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include "defines.h"
#include "engine.h"
#include "io.h"
#include "ai.h"

#ifndef P8_H
#define P8_H

/* main() return codes */
#define SUCCESS 0
#define BADARGS 1
#define BADPNUM 2
#define BADGNUM 3

int main(int argc, char* argv[]) __attribute__((cold));

#endif /* P8_H */
