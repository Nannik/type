#include <signal.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "term.h"

#define ANSI_RESET "\033[0m"
#define ANSI_CLEAR "\033[2J"
#define ANSI_MOVE_LEFT(C) "\033[" #C "D"
#define ANSI_MOVE_RC(R, C) "\033[" #R ";" #C "H"

struct termios orig_term;

typedef struct Term {
  int col;
  int row;
  int width;
} Term;
Term t;

void set_term_width(int signo) {
  struct winsize wins;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &wins);

  t.width = wins.ws_col;
}

void disable_raw_mode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_term);
}

void enable_raw_mode() {
  tcgetattr(STDIN_FILENO, &orig_term);
  atexit(disable_raw_mode);

  struct termios raw = orig_term;
  raw.c_lflag &= ~(ICANON | ECHO);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

  set_term_width(SIGWINCH);
  signal(SIGWINCH, set_term_width);

  t.col = 0;
  t.row = 0;
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

int term_send_char(char ch, char color[ANSI_COLOR_SIZE]) {
  char buf[ANSI_COLOR_SIZE + 10];
  int len = snprintf(
    buf,
    sizeof(buf),
    "%s%c" ANSI_RESET,
    color,
    ch
  );
  write(STDOUT_FILENO, buf, len);

  t.col++;
  if (t.col == t.width) {
    t.col = 0;
    t.row++;
  }

  return t.row * t.width + t.col;
}

int term_send_backspace(char replace) {
  char buf[100];
  int len;

  if (t.col > 0) {
    t.col--;

    len = snprintf(
      buf, 
      sizeof(buf), 
      "\033[%d;%dH%c\033[%d;%dH",
      t.row + 1, t.col + 1, replace, t.row + 1, t.col + 1
    );
  } else if (t.row > 0) {
    len = snprintf(
      buf, 
      sizeof(buf), 
      "\033[%d;%dH%c\033[%d;%dH",
      t.row, t.width, replace, t.row, t.width
    );

    t.col = t.width - 1;
    t.row--;
  } else {
    return 0;
  }

  write(STDOUT_FILENO, buf, len);

  return t.row * t.width + t.col;
}
