#include <stdlib.h>
#include "map.h"
#include <stdio.h>


void map_init(struct Map* map)
{
  map->first = 0;
  map->last = 0;
}

void map_destroy(struct Map* map)
{
  struct Node* current, *temp;

  current = map->first;
  
  while(current != 0)
  {
    temp = current->next;
    free(current);
    current = temp;
  }
}

void map_push(struct Map* map, uint16_t mark, char* descript)
{
  struct Node* new_node = (struct Node*) malloc(sizeof(struct Node));
   
  new_node->descr = descript;
  new_node->marker = mark;
  new_node->next = 0;

  if(map->first == 0)
    map->first = new_node;
  
  if(map->last != 0) {
    map->last->next = new_node;
    map->last = new_node;
  }
  else {
    map->last = new_node;
  }
}

char* map_find(struct Map* map, uint16_t mark)
{
  struct Node* current = map->first;

  while(current != 0)
  {
    if(current->marker == mark)
      return current->descr;
    else
      current = current->next;
  }

  return "Non-standard tag";
}
