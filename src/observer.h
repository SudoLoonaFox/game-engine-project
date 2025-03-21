#ifndef OBSERVER_H
#define OBSERVER_H
#include <stdio.h>
#include <stdlib.h>

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

typedef struct _observer{
	int id;
	// address of concrete observer. Can be an entity, observer, npc, environment object etc
	void* impl;
	// Number of subjects attached to
	int instances;
	// update takes generic input and triggers specific update.
	// observer event subject
	int (*update)(struct _observer*, int, void*);
	int (*destroy)(struct _observer**);
}Observer;

int observerDestroy(Observer**);

// impl, func ptr to update
Observer* newObserver(void*, int (*)(Observer*, int, void*));
#endif
