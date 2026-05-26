#pragma once

typedef struct Settings Settings;
struct Settings {
  int wordc;
  int time; // test time in seconds
  char *dictionary_filepath;
  char *profile_filepath;
};

void configure(int argc, char **argv);

extern Settings config;
