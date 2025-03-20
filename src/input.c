#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#include <pthread.h>

#include <string.h>
// TODO make a device picker
// TODO find a better way to initialize structs
// TODO implement proper terminations

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
  pthread_t thread;
  pthread_rwlock_t lock;
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
  int abs_hat0x;
  int abs_hat0y;
  char tmp_south;
  char tmp_east;
  char tmp_c;
  char tmp_north;
  char tmp_west;
  char tmp_z;
  char tmp_tl;
  char tmp_tr;
  char tmp_tl2;
  char tmp_tr2;
  char tmp_select;
  char tmp_start;
  char tmp_mode;
  char tmp_thumbl;
  char tmp_thumbr;
  char tmp_hat0x;
  char tmp_hat0y;
  int tmp_abs_x;
  int tmp_abs_y;
  int tmp_abs_rx;
  int tmp_abs_ry;
  int tmp_abs_hat0x;
  int tmp_abs_hat0y;
}Gamepad;

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
void* gamepadThreadFunc(void* arg){
  // start the thread
  Gamepad* gamepad = (Gamepad*)arg;
	struct input_event e;
	while(1){
		if(read(gamepad->fd, &e, sizeof(struct input_event))>0){
		//printf("type: %x, code: %x, value: %x\n", e.type, e.code, e.value);
      // TODO convert input into state
      // get write lock
      // TODO setup input buffering?
      pthread_rwlock_wrlock(&gamepad->lock);
      if(e.type == EV_KEY){
        //TODO add dpad
        switch(e.code){
					case BTN_SOUTH:
						gamepad->tmp_south = e.value;
						break;
					case BTN_EAST:
						gamepad->tmp_east = e.value;
						break;
					case BTN_C:
						gamepad->tmp_c = e.value;
						break;
					case BTN_NORTH:
						gamepad->tmp_north = e.value;
						break;
					case BTN_WEST:
						gamepad->tmp_west = e.value;
						break;
					case BTN_Z:
						gamepad->tmp_z = e.value;
						break;
					case BTN_TL:
						gamepad->tmp_tl = e.value;
						break;
					case BTN_TR:
						gamepad->tmp_tr = e.value;
						break;
					case BTN_TL2:
						gamepad->tmp_tl2 = e.value;
						break;
					case BTN_TR2:
						gamepad->tmp_tr2 = e.value;
						break;
					case BTN_SELECT:
						gamepad->tmp_select = e.value;
						break;
					case BTN_START:
						gamepad->tmp_start = e.value;
						break;
					case BTN_MODE:
						gamepad->tmp_mode = e.value;
						break;
					case BTN_THUMBL:
						gamepad->tmp_thumbl = e.value;
						break;
					case BTN_THUMBR:
						gamepad->tmp_thumbr = e.value;
						break;
        }
      }
      if(e.type == EV_ABS){
        //TODO add dpad
        switch(e.code){
					case ABS_X:
						gamepad->tmp_abs_x = e.value;
						break;
					case ABS_Y:
						gamepad->tmp_abs_y = e.value;
						break;
					case ABS_RX:
						gamepad->tmp_abs_rx = e.value;
						break;
					case ABS_RY:
						gamepad->tmp_abs_ry = e.value;
						break;
					case ABS_HAT0X:
						gamepad->tmp_abs_hat0x = e.value;
						break;
					case ABS_HAT0Y:
						gamepad->tmp_abs_hat0y = e.value;
						break;
        }
      }
      pthread_rwlock_unlock(&gamepad->lock);
		}
	}
}
Gamepad* initgamepad(){
  // TODO make a selector code
  Gamepad* gamepad = malloc(sizeof(Gamepad));
  pthread_rwlock_init(&gamepad->lock, NULL);
	gamepad->fd = open ("/dev/input/event14", O_RDONLY);
  pthread_create(&gamepad->thread, NULL, gamepadThreadFunc, gamepad);
	gamepad->tmp_south = 0;
	gamepad->tmp_east = 0;
	gamepad->tmp_c = 0;
	gamepad->tmp_north = 0;
	gamepad->tmp_west = 0;
	gamepad->tmp_z = 0;
	gamepad->tmp_tl = 0;
	gamepad->tmp_tr = 0;
	gamepad->tmp_tl2 = 0;
	gamepad->tmp_tr2 = 0;
	gamepad->tmp_select = 0;
	gamepad->tmp_start = 0;
	gamepad->tmp_mode = 0;
	gamepad->tmp_thumbl = 0;
	gamepad->tmp_thumbr = 0;
	gamepad->tmp_hat0x = 0;
	gamepad->tmp_hat0y = 0;
	gamepad->tmp_abs_x = 0;
	gamepad->tmp_abs_y = 0;
	gamepad->tmp_abs_rx = 0;
	gamepad->tmp_abs_ry = 0;
	gamepad->tmp_abs_hat0x = 0;
	gamepad->tmp_abs_hat0y = 0;
  return gamepad;
}

void pollGamepad(Gamepad* gamepad){
  if(gamepad == NULL){
    return;
  }
  // get lock
  pthread_rwlock_rdlock(&gamepad->lock);
  gamepad->south = gamepad->tmp_south;
  gamepad->east = gamepad->tmp_east;
  gamepad->c = gamepad->tmp_c;
  gamepad->north = gamepad->tmp_north;
  gamepad->west = gamepad->tmp_west;
  gamepad->z = gamepad->tmp_z;
  gamepad->tl = gamepad->tmp_tl;
  gamepad->tr = gamepad->tmp_tr;
  gamepad->tl2 = gamepad->tmp_tl2;
  gamepad->tr2 = gamepad->tmp_tr2;
  gamepad->select = gamepad->tmp_select;
  gamepad->start = gamepad->tmp_start;
  gamepad->mode = gamepad->tmp_mode;
  gamepad->thumbl = gamepad->tmp_thumbl;
  gamepad->thumbr = gamepad->tmp_thumbr;
  gamepad->hat0x = gamepad->tmp_hat0x;
  gamepad->hat0y = gamepad->tmp_hat0y;
  gamepad->abs_x = gamepad->tmp_abs_x;
  gamepad->abs_y = gamepad->tmp_abs_y;
  gamepad->abs_rx = gamepad->tmp_abs_rx;
  gamepad->abs_ry = gamepad->tmp_abs_ry;
  gamepad->abs_hat0x = gamepad->tmp_abs_hat0x;
  gamepad->abs_hat0y = gamepad->tmp_abs_hat0y;
  pthread_rwlock_unlock(&gamepad->lock);
}

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
        if(e.value < 20 && e.value > -20){
          e.value = 0;
        }
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
  spacemouse->tmp_x = 0;
  spacemouse->tmp_y = 0;
  spacemouse->tmp_z = 0;
  spacemouse->tmp_rx = 0;
  spacemouse->tmp_ry = 0;
  spacemouse->tmp_rz = 0;
  spacemouse->tmp_btn_0 = 0;
  spacemouse->tmp_btn_1 = 0;
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

  spacemouse->btn_0 = spacemouse->tmp_btn_0;
  spacemouse->btn_1 = spacemouse->tmp_btn_1;
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
