#include <ctype.h>
#include <signal.h>
#include <string.h>
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
  int pos;
  int rows_c;
  int rows_w[100];
  char str[TYPETEST_BUF_SIZE];
} Term;
Term t;

void wrap_str() {
  int prev_space_pos = 0;
  int put = 0;

  t.rows_c = 1;
  for (int i = 0; i < TYPETEST_BUF_SIZE; i++) {
    if (isspace(t.str[i])) {
      t.str[i] = ' ';

      if (i - put > t.width) {
        t.str[prev_space_pos] = '\n';
        t.rows_w[t.rows_c - 1] = prev_space_pos - put + 1;
        t.rows_c++;
        put = prev_space_pos + 1;
      }

      prev_space_pos = i;
    } else if (t.str[i] == '\0') {
      if (i - put > t.width) {
        t.str[prev_space_pos] = '\n';
        t.rows_w[t.rows_c - 1] = prev_space_pos - put + 1;
        t.rows_c++;
        put = prev_space_pos + 1;
      }
      t.rows_w[t.rows_c - 1] = i - put + 1;
      break;
    }
  }
}

void update_term_width(int signo) {
  struct winsize wins;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &wins);

  t.width = wins.ws_col - 1;

  wrap_str();
  term_write_str();

  int c = 0, pc = 0;
  for (int i = 0; i < t.rows_c; i++) {
    c += t.rows_w[i];
    if (c >= t.pos) {
      t.row = i;
      t.col = t.pos - pc;
      break;
    }
    pc = c;
  }

  char buf[10];
  int len = snprintf(
    buf,
    sizeof(buf),
    "\033[%d;%dH",
    t.row + 1, t.col + 1
  );
  write(STDOUT_FILENO, buf, len);
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

  update_term_width(SIGWINCH);
  signal(SIGWINCH, update_term_width);

  t.col = 0;
  t.row = 0;
  t.pos = 0;
}

void term_feed_str(char s[TYPETEST_BUF_SIZE]) {
  strcpy(t.str, s);
  wrap_str();
}

void term_write_str() {
  char buf[TYPETEST_BUF_SIZE + 5 + 7 + 7];
  int len = snprintf(
    buf,
    sizeof(buf),
    ANSI_CLEAR ANSI_MOVE_RC(0, 0) "%s" ANSI_MOVE_RC(0, 0),
    t.str
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

  t.pos++;
  t.col++;
  if (t.col >= t.rows_w[t.row]) {
    t.col = 0;
    t.row++;

    char buf[10];
    int len = snprintf(
      buf,
      sizeof(buf),
      "\033[%d;0H",
      t.row + 1
    );
    write(STDOUT_FILENO, buf, len);
  }

  return t.pos;
}

int term_send_backspace(char replace) {
  char buf[100];
  int len;

  if (t.col > 0) {
    t.col--;

    len = snprintf(
      buf, 
      sizeof(buf), 
      ANSI_MOVE_LEFT(1) "%c" ANSI_MOVE_LEFT(1),
      replace
    );
  } else if (t.row > 0) {
    len = snprintf(
      buf, 
      sizeof(buf), 
      "\033[%d;%dH%c\033[%d;%dH",
      t.row, t.rows_w[t.row - 1], replace, t.row, t.rows_w[t.row - 1]
    );

    t.col = t.rows_w[t.row - 1] - 1;
    t.row--;
  } else {
    return 0;
  }

  write(STDOUT_FILENO, buf, len);

  t.pos--;
  return t.pos;
}
