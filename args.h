#pragma once

typedef struct Settings Settings;
struct Settings {
  int wordc;
  int time;
  char *dictionary_filepath;
};

void configure(int argc, char **argv);

extern Settings config;
