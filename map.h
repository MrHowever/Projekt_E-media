#ifndef MAP_H
#define MAP_H

struct Node
{
  unsigned char marker;
  char* descr;
  struct Node* next;
};

struct Map
{
  struct Node* first;
  struct Node* last;
};

void map_init(struct Map*);
void map_push(struct Map*, unsigned char, char*);
char* map_find(struct Map*, unsigned char);

#endif
