#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

#include "fadecube.h"
#include "main.h"
#include "snake.h"

#define CUBE_IP   "192.168.1.99"
#define CUBE_PORT 1125

#define UP       0
#define DOWN     1
#define RIGHT    2
#define LEFT     3
#define FORWARD  4
#define BACKWARD 5

#define FOODTIMEOUT 30
#define MOVE_TIME   200
#define SNAKE_INIT_LENGTH 9

int main()
{
   unsigned char gameover = 0;
#ifdef DEBUG
   puts( "DEBUG --- debug mode on" );
#endif
   puts( "Hello world!" );

   do
   {
      int client_socket;
      struct sockaddr_in cube_address;
      char user_direction = FORWARD;
      coord_t food;
      snake_node_t *snake_head = NULL;

      pthread_mutex_t gameover_mutex         = PTHREAD_MUTEX_INITIALIZER;
      pthread_cond_t  gameover_condition_var = PTHREAD_COND_INITIALIZER;

      pthread_mutex_t draw_mutex         = PTHREAD_MUTEX_INITIALIZER;
      pthread_cond_t  draw_condition_var = PTHREAD_COND_INITIALIZER;

      int handle_user_thread_rc;
      pthread_t handle_user_thread;
      handle_user_params_t handle_user_params;

      int handle_snake_thread_rc;
      pthread_t handle_snake_thread;
      handle_snake_params_t handle_snake_params;

      int handle_render_thread_rc;
      pthread_t handle_render_thread;
      handle_render_params_t handle_render_params;

      client_socket = create_udp_socket();
      cube_address = create_sockaddr( CUBE_IP, CUBE_PORT );

      handle_user_params.user_direction = &user_direction;
      handle_user_params.gameover = &gameover;

      handle_snake_params.user_direction = &user_direction;
      handle_snake_params.snake_head = &snake_head;
      handle_snake_params.food = &food;
      handle_snake_params.gameover = &gameover;
      handle_snake_params.draw_mutex = &draw_mutex;
      handle_snake_params.draw_condition_var = &draw_condition_var;

      handle_render_params.client_socket = client_socket;
      handle_render_params.cube_address = cube_address;
      handle_render_params.snake_head = &snake_head;
      handle_render_params.food = &food;
      handle_render_params.gameover = &gameover;
      handle_render_params.draw_mutex = &draw_mutex;
      handle_render_params.draw_condition_var = &draw_condition_var;

      gameover = 0;
#ifdef DEBUG
      printf("thread 1, %d\n", gameover);
#endif
      if( ( handle_render_thread_rc = pthread_create( &handle_render_thread, NULL, (void *)handle_render, &handle_render_params ) ) )
      {
         printf("Unable to create render handler thread, error: %d\n", handle_render_thread_rc );
      }

      if( ( handle_snake_thread_rc = pthread_create( &handle_snake_thread, NULL, (void *)handle_snake, &handle_snake_params ) ) )
      {
         printf("Unable to create snake handler thread, error: %d\n", handle_snake_thread_rc );
      }

      if( ( handle_user_thread_rc = pthread_create( &handle_user_thread, NULL, (void *)handle_user, &handle_user_params ) ) )
      {
         printf("Unable to create user handler thread, error: %d\n", handle_user_thread_rc );
      }
#ifdef DEBUG
      printf("thread 2, %d\n", gameover);
#endif
      pthread_join( handle_snake_thread, NULL );
#ifdef DEBUG
      printf("thread 3, %d\n", gameover);
#endif
      pthread_join( handle_render_thread, NULL );
#ifdef DEBUG
      printf("thread 4, %d\n", gameover);
#endif
      pthread_join( handle_user_thread, NULL );
#ifdef DEBUG
      printf("thread 5, %d\n", gameover);
#endif
   } while( gameover != 2 );
   return 0;
}

//render thread function, always blocked while something happened which modifies the cube data
void *handle_render( handle_render_params_t *params )
{
   cube_frame_t cube_frame;
   snake_node_t *temp_snake_node;

   memset( &cube_frame, 0, sizeof( cube_frame ) );

   fill_frame( &cube_frame, 0 );
   send_frame_to_cube( params->client_socket, params->cube_address, &cube_frame ); //draw an empty frame

   while( 1 )
   {
      pthread_mutex_lock( params->draw_mutex ); //lock the draw mutex

      pthread_cond_wait( params->draw_condition_var, params->draw_mutex ); //unlock the mutex until there is something to draw

      if( *params->gameover )
      {
         pthread_mutex_unlock( params->draw_mutex ); //release the mutex
         return NULL;
      }

      fill_frame( &cube_frame, 0 ); //start the rendering

      if( params->snake_head )
      {
         temp_snake_node = *params->snake_head; //render the snake START
         while( temp_snake_node )
         {
            set_led( &cube_frame, temp_snake_node->data, 3 );
            temp_snake_node = temp_snake_node->next;
         } //render the snake END

         set_led( &cube_frame, *params->food, 3 ); //render the food
      }
      pthread_mutex_unlock( params->draw_mutex ); //release the mutex
      send_frame_to_cube( params->client_socket, params->cube_address, &cube_frame );
   }
}

