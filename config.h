#pragma once

typedef struct Settings Settings;
struct Settings {
  int words_count;
};

void configure(int argc, char **argv);

extern Settings config;
