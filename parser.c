#include <arena.h>
#include <ctype.h>
#include <linkedlist.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "scanner.h"
#include "parser.h"
#include "hashtable.h"

JSONPair* json_pair_create(Arena* arena) {
  JSONPair* new_pair = arena->alloc(arena,sizeof(JSONPair));
  new_pair->current_value = false;
  new_pair->key_length = 0;
  new_pair->value_exists = false;
  return new_pair;
}

JSONObj* json_object_create(Arena* arena) {
  JSONObj* json_object = arena->alloc(arena, sizeof(JSONObj));
  HashTable objects;
  hashtable_init(&objects, 100, arena);
  json_object->objects = objects;
  json_object->current_object = true;
  return json_object;
}

LinkedList* linkedlist_create(Arena* arena) {
  LinkedList* linked_list = arena->alloc(arena, sizeof(LinkedList));
  linked_list_init(linked_list, arena);
  return linked_list;
}

bool isescaped(char current_character) {
  char escape_characters[] = {'"','\n','\t','\v','\f','\r'};
  int array_length = 6;
  
  int i = 0;
  while(i < array_length) {
    if(current_character == escape_characters[i]) {
      return true;
    }
    i++;
  }

  return false;
}

JSONObj* parse_json(JSONBuffer* json_buffer, Arena* arena) {

    JSONPair* json_pair = json_pair_create(arena);

    JSONObj* json = NULL;

    bool was_bracketed = false;

    char current_character;

    while(json_buffer->current_position < json_buffer->file_size-1 && !json_buffer->error) {
      
      current_character = json_buffer->current_file[json_buffer->current_position];

      if(isdigit(current_character) && json_pair->current_value) {
        json_pair->data_type = NUM;
        json_pair->data.num = parse_number(json_buffer, arena);

        json_pair->value_exists = true;
        json_pair->current_value = false;
      }
      else if(current_character == '{') {
        if(json == NULL) {
          json = json_object_create(arena);
          was_bracketed = true;
        }
        else {
          JSONObj* nested_json = parse_json(json_buffer,arena);
          
          json_pair->data_type = JSON;
          json_pair->data.json = nested_json;
          json_pair->current_value = false;
        }
      }
      else if(current_character == '[' && (json_pair->current_value || json == NULL)) {
        json_buffer->current_position++;
        json_buffer->current_col++;
        json_pair->data.array = parse_array(json_buffer, arena);

        json_pair->data_type = ARRAY;
        json_pair->value_exists = true;
        json_pair->current_value = false;

        if(json == NULL) {
          json_pair->key_length = 4;
          parse_key(json_buffer,json_pair, "root");
          json = json_object_create(arena);
          json->objects.methods->insert(&json->objects, json_pair->key, json_pair->key_length, json_pair);
          break;
        }
      }
      else if(current_character == '"' && json != NULL) {
        if(json_pair->key_length == 0) {
          parse_key(json_buffer,json_pair, NULL);
          was_bracketed = false;
        }
        else if(json_pair->current_value) {
          json_pair->data.str = parse_string(json_buffer,arena);

          json_pair->data_type = STR;
          json_pair->value_exists = true;
          json_pair->current_value = false;
        }
        else if(json_pair->key_length > 0 && json_pair->value_exists) {
          json_buffer->error = true;
          json_buffer->error_message = "There needs to be a comma between JSON pairs";
          break;
        }
      }
      else if(current_character == 't' && json != NULL) {
        json_pair->data.boolean = parse_bool(json_buffer, arena, "true");
        
        json_pair->data_type = BOOL;
        json_pair->value_exists = true;
        json_pair->current_value = false;
      }
      else if(current_character == 'f' && json != NULL) {
        json_pair->data.boolean = parse_bool(json_buffer, arena, "false");
        
        json_pair->data_type = BOOL;
        json_pair->value_exists = true;
        json_pair->current_value = false;
      }
      else if(current_character == 'n' && json != NULL) {
        json_pair->data.null = parse_null(json_buffer, "null");

        json_pair->data_type = NUL;
        json_pair->value_exists = true;
        json_pair->current_value = false;
      }
      else if(current_character == ':' && json_pair->key_length > 0) {
        json_pair->current_value = true;
      }
      else if(current_character == ',' && json != NULL) {
        if(json_pair->value_exists && json_pair->key_length > 0) {
          json->objects.methods->insert(&json->objects,json_pair->key,json_pair->key_length,json_pair);
          json_pair = json_pair_create(arena);
        } 
        else {
          json_buffer->error = true;
          json_buffer->error_message = "A comma should not be here";
          break;
        }
      }
      else if(current_character == '}' && json != NULL) {
        if(json_pair->key_length > 0 && json_pair->value_exists) {
          json->objects.methods->insert(&json->objects,json_pair->key, json_pair->key_length,json_pair);
        }
        json->current_object = false;
        json_buffer->current_position++;
        json_buffer->current_col++;
        break;
      }
      else if(was_bracketed && !isspace(current_character)) {
        json_buffer->error = true;
        json_buffer->error_message = "JSON missing starting key";
        break;
      }
      else if(json_pair->key_length > 0 && !json_pair->value_exists && !isspace(current_character)) {
        json_buffer->error = true;
        json_buffer->error_message = "JSON key not properly closed or not followed by :";
        break;
      }
      else if(json_pair->value_exists && !isspace(current_character)) {
        json_buffer->error = true;
        json_buffer->error_message = "This value is either improperly formatted or misplaced";
        break;
      }
      else if(current_character == '\n') {
        json_buffer->current_position++;
        json_buffer->current_col = 1;
        json_buffer->current_row++;
        continue;
      }

      if(!json_buffer->error) {
        json_buffer->current_position++;
        json_buffer->current_col++;
      }
    }
    
    if(json == NULL && !json_buffer->error) {
      json_buffer->error = true;
      json_buffer->error_message = "No JSON detected in this buffer";
    }

    return json;
}

