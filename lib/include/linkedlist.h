#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdlib.h>
#include <stdbool.h>
#include "arena.h"

typedef struct Node Node;

struct Node {
  void* value;
  Node* next;
  Node* previous;
};

typedef struct LinkedList LinkedList;
typedef struct linked_list_operations linked_list_operations;

struct LinkedList { 
  size_t length;
  Node* head;
  Node* tail;
  const linked_list_operations* methods;
  Arena* arena;
  };

struct linked_list_operations {
  void (*insert) (LinkedList* self, void* elt);
  void (*insert_at) (LinkedList* self, size_t index, void* elt);
  void* (*pop) (LinkedList* self);
  void* (*remove_at) (LinkedList* self, size_t index);
  void* (*get) (LinkedList* self, size_t index);
};


void linked_list_init(LinkedList* self, Arena* arena);

#endif