//snake handler, calculates the snake parts in a chained list
void *handle_snake( handle_snake_params_t *params )
{
   coord_t next_coord;
   char can_move = 0; //currenctly the snake stops at the wall, when this happens, this var indicates it
   char last_direction = *params->user_direction;
   char used_direction = *params->user_direction;
   unsigned char snake_length = SNAKE_INIT_LENGTH, snake_actual_length = 0;
   unsigned char food_timer = 0;

   memset( &next_coord, 0, sizeof( next_coord ) );

   puts( "Game started");

   while(1)
   {
      can_move = 0;

      //deny the direction update if the user pressed the opposite
      if( !( ( ( last_direction == FORWARD ) && ( *params->user_direction == BACKWARD ) ) ||
             ( ( last_direction == BACKWARD ) && ( *params->user_direction == FORWARD ) ) ||
             ( ( last_direction == LEFT ) && ( *params->user_direction == RIGHT ) ) ||
             ( ( last_direction == RIGHT ) && ( *params->user_direction == LEFT ) ) ||
             ( ( last_direction == UP ) && ( *params->user_direction == DOWN ) ) ||
             ( ( last_direction == DOWN ) && ( *params->user_direction == UP ) ) ) )
      {
         used_direction = *params->user_direction;
         last_direction = used_direction;
      }

      switch( used_direction ) //calculate the next (from the last added) HEAD item coordinate for the snake
      {
         case FORWARD:
               if( next_coord.y < 9 )
               {
                  next_coord.y++;
                  can_move = 1;
               }
            break;
         case BACKWARD:
               if( next_coord.y )
               {
                  next_coord.y--;
                  can_move = 1;
               }
            break;
         case LEFT:
               if( next_coord.x < 9 )
               {
                  next_coord.x++;
                  can_move = 1;
               }
            break;
         case RIGHT:
               if( next_coord.x )
               {
                  next_coord.x--;
                  can_move = 1;
               }
            break;
         case UP:
               if( next_coord.z < 9 )
               {
                  next_coord.z++;
                  can_move = 1;
               }
            break;
         case DOWN:
               if( next_coord.z )
               {
                  next_coord.z--;
                  can_move = 1;
               }
            break;
      }

      pthread_mutex_lock( params->draw_mutex ); //lock the mutex while updating the snake (avoid render thread to read now)
      if( can_move ) //so this var indicates when there is a place to move forward
      {
         if( !( food_timer ) )
         {
#ifdef DEBUG
            puts( "new food\n" );
#endif
            make_food( params->food );
         }
         if( ++food_timer == FOODTIMEOUT ) food_timer = 0;

         snake_add( params->snake_head, next_coord );

         if( !( memcmp( &((**params->snake_head).data), params->food, sizeof( coord_t ) ) ) ) //detect if the snake has just hit the food
         {
            snake_length++;
            food_timer = 0;
#ifdef DEBUG
            puts( "got it!\n" );
#endif
         }

         snake_actual_length = snake_count( *params->snake_head );
         if( ( snake_actual_length > snake_length ) && snake_actual_length ) //if the snake was not currently growing, remove the last item
         {                                                                   //therefore it will look like moving
            snake_remove_last( params->snake_head );
         }
#ifdef DEBUG
         snake_print( *params->snake_head );
#endif
         pthread_cond_signal( params->draw_condition_var ); //call render thread that there is something new to draw
         pthread_mutex_unlock( params->draw_mutex );
      }
      else
      {
         while( (**params->snake_head).next )
         {
            snake_remove_last( params->snake_head );
#ifdef DEBUG
            //printf( "a" );
#endif
         }
         free( *params->snake_head );
         *params->gameover = 1;
         printf( "Game over!\nYour score is %d.\n\nPress ESC to quit or any to start new game!\n", ( snake_actual_length - SNAKE_INIT_LENGTH ) * 10 );
         fflush(stdout);
         pthread_cond_signal( params->draw_condition_var ); //call render thread that there is something new to draw
         pthread_mutex_unlock( params->draw_mutex );
         return NULL;
      }

      if( *params->gameover )
      {
         return NULL;
      }

      usleep( MOVE_TIME * 1000 ); //sleep the snake thread, this represents the speed
   }
}

//user thread, it does nothing but always waits for a keyboard press
void *handle_user( handle_user_params_t *params )
{
   while( 1 )
   {
      char my_char;
      my_char = mygetch();
#ifdef DEBUG
      printf( "%d ", my_char );
#endif
      if( my_char == 27 )
      {
         *params->gameover = 2;
         return NULL;
      }
      if( *params->gameover )
      {
         return NULL;
      }

      switch( my_char )
      {
         case 'w':
               *params->user_direction = FORWARD;
            break;
         case 's':
               *params->user_direction = BACKWARD;
            break;
         case 'a':
               *params->user_direction = LEFT;
            break;
         case 'd':
               *params->user_direction = RIGHT;
            break;
         case '\'':
               *params->user_direction = UP;
            break;
         case '/':
               *params->user_direction = DOWN;
            break;
      }
   }
}

//the magic function, does the same as getch should
int mygetch(void)
{
   struct termios oldt,
   newt;
   int ch;
   tcgetattr( STDIN_FILENO, &oldt );
   newt = oldt;
   newt.c_lflag &= ~( ICANON | ECHO );
   tcsetattr( STDIN_FILENO, TCSANOW, &newt );
   ch = getchar();
   tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
   return ch;
}