void parse_key(JSONBuffer* json_buffer, JSONPair* current_json, char* optional_key) {
  
  if(optional_key != NULL) {
    for(size_t i = 0; i < current_json->key_length; i++) {
      current_json->key[i] = optional_key[i];
    }
    return;
  }

  json_buffer->current_col++;
  json_buffer->current_position++;
  char current_character;
  bool was_escaped = false;

  while(true) {
    current_character = json_buffer->current_file[json_buffer->current_position];
  

    if(json_buffer->current_position + 1 == json_buffer->file_size) {
      json_buffer->error = true;
      json_buffer->error_message = "JSON key not properly closed";
      return;
    }
    else if(isescaped(current_character) && !was_escaped) {
      if(current_character != '"') {
        json_buffer->error = true;
        json_buffer->error_message = "Certain characters need to be escaped or key not closed";
        return;
      }
      else {
        break;
      }
    }
    else if(current_character == '\\') {
      was_escaped = true;
    }
    
    if(current_json->key_length < 256) {
      current_json->key[current_json->key_length] = current_character;
      current_json->key_length++;
      

      json_buffer->current_position++;
      json_buffer->current_col++;
    }
    else {
      json_buffer->error = true;
      json_buffer->error_message = "JSON key can not exceed 256 characters";
      return;
    }

    if(isescaped(current_character) && was_escaped) {
      was_escaped = false;
    } 
  }
}

LinkedList* parse_string(JSONBuffer* json_buffer, Arena* arena) {
  
  json_buffer->current_position++; 
  json_buffer->current_col++;
  LinkedList* string = linkedlist_create(arena);
  char current_character;
  bool was_escaped = false;

  while(true) {
    current_character = json_buffer->current_file[json_buffer->current_position];
    
    if(json_buffer->current_position == json_buffer->file_size-1) {
      json_buffer->error = true;
      json_buffer->error_message = "String not properly closed";
      return NULL;
    }
    else if(isescaped(current_character) && !was_escaped) {
      if(current_character != '"') {  
        json_buffer->error = true;
        json_buffer->error_message = "Certain characters need to be escaped or string not closed";
        return NULL;
      }
      else {
        break;
      }
    }
    else if(current_character == '\\') {
      was_escaped = true;
    }

    char* current_char = arena_alloc(arena, sizeof(char));
    current_char[0] = json_buffer->current_file[json_buffer->current_position];

    string->methods->insert(string, current_char);

    if(isescaped(current_character) && was_escaped) {
      was_escaped = false;
    }

    json_buffer->current_position++;
    json_buffer->current_col++;
  }
  
  return string;
}

JSONNum* parse_number(JSONBuffer* json_buffer, Arena* arena) {
  JSONNum* number = arena->alloc(arena, sizeof(JSONNum));
  number->length = 0;
  char current_character = json_buffer->current_file[json_buffer->current_position];

  while(isdigit(current_character) || isxdigit(current_character) || current_character == '.') {
    
    if(number->length <= 64) {
      number->value[number->length] = current_character;
      number->length++;
    }
    else {
      json_buffer->error = true;
      json_buffer->error_message = "Non-number or incorrect delimiter in number";
      return NULL;
    }
    
    json_buffer->current_position++;
    json_buffer->current_col++;
    current_character = json_buffer->current_file[json_buffer->current_position];
  }
  
  json_buffer->current_position--;
  json_buffer->current_col--;
  return number;
}


