#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "tablize.h"

int main(int argc, char **argv) {
  char c;
  state s = HEADER_START;
  table_builder *b = new_table_builder();
  int line = 1;
  int position = 1;
  int last_is_ascii = 0;
  while (read(STDIN_FILENO, &c, 1)) {
    if (c == '\n') {
      line += 1;
      position = 1;
    } else if (isascii(c) || (last_is_ascii = !last_is_ascii)) {
      position += 1;
    }
    s = parse[s](c, b);
    if (s == ERROR) {
      printf(" at line %d, position %d\n", line, position);
      return 1;
    }
  };
  if (s != VALUE_START) {
    printf("inproper amount of values\n");
    return 1;
  }
  table *t = finish_table(b);
  print_table(t);
  free_table(t);
  return 0;
}

void print_table(table *t) {
  row *r;
  for (int i = 0; i < t->size; i++) {
    r = t->rows[i];
    printf("| %-*s ", r->max_length, r->header);
  }
  printf("|\n");
  for (int i = 0; i < t->size; i++) {
    r = t->rows[i];
    int length = r->max_length - 3;
    char last = '-';
    switch (r->alignment) {
    case LEFT:
      printf("| :-");
      break;
    case RIGHT:
      last = ':';
    default:
    case NONE:
      printf("| --");
      break;
    }
    while (length--) {
      putchar('-');
    }
    putchar(last);
    putchar(' ');
  }
  printf("|\n");
  for (int i = 0; i < t->rows[0]->size; i++) {
    for (int j = 0; j < t->size; j++) {
      r = t->rows[j];
      int unicodes = count_unicode_chars(r->values[i]);
      char *format = r->alignment == LEFT ? "| %-*s " : "| %*s ";
      printf(format, r->max_length + unicodes, r->values[i]);
    }
    printf("|\n");
  }
}

table_builder *new_table_builder() {
  table_builder *b = (table_builder *)malloc(sizeof(table_builder));
  b->capacity = 8;
  b->rows = (row_builder **)malloc(sizeof(row_builder *) * b->capacity);
  b->length = 0;
  b->current_row = 0;
  return b;
}

void new_row(table_builder *b) {
  if (b->capacity == b->length) {
    b->capacity = b->capacity * 2;
    b->rows = realloc(b->rows, sizeof(row_builder *) * b->capacity);
  }
  b->rows[b->length++] = new_row_builder();
}

void append_row_header_char(table_builder *b, char c) {
  append_header_char(b->rows[b->length - 1], c);
}

void set_row_alignment(table_builder *b, alignment a) {
  b->rows[b->current_row++]->alignment = a;
}

void empty_row_value(table_builder *b) {
  b->current_row %= b->length;
  row_builder *r = b->rows[b->current_row++];
  char *string = (char *)malloc(sizeof(char));
  string[0] = '\0';
  r->values[r->values_length++] = string;
}

void new_row_value(table_builder *b, char c) {
  b->current_row %= b->length;
  new_value(b->rows[b->current_row], c);
}

void append_row_value_char(table_builder *b, char c) {
  append_value_char(b->rows[b->current_row], c);
}

void finish_row_value(table_builder *b) {
  finish_value(b->rows[b->current_row++]);
}

table *finish_table(table_builder *b) {
  table *t = (table *)malloc(sizeof(table));
  t->size = b->length;
  t->rows = (row **)malloc(sizeof(row *) * t->size);
  for (int i = 0; i < b->length; i++) {
    t->rows[i] = finish_row(b->rows[i]);
  }
  free(b->rows);
  free(b);
  return t;
}

void free_table(table *t) {
  row *r;
  for (int i = 0; i < t->size; i++) {
    r = t->rows[i];
    for (int j = 0; j < r->size; j++) {
      free(r->values[j]);
    }
    free(r->values);
  }
  free(t->rows);
}

row_builder *new_row_builder() {
  row_builder *rb = (row_builder *)malloc(sizeof(row_builder));
  rb->header = new_string_builder();
  rb->alignment = NONE;
  rb->type = EMPTY;
  rb->current_value = NULL;
  rb->values_capacity = 8;
  rb->values = (char **)malloc(sizeof(char *) * rb->values_capacity);
  rb->values_length = 0;
  rb->max_length = 3;
  return rb;
}

