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
	// address of concrete observer. Can be an entity, observer, npc, environment object etc
	void* impl;
	// update takes generic input and triggers specific update.
	int (*update)(struct _observer*, Event, void* subject);
	int (*destroy)(struct _observer*);
}Observer;

/*
Entites may be initalized in an array but most access will be by refernce
Should I include the register unregister functions in the struct?
Reg, unreg, notify, destroy can be moved out of this; however,
I am keeping them in for now so functions they are passed to can call their functions
*/
typedef struct _entity{
	int id;
	int (*destroy)(struct _entity*);
	// Observer data
	Observer observers[MAX_OBSERVERS];
	int observerNum;
	// Register observer
	int (*registerObserver)(struct _entity*, Observer*);
	// Unregister observer
	int (*unregisterObserver)(struct _entity*, Observer*);
	// Notify observers runs the update function for each
	void (*notifyObservers)(struct _entity*);
}Entity;

static int _destroyEntity(Entity* ){
	//code to free entity memory and release observers
	// if entity is part of an array then just move last item to entity position and decrement array length
	return 0;
}

static int _registerObserver(Entity* this, Observer* that){
	if(this->observerNum >= MAX_OBSERVERS){
		// Add in some error code here. Observer couldn't be added
		return -1;
	}
	this->observers[++this->observerNum] = that;
	return 0;
}

Entity* newEntity(){
	Entity* this = malloc(sizeof(Entity));
	static int id = 0;
	this->id = id;
	id++;
	this->destroy = _destroyEntity;
	return this;
}
Observer* newObserver(void* impl, int (*update)(Observer*, enum, void* subject)){

}
