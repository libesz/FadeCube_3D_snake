#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "fadecube.h"

int create_udp_socket()
{
   int udp_socket;
   if( ( udp_socket = socket( AF_INET, SOCK_DGRAM, 0 ) ) < 0)
   {
#ifdef DEBUG
      puts( "DEBUG --- could not create client socket" );
#endif
      return -1;
   }
   return udp_socket;
}

struct sockaddr_in create_sockaddr( char *ip, unsigned short int port )
{
   struct sockaddr_in address;
   memset(&(address.sin_zero), 0, sizeof(address.sin_zero));

  	address.sin_family = AF_INET; // sets the server address to type AF_INET, similar to the socket we will use.
	address.sin_addr.s_addr = inet_addr( ip ); // this sets the server address.
	address.sin_port = htons( port );
	return address;
}

int send_frame_to_cube( int client_socket, struct sockaddr_in cube_address, cube_frame_t *cube_frame )
{
   unsigned char packet_payload[251]; //create buffer
   memset( packet_payload, 0, sizeof( packet_payload ) );

   packet_payload[0] = 1; //first byte is 1 -> means frame data for the cube

   memcpy( &packet_payload[1], cube_frame->leds, 250 ); //copy the frame data to the buffer

	if( sendto( client_socket, packet_payload, sizeof( packet_payload ), 0, ( struct sockaddr * ) &cube_address, sizeof( cube_address ) ) < 0 )
	{
#ifdef DEBUG
		puts( "DEBUG --- could not send data to the server");
#endif
		return -1;
	}
	return 1;
}

void set_led( cube_frame_t *frame, coord_t coordinate, unsigned char brightness )
{
   unsigned short int index;
   unsigned char inner_index, reversed_z = 9 - coordinate.z;
   index = ( reversed_z * 100 ) + ( coordinate.y * 10 ) + coordinate.x; //index in the 0-1023 range
   inner_index =  2 * ( 3 - ( index % 4 ) ); //the inner-byte index (0-3)
   index /= 4; //convert index to 0-250 range

   frame->leds[ index ] &= ~( 0b00000011 << inner_index );        //set the state of the coordinate to 0
   frame->leds[ index ] |= ( ( brightness & 3 ) << inner_index ); //set the new value
}

void fill_frame( cube_frame_t *cube_frame, unsigned char brightness )
{
   unsigned char fill_byte;
   switch( brightness )
   {
      case 1: fill_byte = 0b01010101;
         break;
      case 2: fill_byte = 0b10101010;
         break;
      case 3: fill_byte = 255;
         break;
      default: fill_byte = 0;
         break;
   }
   memset( cube_frame->leds, fill_byte, 250 );
}
