#pragma once

typedef struct Settings Settings;
struct Settings {
  int words_count;
  int run_time;
  int repeat_count;
  char *dictionary_filepath;
};

void configure(int argc, char **argv);

extern Settings config;
