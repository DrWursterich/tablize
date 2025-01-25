#ifndef TABLIZE_H
#define TABLIZE_H

typedef enum state {
  HEADER_START = 0,
  HEADER_FIRST_PIPE = 1,
  HEADER_FIELD = 2,
  HEADER_PIPE = 3,
  SEPARATION_START = 4,
  SEPARATION_FIRST_PIPE = 5,
  SEPARATION_LEFT_ALIGNMENT = 6,
  SEPARATION_FIELD = 7,
  SEPARATION_RIGHT_ALIGNMENT = 8,
  SEPARATION_PIPE = 9,
  VALUE_START = 10,
  VALUE_FIRST_PIPE = 11,
  VALUE_FIELD = 12,
  VALUE_PIPE = 13,
  ERROR = 14,
} state;

typedef enum alignment {
  LEFT,
  RIGHT,
  NONE,
} alignment;

typedef enum value_type {
  EMPTY,
  NUMERIC,
  TEXT,
} value_type;

typedef struct row {
  char *header;
  alignment alignment;
  /** size of the longest string found */
  int max_length;

  char **values;
  /** amount of lines (aka values) */
  int size;
} row;

typedef struct table {
  row **rows;
  int size;
} table;

typedef struct string_builder {
  char *content;
  int length;
  int capacity;
  char buffered;
} string_builder;

typedef struct row_builder {
  string_builder *header;
  alignment alignment;
  value_type type;
  string_builder *current_value;
  char **values;
  int values_length;
  int values_capacity;
  int max_length;
} row_builder;

typedef struct table_builder {
  row_builder **rows;
  int length;
  int capacity;
  int current_row;
} table_builder;

int main(int argc, char **argv);

void print_table(table *t);

state parse_fresh(char c, table_builder *b);
state parse_header_first_pipe(char c, table_builder *b);
state parse_header_field(char c, table_builder *b);
state parse_header_pipe(char c, table_builder *b);
state parse_separation_start(char c, table_builder *b);
state parse_separation_first_pipe(char c, table_builder *b);
state parse_separation_left_alignment(char c, table_builder *b);
state parse_separation_field(char c, table_builder *b);
state parse_separation_right_alignment(char c, table_builder *b);
state parse_separation_pipe(char c, table_builder *b);
state parse_value_start(char c, table_builder *b);
state parse_value_first_pipe(char c, table_builder *b);
state parse_value_field(char c, table_builder *b);
state parse_value_pipe(char c, table_builder *b);

table_builder *new_table_builder();
void new_row(table_builder *b);
void append_row_header_char(table_builder *b, char c);
void set_row_alignment(table_builder *b, alignment a);
void empty_row_value(table_builder *b);
void new_row_value(table_builder *b, char c);
void append_row_value_char(table_builder *b, char c);
void finish_row_value(table_builder *b);
table *finish_table(table_builder *b);
void free_table(table *t);

row_builder *new_row_builder();
void append_header_char(row_builder *b, char c);
void new_value(row_builder *b, char c);
void append_value_char(row_builder *b, char c);
void finish_value(row_builder *b);
row *finish_row(row_builder *b);

string_builder *new_string_builder();
void append_char(string_builder *b, char c);
char *finish_string(string_builder *b);

int is_numeric(char *s);
int count_unicode_chars(char *s);

static state (*parse[])(char, table_builder *) = {
    &parse_fresh,
    &parse_header_first_pipe,
    &parse_header_field,
    &parse_header_pipe,
    &parse_separation_start,
    &parse_separation_first_pipe,
    &parse_separation_left_alignment,
    &parse_separation_field,
    &parse_separation_right_alignment,
    &parse_separation_pipe,
    &parse_value_start,
    &parse_value_first_pipe,
    &parse_value_field,
    &parse_value_pipe,
};

#endif
