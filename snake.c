#include "snake.h"
#include <stdlib.h>
#include <stdio.h>

void *snake_add( snake_node_t ** head, coord_t data )
{
   snake_node_t *new_head = malloc( sizeof(*new_head) );
   new_head->next = *head;
   new_head->data = data;
   *head = new_head;
}

void snake_remove_last( snake_node_t ** head )
{
   snake_node_t *temp_snake_newlast = *head;

   while( temp_snake_newlast->next->next )
   {
      temp_snake_newlast = temp_snake_newlast->next;
   }
   free( temp_snake_newlast->next );
   temp_snake_newlast->next = NULL;
}

int snake_count( snake_node_t * head )
{
   int i = 0;
   if( !head ) printf("snake is 0 long\n");
   while( head )
   {
      i++;
      head = head->next;
   }
   return i;
}

void make_food( coord_t *food )
{
   unsigned int iseed = (unsigned int)time(NULL);
   srand (iseed);

   food->x = rand() % 10;
   food->y = rand() % 10;
   food->z = rand() % 10;
}

#ifdef DEBUG
void snake_print( snake_node_t * head )
{
   int i = 0;
   if( !head ) printf( "snake is 0 long\n" );
   while( head )
   {
      i++;
      //printf("%p %p x:%i y:%i z:%i ", n, n->next, n->data.x, n->data.y, n->data.z );
      printf("x:%i y:%i z:%i | ", head->data.x, head->data.y, head->data.z );
      head = head->next;
   }
   printf( "length: %i\n\n", i );
}
#endif
