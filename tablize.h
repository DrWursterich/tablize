#ifndef TABLIZE_H
#define TABLIZE_H

#define ALIGN_CHAR_OFFSET (':' - '-')
#define LEFT_ALIGN_CHAR(a) (char)('-' + (a & 1) * ALIGN_CHAR_OFFSET)
#define RIGHT_ALIGN_CHAR(a) (char)('-' + ((a >> 1) & 1) * ALIGN_CHAR_OFFSET)

typedef enum state {
  HEADER_START = 0,
  HEADER_FIRST_PIPE = 1,
  HEADER_FIELD = 2,
  HEADER_PIPE = 3,
  SEPARATION_START = 4,
  SEPARATION_FIRST_PIPE = 5,
  SEPARATION_FIELD = 6,
  SEPARATION_RIGHT_ALIGNMENT = 7,
  SEPARATION_PIPE = 8,
  VALUE_START = 9,
  VALUE_FIRST_PIPE = 10,
  VALUE_FIELD = 11,
  VALUE_PIPE = 12,
  ERROR = 13,
} state;

typedef enum alignment {
  NONE = 0,
  LEFT = 1,
  RIGHT = 2,
  CENTER = 3,
} alignment;

typedef enum value_type {
  EMPTY = 0,
  NUMERIC = 1,
  TEXT = 2,
} value_type;

typedef struct row {
  char *header;
  alignment alignment;
  int max_length;
  char **values;
  int size;
} row;

typedef struct table {
  row **rows;
  int size;
  char *indent;
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
  string_builder *indent;
} table_builder;

int main(int argc, char **argv);

void print_table(table *t);
void center_value(char *text, int max_length, int unicodes);

state parse_fresh(char c, table_builder *b);
state parse_header_first_pipe(char c, table_builder *b);
state parse_header_field(char c, table_builder *b);
state parse_header_pipe(char c, table_builder *b);
state parse_separation_start(char c, table_builder *b);
state parse_separation_first_pipe(char c, table_builder *b);
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
void separator_left_colon(table_builder *b);
void separator_right_colon(table_builder *b);
void separator_no_right_colon(table_builder *b);
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
    &parse_separation_field,
    &parse_separation_right_alignment,
    &parse_separation_pipe,
    &parse_value_start,
    &parse_value_first_pipe,
    &parse_value_field,
    &parse_value_pipe,
};

#endif
