#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "fadecube.h"

#define CUBE_IP   "192.168.1.99"
#define CUBE_PORT 1125

int handle_user( cube_frame_t *, int , struct sockaddr_in );

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

int main()
{
#ifdef DEBUG
   puts( "DEBUG --- debug mode on" );
#endif

   int client_socket;
   struct sockaddr_in cube_address;
   cube_frame_t cube_frame;

   memset( &cube_frame, 0, sizeof( cube_frame ) );

   client_socket = create_udp_socket();
   cube_address = create_sockaddr( CUBE_IP, CUBE_PORT );

   fill_frame( &cube_frame, 0 );

   puts( "Hello world!" );
   handle_user( &cube_frame, client_socket, cube_address );
   return 0;
}


int handle_user( cube_frame_t *cube_frame_ref, int client_socket, struct sockaddr_in cube_address )
{
   coord_t cube_coord;
   memset( &cube_coord, 0, sizeof( cube_coord ) );

   set_led( cube_frame_ref, cube_coord, 3 );
   send_frame_to_cube( client_socket, cube_address, cube_frame_ref );

   while( 1 )
   {
      char my_char, something_happened = 0;
      my_char = mygetch();
      printf( "%c ", my_char );

      set_led( cube_frame_ref, cube_coord, 0 );
      switch( my_char )
      {
         case 27: return( 0 );
         case 'w':
               if( cube_coord.y < 9 )
               {
                  cube_coord.y++;
                  something_happened = 1;
               }
            break;
         case 's':
               if( cube_coord.y )
               {
                  cube_coord.y--;
                  something_happened = 1;
               }
            break;
         case 'a':
               if( cube_coord.x < 9 )
               {
                  cube_coord.x++;
                  something_happened = 1;
               }
            break;
         case 'd':
               if( cube_coord.x )
               {
                  cube_coord.x--;
                  something_happened = 1;
               }
            break;
         case '\'':
               if( cube_coord.z < 9 )
               {
                  cube_coord.z++;
                  something_happened = 1;
               }
            break;
         case '/':
               if( cube_coord.z )
               {
                  cube_coord.z--;
                  something_happened = 1;
               }
            break;
      }
      set_led( cube_frame_ref, cube_coord, 3 );
      if( something_happened ) send_frame_to_cube( client_socket, cube_address, cube_frame_ref );
   }
   return 1;
}
