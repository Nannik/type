#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <strings.h>

#include "args.h"

Settings config = {
  .wordc = 10,
  .time = 0,
  .dictionary_filepath = "dictionary"
};

void print_usage () {
  fprintf(stderr, "Usage: [-twr] [preset]\n");
}

void configure(int argc, char **argv) {
  int opt;
  char *end;

  while ((opt = getopt(argc, argv, "t:w:r")) != -1) {
    switch (opt) {
    case 't':
      config.time = strtol(optarg, &end, 10);

      if (*end != '\0') {
        fprintf(stderr, "Option(-t) parsing error: Invalid characters detected\n");
        print_usage();
        exit(EXIT_FAILURE);
      }
      break;
    case 'w':
      config.wordc = strtol(optarg, &end, 10);

      if (*end != '\0') {
        fprintf(stderr, "Option(-w) parsing error: Invalid characters detected\n");
        print_usage();
        exit(EXIT_FAILURE);
      }
      break;
    default:
      print_usage();
      exit(EXIT_FAILURE);
    }
  }

  if (argc > optind) {
    config.dictionary_filepath = argv[optind];
  }
}
