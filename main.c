#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>

#define BUF_SIZE 10000
#define WORD_SIZE 100

#define ANSI_RESET "\033[0m"
#define ANSI_FG_GREEN "\033[32m"
#define ANSI_FG_RED "\033[31m"
#define ANSI_CLEAR "\033[2J"
#define ANSI_MOVE_RC(R, C) "\033[" #R ";" #C "H"
#define ANSI_MOVE_LEFT(C) "\033[" #C "D"

int run_time = 30;
int run_words = 10;
int repeat = 0;
char *dictionary_filename = "dictionary";

FILE *f;
char words[BUF_SIZE][WORD_SIZE];
int words_c = 0;

struct termios orig_term;
void disable_raw_mode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_term);
}

void enable_raw_mode() {
  tcgetattr(STDIN_FILENO, &orig_term);
  atexit(disable_raw_mode);

  struct termios raw = orig_term;
  raw.c_lflag &= ~(ICANON | ECHO);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void handle_signal (int sig) {
  if (f) fclose(f);
  disable_raw_mode();
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
  char word[WORD_SIZE];

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

  char picked_word[WORD_SIZE];
  for (int i = 0; i < run_words; i++) {
    pick(picked_word);
    strcat(str, picked_word);
  }

  int run_len = strlen(str) - 1;
  str[run_len] = '\0';

  fclose(f);

  enable_raw_mode();

  char buf[1024 + 500];
  int len = snprintf(
    buf,
    sizeof(buf),
    ANSI_CLEAR ANSI_MOVE_RC(0, 0) "%s" ANSI_MOVE_RC(0, 0),
    str
  );

  write(STDOUT_FILENO, buf, len);

  char c;
  int pos = 0;
  while (read(STDIN_FILENO, &c, 1) == 1) {
    // write(STDOUT_FILENO, "a", 1);
    if (c == 127) {
      pos--;

      char s[10];
      sprintf(s, ANSI_MOVE_LEFT(1) "%c" ANSI_MOVE_LEFT(1), str[pos]);
      write(STDOUT_FILENO, s, 10);

      if (pos < 0) pos = 0;
    } else if (
      (c >= 'a' && c <= 'z') || 
      (c >= 'A' && c <= 'Z') ||
      isspace(c)
    ) {
      char s[11];
      if (c == str[pos]) {
        sprintf(s, ANSI_FG_GREEN "%c" ANSI_RESET, str[pos]);
      } else {
        sprintf(s, ANSI_FG_RED "%c" ANSI_RESET, str[pos]);
      }
      write(STDOUT_FILENO, s, 11);

      if (pos == run_len - 1) {
        break;
      }

      pos++;
    }
  }

  write(STDOUT_FILENO, "\n", 1);

  disable_raw_mode();
}
