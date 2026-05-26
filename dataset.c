#include <stdio.h>
#include <time.h>

#include "dataset.h"


FILE *dataset;

typedef struct timespec timespec;
timespec start;
timespec last;


long toms(timespec t);


void dataset_start_test() {
  dataset = fopen("dataset.csv", "a");
  if (dataset == NULL) {
    perror("fopen");
    return;
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
}

void dataset_write(int pos, char char_orig, char char_typed) {
  timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);

  fprintf(
    dataset,
    "%ld,%ld,%d,%c,%c\n",
    start.tv_sec, toms(t) - toms(last), pos, char_orig, char_typed
  );

  last = t;
}


long toms(timespec t) {
  return (t.tv_sec * 1000) + (t.tv_nsec / 1e6);
}
