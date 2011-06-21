#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <netinet/in.h>

typedef struct {
   cube_frame_t *cube_frame_ref;
   int client_socket;
   struct sockaddr_in cube_address;
} handle_user_params_t;

int mygetch( void );

int handle_user( handle_user_params_t * );

#endif // MAIN_H_INCLUDED
