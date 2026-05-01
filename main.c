#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "args.h"
#include "term.h"
#include "typetest.h"

FILE *f;

void handle_signal (int sig) {
  if (f) fclose(f);
  disable_raw_mode();
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  srand(time(NULL));
  signal(SIGTERM, handle_signal);
  signal(SIGKILL, handle_signal);

  configure(argc, argv);

  f = fopen(config.dictionary_filepath, "r");
  if (!f) {
    fprintf(stderr, "File %s not found", config.dictionary_filepath);
    exit(EXIT_FAILURE);
  }

  read_dictionary(f);
  fclose(f);

  start_test();

  write(STDOUT_FILENO, "\n", 1);
}