void append_header_char(row_builder *b, char c) {
  int isascii = isascii(c);
  if (c == ' ' || (!isascii && !isascii(b->header->buffered))) {
    b->header->buffered = c;
    return;
  }
  if (b->header->buffered) {
    append_char(b->header, b->header->buffered);
    b->header->buffered = 0;
  }
  append_char(b->header, c);
}

void new_value(row_builder *b, char c) {
  b->current_value = new_string_builder();
  int isascii = isascii(c);
  if (!isascii && !isascii(b->current_value->buffered)) {
    b->current_value->buffered = c;
  } else {
    append_char(b->current_value, c);
  }
}

void append_value_char(row_builder *b, char c) {
  int isascii = isascii(c);
  if (c == ' ' || (!isascii && !isascii(b->current_value->buffered))) {
    b->current_value->buffered = c;
    return;
  }
  if (b->current_value->buffered) {
    append_char(b->current_value, b->current_value->buffered);
    b->current_value->buffered = 0;
  }
  append_char(b->current_value, c);
}

void finish_value(row_builder *b) {
  int real_length = b->current_value->length;
  char *value = finish_string(b->current_value);
  if (b->values_capacity == b->values_length) {
    b->values_capacity = b->values_capacity * 2;
    b->values = realloc(b->values, sizeof(char *) * (b->values_capacity));
  }
  if (b->type != TEXT && *value != '\0') {
    b->type = is_numeric(value) ? NUMERIC : TEXT;
  }
  b->values[b->values_length++] = value;
  real_length -= count_unicode_chars(value);
  if (real_length > b->max_length) {
    b->max_length = real_length;
  }
}

int is_numeric(char *s) {
  char *endptr;
  strtod(s, &endptr);
  return *endptr == '\0';
}

int count_unicode_chars(char *s) {
  int count = 0;
  int last = 0;
  while (*s != '\0') {
    if (!isascii(*s++) && !(last = !last)) {
      count += 1;
    }
  }
  return count;
}

alignment alignment_by_type(value_type type) {
  switch (type) {
  case NUMERIC:
    return RIGHT;
  case TEXT:
    return LEFT;
  default:
  case EMPTY:
    return NONE;
  }
}

row *finish_row(row_builder *b) {
  row *r = (row *)malloc(sizeof(row));
  int real_length = b->header->length;
  r->header = finish_string(b->header);
  real_length -= count_unicode_chars(r->header);
  r->alignment = b->alignment ? b->alignment : alignment_by_type(b->type);
  r->max_length = real_length > b->max_length ? real_length : b->max_length;
  r->values = realloc(b->values, sizeof(char **) * b->values_length);
  r->size = b->values_length;
  free(b);
  return r;
}

string_builder *new_string_builder() {
  string_builder *b = (string_builder *)malloc(sizeof(string_builder));
  b->capacity = 16;
  b->content = (char *)malloc(sizeof(char) * b->capacity);
  b->length = 0;
  b->buffered = 0;
  return b;
}

void append_char(string_builder *b, char c) {
  if (b->capacity == b->length) {
    b->capacity = b->capacity * 2;
    b->content = realloc(b->content, sizeof(char) * (b->capacity));
  }
  b->content[b->length++] = c;
}

char *finish_string(string_builder *b) {
  char *string = realloc(b->content, sizeof(char) * (b->length + 1));
  string[b->length] = '\0';
  free(b);
  return string;
}

state parse_fresh(char c, table_builder *b) {
  switch (c) {
  case '|':
    new_row(b);
    return HEADER_FIRST_PIPE;
  case ' ':
  case '\t':
  case '\n':
    return HEADER_START;
  default:
    printf("expected '|', got '%c'", c);
    return ERROR;
  }
}

state parse_header_first_pipe(char c, table_builder *b) {
  switch (c) {
  case ' ':
  case '\t':
    return HEADER_FIRST_PIPE;
  case '\n':
  case '|':
    printf("header field cannot be empty");
    return ERROR;
  default:
    append_row_header_char(b, c);
    return HEADER_FIELD;
  }
}

state parse_header_field(char c, table_builder *b) {
  switch (c) {
  case '|':
    return HEADER_PIPE;
  case '\n':
    printf("expected '|', got newline");
    return ERROR;
  case '\t':
    c = ' ';
  default:
    append_row_header_char(b, c);
    return HEADER_FIELD;
  }
}