LinkedList* parse_array(JSONBuffer* json_buffer, Arena *arena) {
  LinkedList* array = linkedlist_create(arena);
  char current_character;
  
  bool was_comma = false;

  ArrayData* array_member = arena->alloc(arena, sizeof(ArrayData));

  while(true) {
    
    current_character = json_buffer->current_file[json_buffer->current_position];

    if(current_character == ']' || json_buffer->error) {
      break;
    }

    if((array->length == 0 || was_comma) && current_character != ',' && current_character != '\n') { 
      if(isdigit(current_character)) {
        array_member->data_type = NUM;
        array_member->data.num = parse_number(json_buffer, arena);
        array->methods->insert(array, array_member);

        array_member = arena->alloc(arena, sizeof(ArrayData));
        was_comma = false;
      }
      else if(current_character == '[') {
        array_member->data_type = ARRAY;
        json_buffer->current_position++;
        json_buffer->current_col++;
        array_member->data.array = parse_array(json_buffer, arena);
        array->methods->insert(array, array_member);

        array_member = arena->alloc(arena, sizeof(ArrayData));
        was_comma = false;
      }
      else if(current_character == '{') {
        array_member->data_type = JSON;
        array_member->data.json = parse_json(json_buffer,arena);
        array->methods->insert(array, array_member);
        json_buffer->current_col--;
        json_buffer->current_position--;

        array_member = arena->alloc(arena, sizeof(ArrayData));
        was_comma = false;
      }
      else if(current_character == '"') {
        array_member->data_type = STR;
        array_member->data.str = parse_string(json_buffer,arena);
        array->methods->insert(array, array_member);

        array_member = arena->alloc(arena, sizeof(ArrayData));
        was_comma = false;
      }
      else if(current_character == 't') {
        array_member->data_type = BOOL;
        array_member->data.boolean = parse_bool(json_buffer, arena, "true");
        array->methods->insert(array, array_member);

        array_member = arena->alloc(arena, sizeof(ArrayData));
        was_comma = false;
      }
      else if(current_character == 'f'){
        array_member->data_type = BOOL;
        array_member->data.boolean = parse_bool(json_buffer, arena, "false");
        array->methods->insert(array, array_member);

        array_member = arena->alloc(arena, sizeof(ArrayData));
        was_comma = false;
      }
      else if(current_character == 'n') {
        array_member->data_type = NUL;
        array_member->data.null = parse_null(json_buffer, "null");
        array->methods->insert(array, array_member);

        array_member = arena->alloc(arena, sizeof(ArrayData));
        was_comma = false;
      }
    }
    else if(current_character == ',' && array->length != 0) {
      was_comma = true;
    }
    else if(current_character == ',' && array->length == 0) {
      json_buffer->error = true;
      json_buffer->error_message = "Can not have comma in array before values";
      return NULL;
    }
    else if(!isspace(current_character)) {
      json_buffer->error = true;
      json_buffer->error_message = "Comma missing between values in array";
      return NULL;
    }
    else if(current_character == '\n') {
      json_buffer->current_position++;
      json_buffer->current_col = 1;
      json_buffer->current_row++;
      continue;
    }
    
    json_buffer->current_position++;
    json_buffer->current_col++;
  }

  return array;
}

JSONBool* parse_bool(JSONBuffer* json_buffer, Arena* arena, char boolean_value[]) {
  
  JSONBool* json_bool = arena->alloc(arena, sizeof(JSONBool));
  char current_character = json_buffer->current_file[json_buffer->current_position];
  
  int bool_length;

  if(current_character == 't') {
    bool_length = 4;
    json_bool->value = true;
  }
  else {
    bool_length = 5;
    json_bool->value = false;
  }

  for(int i = 0; i < bool_length; i++) {
    current_character = json_buffer->current_file[json_buffer->current_position];

    if(current_character != boolean_value[i]) {
      json_buffer->error = true;
      json_buffer->error_message = "Boolean value is likely misspelled";
      return NULL;
    }

    json_bool->parsed_value[i] = current_character;

    json_buffer->current_position++;
    json_buffer->current_col++;
  }

  json_buffer->current_position--;
  json_buffer->current_col--;
  return json_bool;
}

void* parse_null(JSONBuffer* json_buffer, char* null_string) {
  
  char current_character;

  for(int i = 0; i < 4; i++) {
    current_character = json_buffer->current_file[json_buffer->current_position];
    
    if(current_character != null_string[i]) {
      json_buffer->error = true;
      json_buffer->error_message = "Null value was likely misspelled";
      return NULL;
    }
    
  json_buffer->current_position++;
  json_buffer->current_col++;
  }

  json_buffer->current_position--;
  json_buffer->current_col--;
  return NULL;
}



