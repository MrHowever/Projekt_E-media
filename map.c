#include <stdlib.h>
#include "map.h"


void map_init(struct Map* map)
{
  map->first = 0;
}

void map_push(struct Map* map, uint8_t mark, char* descript)
{
  struct Node* new_node = (struct Node*) malloc(sizeof(struct Node));

  new_node->descr = descript;
  new_node->marker = mark;
  new_node->next = 0;

  map->last->next = new_node;
  map->last = new_node;
}

char* map_find(struct Map* map, uint8_t mark)
{
  struct Node* current = map->first;

  while(current != 0)
  {
    if(current->marker == mark)
      return current->descr;
    else
      current = current->next;
  }

  return 0;
}
