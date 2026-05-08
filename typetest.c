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

int pick (char *res) {
  strcpy(res, words[rand() % words_c]);
  return strlen(res);
}

void start_test() {
  term_init(pick);

  char c;
  int pos = 0;
  while (read(STDIN_FILENO, &c, 1) == 1) {
    if (c == 127) {
      pos = term_send_backspace(term_ref->s[pos - 1]);
    } else if (
      (c >= 'a' && c <= 'z') || 
      (c >= 'A' && c <= 'Z') ||
      isspace(c)
    ) {
      char s[11];
      if (c == term_ref->s[pos]) {
        pos = term_send_char(term_ref->s[pos], true);
      } else {
        pos = term_send_char(term_ref->s[pos], false);
      }

      if (pos >= term_ref->len) {
        break;
      }
    }
  }
}
