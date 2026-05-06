#pragma once

#define ANSI_FG_GREEN "\033[32m"
#define ANSI_FG_RED "\033[31m"
#define ANSI_COLOR_SIZE 5
#define TYPETEST_BUF_SIZE 1024

void disable_raw_mode();
void enable_raw_mode();

void term_set_text(char s[TYPETEST_BUF_SIZE]);
int term_send_char(char ch, char color[ANSI_COLOR_SIZE]);
int term_send_backspace(char replace);
