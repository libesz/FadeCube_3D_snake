#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <netinet/in.h>
#include "fadecube.h"

typedef struct {
   char *user_direction;
} handle_user_params_t;

int handle_user( handle_user_params_t * );

typedef struct {
   cube_frame_t *cube_frame_ref;
   int client_socket;
   struct sockaddr_in cube_address;
   char *user_direction;
} handle_snake_params_t;

int handle_snake( handle_snake_params_t * );

int mygetch( void );

#endif // MAIN_H_INCLUDED
