#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>

#include "term.h"

#define ANSI_FG_GREEN "\033[32m"
#define ANSI_FG_RED "\033[31m"
#define ANSI_RESET "\033[0m"
#define ANSI_CLEAR "\033[2J"
#define ANSI_MOVE_LEFT(C) "\033[" #C "D"
#define ANSI_MOVE_RC(R, C) "\033[" #R ";" #C "H"

typedef char color;
#define COLOR_RESET 0
#define COLOR_ACC 1
#define COLOR_WRNG 2

char *get_color_escape(color c) {
  switch (c) {
  case COLOR_RESET:
    return ANSI_RESET;
  case COLOR_ACC:
    return ANSI_FG_GREEN;
  case COLOR_WRNG:
    return ANSI_FG_RED;
  }

  return "";
}

typedef struct Term {
  int width;
} Term;
Term t;

typedef struct Reference {
  char s[TYPETEST_BUF_SIZE];
  color acc[TYPETEST_BUF_SIZE];
  int roww[128];
  int rowc;
} Reference;
Reference ref;

typedef struct Cursor {
  int posn;
  int posr;
  int posc;
} Cursor;
Cursor cur;

struct termios orig_term;

void cur_pos_inc() {
  cur.posn++;
  cur.posc++;
  if (cur.posc >= ref.roww[cur.posr]) {
    cur.posc = 0;
    cur.posr++;
  }
}

void cur_pos_dec() {
  if (cur.posn == 0) return;

  cur.posn--;

  if (cur.posc > 0) {
    cur.posc--;
  } else {
    cur.posr--;
    cur.posc = ref.roww[cur.posr] - 1;
  }
}

void jump_to_cur() {
  char buf[128];
  int len = snprintf(
    buf,
    sizeof(buf),
    "\033[%d;%dH",
    cur.posr + 1, cur.posc + 1
  );
  write(STDOUT_FILENO, buf, len);
}

void cur_replace(char replace, color acc) {
  char buf[128];

  int len = snprintf(
    buf,
    sizeof(buf),
    "%s\033[%d;%dH%c\033[%d;%dH",
    get_color_escape(acc), cur.posr + 1, cur.posc + 1, replace, cur.posr + 1, cur.posc + 1
  );
  write(STDOUT_FILENO, buf, len);
}

void wrap_str() {
  int prev_space_pos = 0;
  int put = 0;

  ref.rowc = 1;
  for (int i = 0; i < TYPETEST_BUF_SIZE; i++) {
    if (isspace(ref.s[i])) {
      ref.s[i] = ' ';

      if (i - put > t.width) {
        ref.s[prev_space_pos] = '\n';
        ref.roww[ref.rowc - 1] = prev_space_pos - put + 1;
        ref.rowc++;
        put = prev_space_pos + 1;
      }

      prev_space_pos = i;
    } else if (ref.s[i] == '\0') {
      if (i - put > t.width) {
        ref.s[prev_space_pos] = '\n';
        ref.roww[ref.rowc - 1] = prev_space_pos - put + 1;
        ref.rowc++;
        put = prev_space_pos + 1;
      }
      ref.roww[ref.rowc - 1] = i - put + 1;
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
  for (int i = 0; i < ref.rowc; i++) {
    c += ref.roww[i];
    if (c >= cur.posn) {
      cur.posr = i;
      cur.posc = cur.posn - pc;
      break;
    }
    pc = c;
  }

  char buf[10];
  int len = snprintf(
    buf,
    sizeof(buf),
    "\033[%d;%dH",
    cur.posr + 1, cur.posc + 1
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

  cur.posc = 0;
  cur.posr = 0;
  cur.posn = 0;
}

void term_feed_str(char s[TYPETEST_BUF_SIZE]) {
  strcpy(ref.s, s);
  wrap_str();
}

void term_write_str() {
  char buf[TYPETEST_BUF_SIZE * 8];
  char *p = buf;
  size_t rem = sizeof(buf);
  size_t len = strlen(ref.s);

  int i;
  int n = snprintf(
    buf,
    sizeof(buf),
    ANSI_CLEAR ANSI_MOVE_RC(0, 0)
  );

  if (n < 0) return;
  p += n; rem -= n;

  for (int i = 0; i < cur.posn; i++) {
    char *esc = get_color_escape(ref.acc[i]);
    char n = strlen(esc);
    memcpy(p, esc, n);
    p += n;
    *p++ = ref.s[i];
    rem -= n + 1;
  }

  n = snprintf(p, rem, ANSI_RESET);
  if (n < 0) return;
  p += n; rem -= n;

  for (i = cur.posn; i < strlen(ref.s); i++) {
    *p++ = ref.s[i];
    rem--;
  }

  n = snprintf(p, rem, ANSI_MOVE_RC(0, 0));
  if (n < 0) return;
  p += n; rem -= n;

  write(STDOUT_FILENO, buf, p - buf);
}

int term_send_char(char ch, bool is_acc) {
  if (is_acc) {
    cur_replace(ch, COLOR_ACC);
    ref.acc[cur.posn] = COLOR_ACC;
  } else {
    cur_replace(ch, COLOR_WRNG);
    ref.acc[cur.posn] = COLOR_WRNG;
  }

  cur_pos_inc();
  jump_to_cur();

  return cur.posn;
}

int term_send_backspace(char replace) {
  cur_pos_dec();
  cur_replace(replace, COLOR_RESET);

  return cur.posn;
}
