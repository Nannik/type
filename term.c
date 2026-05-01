#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "term.h"

#define ANSI_RESET "\033[0m"
#define ANSI_CLEAR "\033[2J"
#define ANSI_MOVE_RC(R, C) "\033[" #R ";" #C "H"
#define ANSI_MOVE_LEFT(C) "\033[" #C "D"

struct termios orig_term;

void disable_raw_mode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_term);
}

void enable_raw_mode() {
  tcgetattr(STDIN_FILENO, &orig_term);
  atexit(disable_raw_mode);

  struct termios raw = orig_term;
  raw.c_lflag &= ~(ICANON | ECHO);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void term_set_text(char s[TYPETEST_BUF_SIZE]) {
  char buf[TYPETEST_BUF_SIZE + 5 + 7 + 7];
  int len = snprintf(
    buf,
    sizeof(buf),
    ANSI_CLEAR ANSI_MOVE_RC(0, 0) "%s" ANSI_MOVE_RC(0, 0),
    s
  );
  write(STDOUT_FILENO, buf, len);
}

void term_send_char(char ch, char color[ANSI_COLOR_SIZE]) {
  char buf[ANSI_COLOR_SIZE + 10];
  int len = snprintf(
    buf,
    sizeof(buf),
    "%s%c" ANSI_RESET,
    color,
    ch
  );
  write(STDOUT_FILENO, buf, len);
}

void term_send_backspace(char replace) {
  char buf[11];
  int len = snprintf(
    buf, 
    sizeof(buf), 
    ANSI_MOVE_LEFT(1) "%c" ANSI_MOVE_LEFT(1), 
    replace
  );
  write(STDOUT_FILENO, buf, len);
}
