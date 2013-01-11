#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <math.h>

#include "fadecube.h"
#include "main.h"

#define CUBE_IP   "192.168.1.100"
#define CUBE_PORT 1125

#define GROW_INTERVAL 0.1
#define R_MIN 0.8
#define R_MAX 4.5

#define GROW 1
#define SHRINK 0

int main()
{
#ifdef DEBUG
   puts( "DEBUG --- debug mode on" );
#endif
   puts( "Hello world!" );

   int client_socket;
   struct sockaddr_in cube_address;

   unsigned int usecs = 30000;

   unsigned char grow_dir = 1;

   float r = 4, t, s, rtemp=0.8;

   coord_t point;
   cube_frame_t cube_frame;

   client_socket = create_udp_socket();
   cube_address = create_sockaddr( CUBE_IP, CUBE_PORT );

   fill_frame( &cube_frame, 0 );

   while(1)
   {
      if( grow_dir )
      {
         rtemp += GROW_INTERVAL;
         if(rtemp > R_MAX )
         {
            grow_dir = SHRINK;
         }
      }
      else
      {
         rtemp -= GROW_INTERVAL;
         if( rtemp <= R_MIN  )
         {
            grow_dir = GROW;
         }
      }
      printf("rtemp: %f \n", rtemp);


      fill_frame( &cube_frame, 0 );

      for( r = 0; r < rtemp; r+=GROW_INTERVAL ) // kitoltott gomb
      {
         for( t = 0; t < 10; t++ ) // 10 layer
         {
            for( s = 0; s < 360; s+=2)
            {
               point.x = r * cos(s) * sin(t) + 5;
               point.y = r * sin(s) * sin(t) + 5;
               point.z = r * cos(t) + 5;

               set_led(&cube_frame, point, 3);
            }
         }
      }
      send_frame_to_cube( client_socket, cube_address, &cube_frame );
      usleep(usecs);
   }

   return 0;
}

