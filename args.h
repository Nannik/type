#pragma once

#include <stdbool.h>

typedef struct Settings Settings;
struct Settings {
  int wordc;
  int time; // test time in seconds
  char *dictionary_filepath;
  char *profile;
  bool output;
  bool exit;
};

void configure(int argc, char **argv);

extern Settings config;
