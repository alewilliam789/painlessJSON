#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdbool.h>
#include "scanner.h"
#include "hashtable.h"
#include "linkedlist.h"
#include "arena.h"

typedef struct JSONNum JSONNum;
typedef struct JSONString JSONString;
typedef struct JSONBool JSONBool;
typedef struct JSONObj JSONObj;

struct JSONNum {
  char value[64];
  size_t length;
  bool decimal;
};

struct JSONBool {
  char parsed_value[5];
  bool value;
};

struct JSONObj {
  bool current_object;
  HashTable objects;
};

typedef union JSONData JSONData;

union JSONData {
  JSONNum* num;
  LinkedList* str;
  JSONBool* boolean;
  void* null;
  JSONObj* json;
  LinkedList* array;
};

typedef enum JSONType JSONType;

enum JSONType {
  NUM,
  STR,
  BOOL,
  NUL,
  JSON,
  ARRAY
};

typedef struct ArrayData ArrayData;

struct ArrayData {
  JSONType data_type;
  JSONData data;
};

typedef struct JSONPair JSONPair;

struct JSONPair {
  char key[256];
  size_t key_length;
  JSONType data_type;
  JSONData data;
  bool current_value;
  bool value_exists;
};


JSONObj* parse_json(JSONBuffer* json_buffer, Arena* arena);
void parse_key(JSONBuffer* json_buffer, JSONPair* current_pair, char* optional_key);
LinkedList* parse_array(JSONBuffer* json_buffer, Arena* arena);
LinkedList* parse_string(JSONBuffer* json_buffer, Arena* arena);
JSONNum* parse_number(JSONBuffer* json_buffer, Arena* arena);
JSONBool* parse_bool(JSONBuffer* json_buffer, Arena* arena, char* boolean_value);
void* parse_null(JSONBuffer* json_buffer, char* null_string);

#endif


