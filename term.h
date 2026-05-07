#pragma once

#include <stdbool.h>

#define TYPETEST_BUF_SIZE 1024

void disable_raw_mode();
void enable_raw_mode();

void term_feed_str(char s[TYPETEST_BUF_SIZE]);
void term_write_str();
int term_send_char(char ch, bool acc);
int term_send_backspace(char replace);
