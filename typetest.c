#include <pthread.h>
#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dataset.h"
#include "typetest.h"
#include "args.h"
#include "term.h"

#define WORDS_BUF_SIZE 10000
#define WORD_SIZE 100


volatile sig_atomic_t stop = 0;

volatile int wpm = 0;
volatile int pos = 0;
volatile int cur_word = 0;
volatile int time_passed = 0;

int words_c = 0;
char words[WORDS_BUF_SIZE][WORD_SIZE];

int pick (char *);
void on_timer_stop(int);
void* timer(void *arg);
void get_status_text(char *buf, size_t max_len);


void read_dictionary (FILE *f) {
  char word[WORD_SIZE];

  while (fscanf(f, "%99s", word) == 1 && words_c < WORDS_BUF_SIZE) {
    strcpy(words[words_c++], word);
  };
}

void start_test() {
  pthread_t p = 0;

  struct sigaction int_handler = {
    .sa_handler=on_timer_stop,
    .sa_flags=0
  };
  sigemptyset(&int_handler.sa_mask);
  sigaction(SIGINT,&int_handler,0);

  term_init(pick);

  char sl[STATUS_LINE_SIZE];
  get_status_text(sl, sizeof(sl));
  term_update_status_line(sl);

  char c;
  while (!stop && read(STDIN_FILENO, &c, 1) != -1) {
    if (p == 0) {
      pthread_create(&p, NULL, timer, NULL);
      dataset_start_test();
    }

    dataset_write(pos, term_ref->s[pos], c);

    if (c == 127) {
      pos = term_send_backspace(term_ref->s[pos - 1]);

      if (isspace(term_ref->s[pos])) {
        cur_word--;
        if (cur_word < 0) cur_word = 0;
      }
    } else if (
      (c >= 'a' && c <= 'z') || 
      (c >= 'A' && c <= 'Z') ||
      isspace(c)
    ) {
      if (isspace(term_ref->s[pos])) {
        cur_word++;
      }

      if (c == term_ref->s[pos]) {
        pos = term_send_char(term_ref->s[pos], true);
      } else {
        pos = term_send_char(term_ref->s[pos], false);
      }

      if (pos >= term_ref->len) {
        break;
      }
    }

    get_status_text(sl, sizeof(sl));
    term_update_status_line(sl);
  }

  get_status_text(sl, sizeof(sl));
  term_update_status_line(sl);
}


int generated_c = 0;
int pick (char *res) {
  if (config.wordc == 0 || generated_c++ < config.wordc) {
    strcpy(res, words[rand() % words_c]);
    return strlen(res);
  }

  return -1;
}

void on_timer_stop(int sig) {
  stop = 1;
}

void* timer(void *arg) {
  while (1) {
    sleep(1);
    time_passed += 1;

    char sl[STATUS_LINE_SIZE];
    get_status_text(sl, sizeof(sl));
    term_update_status_line(sl);
    wpm = ((double)pos / 5) / ((double)time_passed / 60);

    if (config.time != 0 && time_passed >= config.time) {
      kill(getpid(), SIGINT);
    }
  }

  return 0;
}

void get_status_text(char *buf, size_t max_len) {
  char *p = buf;

  if (config.wordc > 0) {
    int n = snprintf(
      p,
      max_len,
      "Words: %d/%d | ",
      cur_word,
      config.wordc
    );
    p += n; max_len -= n;
  }

  if (config.time > 0) {
    int n = snprintf(
      p,
      max_len,
      "Time: %ds | ",
      config.time - time_passed
    );
    p += n; max_len -= n;
  }

  int n = snprintf(
    p,
    max_len,
    "WPM: %d",
    wpm
  );
}
