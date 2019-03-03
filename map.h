#ifndef MAP_H
#define MAP_H

#include <stdint.h>

struct Node
{
  uint16_t marker;
  char* descr;
  struct Node* next;
};

struct Map
{
  struct Node* first;
  struct Node* last;
};

void map_init(struct Map*);
void map_push(struct Map*, uint16_t, char*);
char* map_find(struct Map*, uint16_t);

#endif
