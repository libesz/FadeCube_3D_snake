#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>

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

int main()
{
   int client_socket;
   struct sockaddr_in cube_address;
   cube_frame_t cube_frame;
   char user_direction = FORWARD;

   int handle_user_thread_rc;
   pthread_t handle_user_thread;
   handle_user_params_t handle_user_params;

   int handle_snake_thread_rc;
   pthread_t handle_snake_thread;
   handle_snake_params_t handle_snake_params;

   int handle_render_thread_rc;
   pthread_t handle_handle_thread;
   handle_render_params_t handle_render_params;

#ifdef DEBUG
   puts( "DEBUG --- debug mode on" );
#endif
   puts( "Hello world!" );

   memset( &cube_frame, 0, sizeof( cube_frame ) );
   snake_node_t *snake_head = NULL;

   client_socket = create_udp_socket();
   cube_address = create_sockaddr( CUBE_IP, CUBE_PORT );

   fill_frame( &cube_frame, 0 );


   handle_user_params.user_direction = &user_direction;

   handle_snake_params.cube_frame_ref = &cube_frame;
   handle_snake_params.client_socket = client_socket;
   handle_snake_params.cube_address = cube_address;
   handle_snake_params.user_direction = &user_direction;
   handle_snake_params.snake_head = &snake_head;

   if( handle_snake_thread_rc = pthread_create( &handle_snake_thread, NULL, &handle_snake, (void *)&handle_snake_params ) )
   {
      printf("Unable to create snake handler thread, error: %d\n", handle_snake_thread_rc );
   }

   if( handle_user_thread_rc = pthread_create( &handle_user_thread, NULL, &handle_user, (void *)&handle_user_params ) )
   {
      printf("Unable to create user handler thread, error: %d\n", handle_user_thread_rc );
   }
   else
   {
      pthread_join( handle_user_thread, NULL );
   }

   return 0;
}

int handle_snake( handle_snake_params_t *params )
{
   coord_t next_coord;
   char something_happened = 0;
   char last_direction = *params->user_direction;
   char used_direction = *params->user_direction;
   unsigned char snake_length = 10, snake_actual_length = 0;

   snake_node_t *temp_snake_node;

   memset( &next_coord, 0, sizeof( next_coord ) );

   while(1)
   {
      something_happened = 0;
      //set_led( params->cube_frame_ref, next_coord, 0 );
      if( ( ( last_direction == FORWARD ) && ( *params->user_direction == BACKWARD ) ) ||
          ( ( last_direction == BACKWARD ) && ( *params->user_direction == FORWARD ) ) ||
          ( ( last_direction == LEFT ) && ( *params->user_direction == RIGHT ) ) ||
          ( ( last_direction == RIGHT ) && ( *params->user_direction == LEFT ) ) ||
          ( ( last_direction == UP ) && ( *params->user_direction == DOWN ) ) ||
          ( ( last_direction == DOWN ) && ( *params->user_direction == UP ) ) )
      {
         used_direction = last_direction;
      }
      else
      {
         used_direction = *params->user_direction;
         last_direction = used_direction;
      }
//      if( something_happened )

      switch( used_direction )
      {
         case FORWARD:
               if( next_coord.y < 9 )
               {
                  next_coord.y++;
                  something_happened = 1;
               }
            break;
         case BACKWARD:
               if( next_coord.y )
               {
                  next_coord.y--;
                  something_happened = 1;
               }
            break;
         case LEFT:
               if( next_coord.x < 9 )
               {
                  next_coord.x++;
                  something_happened = 1;
               }
            break;
         case RIGHT:
               if( next_coord.x )
               {
                  next_coord.x--;
                  something_happened = 1;
               }
            break;
         case UP:
               if( next_coord.z < 9 )
               {
                  next_coord.z++;
                  something_happened = 1;
               }
            break;
         case DOWN:
               if( next_coord.z )
               {
                  next_coord.z--;
                  something_happened = 1;
               }
            break;
      }

      if( something_happened )
      {
         snake_add( params->snake_head, next_coord );
         snake_actual_length = snake_count( *params->snake_head );
         if( ( snake_actual_length > snake_length ) && snake_actual_length )
         {
            snake_remove_last( params->snake_head );
         }
#ifdef DEBUG
         snake_print( *params->snake_head );
#endif
         fill_frame( params->cube_frame_ref, 0 );

         temp_snake_node = *params->snake_head;
         while( temp_snake_node )
         {
            set_led( params->cube_frame_ref, temp_snake_node->data, 3 );
            temp_snake_node = temp_snake_node->next;
         }
      }
      send_frame_to_cube( params->client_socket, params->cube_address, params->cube_frame_ref );
      usleep( 200000 );
   }
   return 1;
}

int handle_user( handle_user_params_t *params )
{
   while( 1 )
   {
      char my_char, something_happened = 0;
      my_char = mygetch();
#ifdef DEBUG
      printf( "%c ", my_char );
#endif
      switch( my_char )
      {
         case 27: return( 0 );
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
   return 1;
}

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
