#inlcude<stdio.h>

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

void map_init(Map& map)
{
  map.first = NULL;
}

void map_push(Map& map, unsigned char mark, char* descript)
{
  struct Node* new_node = (Node*) malloc(sizeof(Node));

  new_node.descr = descript;
  new_node.marker = mark;
  new_node.next = NULL;

  map.last->next = new_node;
  map.last = new_node;
}

char* map_find(Map& map, unsigned char mark)
{
  struct Node* current = map.first;

  while(current != NULL)
  {
    if(current->marker == mark)
      return current->descr;
    else
      current = current->next;
  }

  return NULL;
}
