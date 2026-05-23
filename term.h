#pragma once

#include <stdbool.h>

#define TYPETEST_BUF_SIZE 1024
#define STATUS_LINE_SIZE 64

typedef struct Reference {
  int len;
  char s[TYPETEST_BUF_SIZE];
} Reference;
extern Reference *term_ref;

void disable_raw_mode();

typedef int (*reference_generator)(char *buf);
void term_init(reference_generator);

int term_send_char(char ch, bool acc);
int term_send_backspace(char replace);
void term_update_status_line(char *sl);
