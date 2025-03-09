#ifndef INPUT_H
#define INPUT_H

#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#include <pthread.h>

#include <string.h>

/*
           ____________________________              __
         / [__ZL__]          [__ZR__] \               |
        / [__ TL __]        [__ TR __] \              | Front Triggers
     __/________________________________\__         __|
    /                                  _   \          |
   /      /\           __             (N)   \         |
  /       ||      __  |MO|  __     _       _ \        | Main Pad
 |    <===DP===> |SE|      |ST|   (W) -|- (E) |       |
  \       ||    ___          ___       _     /        |
  /\      \/   /   \        /   \     (S)   /\      __|
 /  \________ | LS  | ____ |  RS | ________/  \       |
|         /  \ \___/ /    \ \___/ /  \         |      | Control Sticks
|        /    \_____/      \_____/    \        |    __|
|       /                              \       |
 \_____/                                \_____/

     |________|______|    |______|___________|
       D-Pad    Left       Right   Action Pad
               Stick       Stick

                 |_____________|
                    Menu Pad
*/

typedef struct{
  int lock;
  int fd;
  char south;
  char east;
  char c;
  char north;
  char west;
  char z;
  char tl;
  char tr;
  char tl2;
  char tr2;
  char select;
  char start;
  char mode;
  char thumbl;
  char thumbr;
  char hat0x;
  char hat0y;
  int abs_x;
  int abs_y;
  int abs_rx;
  int abs_ry;
}Gamepad;

// write to the temp and write the temp to the main when polled
typedef struct{
  pthread_t thread;
  pthread_rwlock_t lock;
  int fd;
  int x;
  int y;
  int z;
  int rx;
  int ry;
  int rz;
  int btn_0;
  int btn_1;

  int tmp_x;
  int tmp_y;
  int tmp_z;
  int tmp_rx;
  int tmp_ry;
  int tmp_rz;
  int tmp_btn_0;
  int tmp_btn_1;
}Spacemouse;
// range appears to be 350 for xyz and for rot
// multiply this by the delta time to get the relative
//

void* spacemouseThreadFunc(void* arg);

Spacemouse* initSpacemouse();

void pollSpacemouse(Spacemouse* spacemouse);

void termSpacemouse(Spacemouse** spacemouse);
#endif
