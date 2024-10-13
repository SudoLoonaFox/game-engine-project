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

static int _destroyEntity(Entity* ){
	//code to free entity memory and release observers
	// if entity is part of an array then just move last item to entity position and decrement array length
	return 0;
}

// TODO add check if already attached
static int _registerObserver(Entity* this, Observer* that){
	if(this->observerNum >= MAX_OBSERVERS){
		// Add in some error code here. Observer couldn't be added
		return -1;
	}
	this->observers[(this->observerNum)++] = that;
	that->instances++;
	//that->update(that, 2, that->impl);
	return 0;
}
static int _unregisterObserver(Entity* this, Observer* that){
	for(int i = 0; i < this->observerNum; i++){
		// TODO Need to check there are no other registered locations
		if(this->observers[i] == that){
			that->instances--;
			if(this->observerNum>1){
				this->observers[i] = this->observers[this->observerNum];
			}
			this->observerNum--;
		}
	}
	return 0;
}

static int _notifyObservers(Entity* this){
	if(this->observerNum <1){
		return -1;
	}
	for(int i = 0; i < this->observerNum; i++){
		printf("Number of observers:%i\n", this->observerNum);
		this->observers[i]->update(this->observers[i], this->event, this);
	}
	return 0;
}

Entity* newEntity(){
	Entity* this = malloc(sizeof(Entity));
	static int id = 0;
	this->id = id++;
	this->registerObserver = _registerObserver;
	this->unregisterObserver = _unregisterObserver;
	this->notifyObservers = _notifyObservers;
	this->destroy = _destroyEntity;
	return this;
}
int updateTest(Observer* this, int event, void* subject){
	printf("Observer triggered by Entity: %i\n", ((Entity*)subject)->id);
	*(int*)this->impl = event;
	return 0;
}


int observerDestroy(Observer* this){
	return 0;
}
Observer* newObserver(void* impl, int (*update)(Observer*, int, void*)){
	Observer* this = malloc(sizeof(Observer));
	static int id = 0;
	this->id = id++;
	this->impl = impl;
	this->update = updateTest;
	this->destroy = observerDestroy;
	return this;
}
