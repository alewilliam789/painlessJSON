#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include <stdbool.h>

int json_scanner(FILE* json_file, char* file_path);

typedef struct JSONBuffer JSONBuffer;

struct JSONBuffer {
  char* current_file;
  size_t current_position;
  int current_col;
  int current_row;
  size_t file_size;
  bool error;
  char* error_message;
};

#endif
