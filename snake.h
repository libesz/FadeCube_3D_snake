#ifndef SNAKE_H_INCLUDED
#define SNAKE_H_INCLUDED

#include "fadecube.h"

typedef struct ns
{
   struct coord data;
   struct ns *next;
} snake_node_t;

snake_node_t *snake_add( snake_node_t **, coord_t );

void snake_remove_last( snake_node_t ** );

int snake_count( snake_node_t * );

#ifdef DEBUG
void snake_print( snake_node_t * );
#endif

#endif // SNAKE_H_INCLUDED