state parse_header_pipe(char c, table_builder *b) {
  switch (c) {
  case '|':
    printf("header field cannot be empty");
    return ERROR;
  case ' ':
  case '\t':
    return HEADER_PIPE;
  case '\n':
    return SEPARATION_START;
  default:
    new_row(b);
    append_row_header_char(b, c);
    return HEADER_FIELD;
  }
}

state parse_separation_start(char c, table_builder *b) {
  switch (c) {
  case '|':
    return SEPARATION_FIRST_PIPE;
  case ' ':
  case '\t':
  case '\n':
    return SEPARATION_START;
  default:
    printf("expected '|', got '%c'", c);
    return ERROR;
  }
}

state parse_separation_first_pipe(char c, table_builder *b) {
  switch (c) {
  case ':':
    set_row_alignment(b, LEFT);
    return SEPARATION_LEFT_ALIGNMENT;
  case '-':
    return SEPARATION_FIELD;
  case ' ':
  case '\t':
    return SEPARATION_FIRST_PIPE;
  case '\n':
    printf("expected ':' or '-', got newline");
    return ERROR;
  default:
    printf("expected ':' or '-', got '%c'", c);
    return ERROR;
  }
}

state parse_separation_left_alignment(char c, table_builder *b) {
  switch (c) {
  case '|':
    return SEPARATION_PIPE;
  case '-':
    return SEPARATION_LEFT_ALIGNMENT;
  case ' ':
  case '\t':
    return SEPARATION_RIGHT_ALIGNMENT;
  case '\n':
    printf("expected '-' or '|', got newline");
    return ERROR;
  default:
    printf("expected '-' or '|', got '%c'", c);
    return ERROR;
  }
}

state parse_separation_field(char c, table_builder *b) {
  switch (c) {
  case '|':
    set_row_alignment(b, NONE);
    return SEPARATION_PIPE;
  case ':':
    set_row_alignment(b, RIGHT);
    return SEPARATION_RIGHT_ALIGNMENT;
  case ' ':
  case '\t':
    set_row_alignment(b, NONE);
    return SEPARATION_RIGHT_ALIGNMENT;
  case '-':
    return SEPARATION_FIELD;
  case '\n':
    printf("expected '-', ':' or '|', got newline");
    return ERROR;
  default:
    printf("expected '-', ':' or '|', got '%c'", c);
    return ERROR;
  }
}

state parse_separation_right_alignment(char c, table_builder *b) {
  switch (c) {
  case '|':
    return SEPARATION_PIPE;
  case ' ':
  case '\t':
    return SEPARATION_RIGHT_ALIGNMENT;
  case '\n':
    printf("expected '|', got newline");
    return ERROR;
  default:
    printf("expected '|', got '%c'", c);
    return ERROR;
  }
}

state parse_separation_pipe(char c, table_builder *b) {
  switch (c) {
  case ' ':
  case '\t':
    return SEPARATION_PIPE;
  case ':':
    set_row_alignment(b, LEFT);
    return SEPARATION_LEFT_ALIGNMENT;
  case '-':
    return SEPARATION_FIELD;
  case '\n':
    return VALUE_START;
  default:
    printf("expected '-', ':' or a newline, got '%c'", c);
    return ERROR;
  }
}

state parse_value_start(char c, table_builder *b) {
  switch (c) {
  case '|':
    return VALUE_FIRST_PIPE;
  case ' ':
  case '\t':
  case '\n':
    return VALUE_START;
  default:
    printf("expected '|', got '%c'", c);
    return ERROR;
  }
}

state parse_value_first_pipe(char c, table_builder *b) {
  switch (c) {
  case '|':
    empty_row_value(b);
    return VALUE_PIPE;
  case ' ':
  case '\t':
    return VALUE_FIRST_PIPE;
  default:
    new_row_value(b, c);
    return VALUE_FIELD;
  }
}

state parse_value_field(char c, table_builder *b) {
  switch (c) {
  case '|':
    finish_row_value(b);
    return VALUE_PIPE;
  case '\t':
    c = ' ';
  default:
    append_row_value_char(b, c);
    return VALUE_FIELD;
  }
}

state parse_value_pipe(char c, table_builder *b) {
  switch (c) {
  case '|':
    empty_row_value(b);
  case ' ':
  case '\t':
    return VALUE_PIPE;
  case '\n':
    return VALUE_START;
  default:
    new_row_value(b, c);
    return VALUE_FIELD;
  }
}
