#include <stdio.h>
#include "entity.h"

#define MAX_ENTITY_NUM 10

int main(){
	Entity *entity[MAX_ENTITY_NUM];
	int entityLen = 0;
	for(int i = 0; i < MAX_ENTITY_NUM; i++){
		entity[i] = newEntity();
		entityLen++;
	}
	for(int i = 0; i < MAX_ENTITY_NUM; i++){
		printf("Entity: %i\n", entity[i]->id);
	}
	int testInt = 0;
	Observer* obs = newObserver((void*)&testInt, updateTest);
	entity[0]->registerObserver(entity[0], obs);
	printf("testInt:%i\n",testInt);

	entity[0]->event = 1;
	entity[0]->notifyObservers(entity[0]);

	notify(entity[0], 5);

	printf("testInt:%i\n",testInt);
	printf("Unregistering observer\n");
	entity[0]->unregisterObserver(entity[0], obs);
	obs->destroy(&obs);
}

