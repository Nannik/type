#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "dataset.h"
#include "args.h"


FILE *dataset;

typedef struct timespec timespec;
timespec start;
timespec last;


long toms(timespec t);
char *get_dataset_path();
char *char_decode(char ch);


int dataset_start_test() {
  dataset = fopen(get_dataset_path(), "a");
  if (dataset == NULL) {
    perror("fopen");
    return -1;
  }

  long pos = ftell(dataset);

  if (pos == 0) {
    fprintf(
      dataset,
      "test_started_time,time,pos,char_orig,char_typed\n"
    );
  }

  clock_gettime(CLOCK_MONOTONIC, &start);
  last = start;

  return 0;
}

void dataset_write(int pos, char char_orig, char char_typed) {
  if (dataset == NULL) {
    return;
  }

  timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);

  fprintf(
    dataset,
    "%ld,%ld,%d,%c,%c\n",
    start.tv_sec, toms(t) - toms(last), pos, char_orig, char_typed
  );

  last = t;
}

int dataset_output() {
  char *path = get_dataset_path();
  FILE *f = fopen(path, "r");

  if (f == NULL) {
    perror(path);
    return -1;
  }


  char buf[512];
  fgets(buf, sizeof(buf), f);

  printf("session,delay,position,original_char,typed_char,is_correct\n");

  int session = 0;
  long last_start_t;
  long start_t;
  long dt;
  int pos;
  char char_orig;
  char char_typed;

  while (
    fscanf(
      f, 
      "%ld,%ld,%d,%c,%c\n",
      &start_t,
      &dt,
      &pos,
      &char_orig,
      &char_typed
    ) != -1
  ) {
    if (last_start_t != start_t) {
      session++;
    }

    bool is_correct = false;
    if (char_orig == char_typed) {
      is_correct = true;
    }

    printf(
      "%d,%ld,%d,%s,%s,%d\n",
      session,
      dt,
      pos,
      char_decode(char_orig),
      char_decode(char_typed),
      is_correct
    );

    last_start_t = start_t;
  }

  return 0;
}


long toms(timespec t) {
  return (t.tv_sec * 1000) + (t.tv_nsec / 1e6);
}

char path[512];
char *get_dataset_path() {
  const char *home = getenv("HOME");

  if (home == NULL) {
    fprintf(stderr, "Unable to find home directory");
    return 0;
  }

  snprintf(path, sizeof(path), "%s/.local/share/%s.csv", home, config.profile);

  return path;
}

char decoded_char[32];
char *char_decode(char ch) {
  if (
    (ch >= 'a' && ch <= 'z') ||
    (ch >= 'A' && ch <= 'Z')
  ) {
    decoded_char[0] = ch;
    decoded_char[1] = '\0';
  } else if (ch == 127) {
    strcpy(decoded_char, "<BS>\0");
  } else if (isspace(ch)) {
    strcpy(decoded_char, "<SP>\0");
  } else {
    strcpy(decoded_char, "<NULL>\0");
  }

  return decoded_char;
}
