#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>

#include "fadecube.h"
#include "main.h"

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
#ifdef DEBUG
   puts( "DEBUG --- debug mode on" );
#endif

   int client_socket;
   struct sockaddr_in cube_address;
   cube_frame_t cube_frame;
   handle_user_params_t handle_user_params;
   handle_snake_params_t handle_snake_params;
   char direction = FORWARD;

   int handle_user_thread_rc;
   pthread_t handle_user_thread;
   int handle_snake_thread_rc;
   pthread_t handle_snake_thread;

   memset( &cube_frame, 0, sizeof( cube_frame ) );

   client_socket = create_udp_socket();
   cube_address = create_sockaddr( CUBE_IP, CUBE_PORT );

   fill_frame( &cube_frame, 0 );

   puts( "Hello world!" );

   handle_user_params.cube_frame_ref = &cube_frame;
   handle_user_params.client_socket = client_socket;
   handle_user_params.cube_address = cube_address;
   handle_user_params.current_direction = &direction;

   handle_snake_params.cube_frame_ref = &cube_frame;
   handle_snake_params.client_socket = client_socket;
   handle_snake_params.cube_address = cube_address;
   handle_snake_params.current_direction = &direction;

   if( handle_snake_thread_rc = pthread_create( &handle_snake_thread, NULL, &handle_snake, (void *)&handle_snake_params ) )
   {
      printf("Unable to create snake handler thread, error: %d\n", handle_snake_thread_rc );
   }
   /*else
   {
      pthread_join( handle_snake_thread, NULL );
   }*/

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
   coord_t cube_coord;
   char something_happened = 0;
   char last_direction = *params->current_direction;
   char used_direction = *params->current_direction;

   memset( &cube_coord, 0, sizeof( cube_coord ) );

   while(1)
   {
      something_happened = 0;
      set_led( params->cube_frame_ref, cube_coord, 0 );
      if( ( ( last_direction == FORWARD ) && ( *params->current_direction == BACKWARD ) ) ||
          ( ( last_direction == BACKWARD ) && ( *params->current_direction == FORWARD ) ) ||
          ( ( last_direction == LEFT ) && ( *params->current_direction == RIGHT ) ) ||
          ( ( last_direction == RIGHT ) && ( *params->current_direction == LEFT ) ) ||
          ( ( last_direction == UP ) && ( *params->current_direction == DOWN ) ) ||
          ( ( last_direction == DOWN ) && ( *params->current_direction == UP ) ) )
      {
         used_direction = last_direction;
      }
      else
      {
         used_direction = *params->current_direction;
         last_direction = used_direction;
      }
      switch( used_direction )
      {
         case FORWARD:
               if( cube_coord.y < 9 )
               {
                  cube_coord.y++;
                  something_happened = 1;
               }
            break;
         case BACKWARD:
               if( cube_coord.y )
               {
                  cube_coord.y--;
                  something_happened = 1;
               }
            break;
         case LEFT:
               if( cube_coord.x < 9 )
               {
                  cube_coord.x++;
                  something_happened = 1;
               }
            break;
         case RIGHT:
               if( cube_coord.x )
               {
                  cube_coord.x--;
                  something_happened = 1;
               }
            break;
         case UP:
               if( cube_coord.z < 9 )
               {
                  cube_coord.z++;
                  something_happened = 1;
               }
            break;
         case DOWN:
               if( cube_coord.z )
               {
                  cube_coord.z--;
                  something_happened = 1;
               }
            break;
      }
      set_led( params->cube_frame_ref, cube_coord, 3 );
      if( something_happened )send_frame_to_cube( params->client_socket, params->cube_address, params->cube_frame_ref );
      usleep(200000);
   }
   return 1;
}

int handle_user( handle_user_params_t *params )
{
   while( 1 )
   {
      char my_char, something_happened = 0;
      my_char = mygetch();
      printf( "%c ", my_char );

      switch( my_char )
      {
         case 27: return( 0 );
         case 'w':
               *params->current_direction = FORWARD;
            break;
         case 's':
               *params->current_direction = BACKWARD;
            break;
         case 'a':
               *params->current_direction = LEFT;
            break;
         case 'd':
               *params->current_direction = RIGHT;
            break;
         case '\'':
               *params->current_direction = UP;
            break;
         case '/':
               *params->current_direction = DOWN;
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
