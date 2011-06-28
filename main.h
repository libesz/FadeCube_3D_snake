#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <netinet/in.h>
#include "fadecube.h"
#include "snake.h"

typedef struct {
   char            *user_direction;
   unsigned char *gameover;
} handle_user_params_t;

void *handle_user( handle_user_params_t * );

typedef struct {
   char            *user_direction;
   unsigned char *gameover;
   snake_node_t     **snake_head;
   coord_t          *food;
   pthread_mutex_t  *draw_mutex;
   pthread_cond_t   *draw_condition_var;
} handle_snake_params_t;

void *handle_snake( handle_snake_params_t * );

typedef struct {
   int             client_socket;
   struct         sockaddr_in cube_address;
   snake_node_t    **snake_head;
   coord_t         *food;
   unsigned char *gameover;
   pthread_mutex_t *draw_mutex;
   pthread_cond_t  *draw_condition_var;
} handle_render_params_t;

void *handle_render( handle_render_params_t * );

int mygetch( void );

#endif // MAIN_H_INCLUDED
