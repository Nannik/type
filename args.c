#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <strings.h>

#include "args.h"


void print_usage();


Settings config = {
  .wordc = 0,
  .time = 0,
  .dictionary_filepath = "dictionary",
  .profile = "default",
  .exit = false,
  .output = false
};

void configure(int argc, char **argv) {
  int opt;
  char *end;

  while ((opt = getopt(argc, argv, "t:w:p:oh")) != -1) {
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
    case 'p':
      config.profile = optarg;
      break;
    case 'o':
      config.output = true;
      config.exit = true;
      break;
    default:
      print_usage();
      config.exit = true;
    }
  }

  if (config.wordc == 0 && config.time == 0) {
    config.wordc = 30;
  }

  if (argc > optind) {
    config.dictionary_filepath = argv[optind];
  }
}


void print_usage(void) {
    fprintf(stderr,
        "Usage: program [options] [dictionary]\n"
        "\n"
        "Options:\n"
        "  -t  Run time in seconds (0 = no timer, default: 0)\n"
        "  -w  Number of words (default: 30)\n"
        "  -r  Profile name (CSV file for typing runs, default: default)\n"
        "  -o  Write profile data to standard output\n"
        "  -h  Show this help message\n"
        "\n"
        "Arguments:\n"
        "  dictionary  Path to dictionary file\n"
    );
}

