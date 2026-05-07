#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "typetest.h"
#include "args.h"
#include "term.h"

#define WORDS_BUF_SIZE 10000
#define WORD_SIZE 100

int words_c = 0;
char words[WORDS_BUF_SIZE][WORD_SIZE];
void read_dictionary (FILE *f) {
  char word[WORD_SIZE];

  while (fscanf(f, "%99s", word) == 1 && words_c < WORDS_BUF_SIZE) {
    strcpy(words[words_c++], word);
  };
}

void pick (char *res) {
  strcpy(res, words[rand() % words_c]);
  strcat(res, " ");
}

void start_test() {
  char test[TYPETEST_BUF_SIZE];
  test[0] = '\0';

  char picked_word[WORD_SIZE];
  for (int i = 0; i < config.words_count; i++) {
    pick(picked_word);
    strcat(test, picked_word);
  }

  int test_len = strlen(test) - 1;
  test[test_len] = '\0';

  enable_raw_mode();

  term_feed_str(test);
  term_write_str();

  char c;
  int pos = 0;
  while (read(STDIN_FILENO, &c, 1) == 1) {
    if (c == 127) {
      pos = term_send_backspace(test[pos - 1]);
    } else if (
      (c >= 'a' && c <= 'z') || 
      (c >= 'A' && c <= 'Z') ||
      isspace(c)
    ) {
      char s[11];
      if (c == test[pos]) {
        pos = term_send_char(test[pos], true);
      } else {
        pos = term_send_char(test[pos], false);
      }

      if (pos >= test_len) {
        break;
      }
    }
  }
}
