#ifndef ENTITY_H
#define ENTITY_H

#include <stdio.h>
#include <stdlib.h>
#include "observer.h"

#define MAX_OBSERVERS 100

/*
Observers can be entities, achievements, quests or environment
An example use is rain starts and npcs seek cover
Another example is you kill 10 enemies with x weapon and get an achievement
Observers need to run code on recieving an update from an entity
Observers will use a generic type with pointers to update specific objects
The path an event may go is entity/environment/npc/enemy->observer->entity/quest/
Observers also need to be able to read data of the objects they are observing
A quest may get an event for an enemy dying. The quest needs to check the enemy was killed by player and player is using a specific item
*/

/*
Entities need to either store component data or have component data referenced by an id
Entites need to be able to notify observers of an event taken
*/

/*
Entites may be initalized in an array but most access will be by refernce
Should I include the register unregister functions in the struct?
Reg, unreg, notify, destroy can be moved out of this; however,
I am keeping them in for now so functions they are passed to can call their functions
*/
typedef struct _entity{
	int id;
	int event;
	int (*destroy)(struct _entity*);
	// Observer data
	Observer* observers[MAX_OBSERVERS];
	int observerNum;
	// Register observer
	int (*registerObserver)(struct _entity*, Observer*);
	// Unregister observer
	int (*unregisterObserver)(struct _entity*, Observer*);
	// Notify observers runs the update function for each
	int (*notifyObservers)(struct _entity*);
}Entity;

void notify(Entity* , int);

Entity* newEntity();

int updateTest(Observer*, int, void*);

#endif
