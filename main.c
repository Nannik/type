#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define BUF_SIZE 10000

char *ANSI_RESET = "\033[0m";
char *ANSI_BG_GREEN = "\033[42m";

int run_time = 30;
int run_words = 10;
int repeat = 0;
char *dictionary_filename = "dictionary";

FILE *f;
char words[BUF_SIZE][100];
int words_c = 0;

void handle_signal (int sig) {
  if (f) fclose(f);
  exit(EXIT_FAILURE);
}

void print_usage () {
  fprintf(stderr, "Usage: [-twr] [preset]\n");
}

void config(int argc, char **argv) {
  int opt;
  char *end;

  while ((opt = getopt(argc, argv, "t:w:r")) != -1) {
    switch (opt) {
    case 't':
      run_time = strtol(optarg, &end, 10);

      if (*end != '\0') {
        fprintf(stderr, "Option(-t) parsing error: Invalid characters detected\n");
        print_usage();
        exit(EXIT_FAILURE);
      }
      break;
    case 'w':
      run_words = strtol(optarg, &end, 10);

      if (*end != '\0') {
        fprintf(stderr, "Option(-w) parsing error: Invalid characters detected\n");
        print_usage();
        exit(EXIT_FAILURE);
      }
      break;
    case 'r':
      run_words = strtol(optarg, &end, 10);

      if (*end != '\0') {
        fprintf(stderr, "Option(-r) parsing error: Invalid characters detected\n");
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
    dictionary_filename = argv[optind];
  }
}

void pick (char *res) {
  strcpy(res, words[rand() % words_c]);
  strcat(res, " ");
}

void read_words () {
  char word[100];

  while (fscanf(f, "%99s", word) == 1 && words_c < BUF_SIZE) {
    strcpy(words[words_c++], word);
  };
}

int main(int argc, char **argv) {
  config(argc, argv);
  srand(time(NULL));

  char str[1024];
  str[0] = '\0';

  f = fopen(dictionary_filename, "r");

  if (!f) {
    fprintf(stderr, "File %s not found", dictionary_filename);
    exit(EXIT_FAILURE);
  }

  signal(SIGTERM, handle_signal);
  signal(SIGKILL, handle_signal);

  read_words();

  char *picked_word;
  for (int i = 0; i < run_words; i++) {
    pick(picked_word);
    strcat(str, picked_word);
  }

  printf("%s\n", str);

  fclose(f);
}
