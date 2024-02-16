#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "parser.h"
#include "scanner.h"


void file_test(char* file_path, int assert_value) {
  FILE* current_file;

  current_file = fopen(file_path, "r");

  assert(current_file != NULL);

  assert(json_scanner(current_file, file_path) == assert_value);
  fclose(current_file);
}

int main() {
    
  // Can handle empty JSON
  char* test_file= "tests/empty-test.json";

  file_test(test_file,0);
  
  // Can parse strings, boolean, and numbers
  test_file = "tests/base-test.json";

  file_test(test_file,0);

  // Can parse nested json
  test_file = "tests/nested-test.json";

  file_test(test_file,0);

  // Can parse array
  test_file = "tests/array-test.json";

  file_test(test_file,0);

  // Can parse a json array
  test_file = "tests/json-array-test.json";

  file_test(test_file,0);

  // Can handle non-json text without false positive
  test_file = "tests/not-test.json";

  file_test(test_file,1);

  // Can detect a file without any json
  test_file = "tests/no-json-test.json";

  file_test(test_file,1);

  // Can detect an unclosed key
  test_file = "tests/not-closed-key-test.json";

  file_test(test_file,1);

  // Can detect unclosed string
  test_file = "tests/not-closed-string-test.json";

  file_test(test_file,1);

  // Can detect faulty boolean data
  test_file = "tests/not-boolean-test.json";

  file_test(test_file,1);

  // Can detect faulty null data
  test_file = "tests/not-null-test.json";

  file_test(test_file,1);

  // Can handle detect missing comma
  test_file = "tests/not-nested-test.json";

  file_test(test_file, 1);

  // Can handle no comma array
  test_file = "tests/no-comma-array-test.json";

  file_test(test_file, 1);

  // Can handle comma in the beginning of array
  test_file = "tests/comma-without-value-array-test.json";

  file_test(test_file, 1);

  return 0;
}
