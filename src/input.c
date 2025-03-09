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

void* spacemouseThreadFunc(void* arg){
  // start the thread
  Spacemouse* spacemouse = (Spacemouse*)arg;
	struct input_event e;
	while(1){
		if(read(spacemouse->fd, &e, sizeof(struct input_event))>0){
		//printf("type: %x, code: %x, value: %x\n", e.type, e.code, e.value);
      // TODO convert input into state
      // get write lock
      // TODO setup input buffering?
      pthread_rwlock_wrlock(&spacemouse->lock);
      if(e.type == EV_KEY){
        switch(e.code){
  				case BTN_0:
						spacemouse->tmp_btn_0 = e.value;
						break;
  				case BTN_1:
						spacemouse->tmp_btn_1 = e.value;
						break;
        }
      }

      if(e.type == EV_REL){
        switch(e.code){
  				case REL_X:
						spacemouse->tmp_x = e.value;
						break;
  				case REL_Y:
						spacemouse->tmp_y = e.value;
						break;
  				case REL_Z:
						spacemouse->tmp_z = e.value;
						break;
  				case REL_RX:
						spacemouse->tmp_rx = e.value;
						break;
  				case REL_RY:
						spacemouse->tmp_ry = e.value;
						break;
  				case REL_RZ:
						spacemouse->tmp_rz = e.value;
						break;
        }
      }
      // release write lock
      pthread_rwlock_unlock(&spacemouse->lock);
		}
	}
}

Spacemouse* initSpacemouse(){
  // TODO make a selector code
  Spacemouse* spacemouse = malloc(sizeof(Spacemouse));
  pthread_rwlock_init(&spacemouse->lock, NULL);
	spacemouse->fd = open ("/dev/input/event6", O_RDONLY);
  pthread_create(&spacemouse->thread, NULL, spacemouseThreadFunc, spacemouse);
  return spacemouse;
}

void pollSpacemouse(Spacemouse* spacemouse){
  if(spacemouse == NULL){
    return;
  }
  // get lock
  pthread_rwlock_rdlock(&spacemouse->lock);
  spacemouse->x = spacemouse->tmp_x;
  spacemouse->y = spacemouse->tmp_y;
  spacemouse->z = spacemouse->tmp_z;
  spacemouse->rx = spacemouse->tmp_rx;
  spacemouse->ry = spacemouse->tmp_ry;
  spacemouse->rz = spacemouse->tmp_rz;
  // release lock
  pthread_rwlock_unlock(&spacemouse->lock);
}

// double pointer to set the pointer to be null
void termSpacemouse(Spacemouse** spacemouse){
  if(*spacemouse == NULL){
    return;
  }
  close((*spacemouse)->fd);
  // close lock
  // close file
  // kill thread
}

void initGamepad(Gamepad* gamepad){
  // open the file
  // remove lock if exists
  // initialize lock
  

}

void pollGamepad(Gamepad* gamepad){
  // read in data
  // get lock
  // update struct
  // release lock

}

void termGamepad(Gamepad* gamepad){
  // remove lock
  // close file descriptor
}
/*
int main(){
  /*
  Spacemouse* spacemouse = initSpacemouse();
  while(1){
    pollSpacemouse(spacemouse);
    printf("\r");
  	printf("x: %04i,\t", spacemouse->x);
  	printf("y: %04i,\t", spacemouse->y);
  	printf("z: %04i,\t", spacemouse->z);
  	printf("rx: %04i,\t", spacemouse->rx);
  	printf("ry: %04i,\t", spacemouse->ry);
  	printf("rz: %04i,\t", spacemouse->rz);
  	printf("btn_0: %04i,\t", spacemouse->btn_0);
  	printf("btn_1: %04i", spacemouse->btn_1);
  }
  return 0;
}
*/
