#ifndef FADECUBE_H_INCLUDED
#define FADECUBE_H_INCLUDED

typedef struct coord {
    char x;
    char y;
    char z;
} coord_t;

typedef struct cube_frame {
    unsigned char leds[250];
} cube_frame_t;

int create_udp_socket( void );

struct sockaddr_in create_sockaddr( char *, unsigned short int );

int send_frame_to_cube( int client_socket, struct sockaddr_in cube_address, cube_frame_t *cube_frame );

void set_led( cube_frame_t *, coord_t , unsigned char );

void fill_frame( cube_frame_t *, unsigned char );

#endif // FADECUBE_H_INCLUDED
