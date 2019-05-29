#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#ifdef __MINGW32__
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif // __MINGW32__
#include "fadecube.h"
#include "snake.h"

#endif // MAIN_H_INCLUDED
