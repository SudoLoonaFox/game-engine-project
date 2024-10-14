#include <criterion/criterion.h>
#include "../src/observer.h"

int testInt = 0;
Observer* obs;

static int update(Observer* this, int event, void* subject){
	*(int*)this->impl = event;
	return 0;
}

void suiteSetup(void){
	obs = newObserver((void*)&testInt, update);
}

void suiteTeardown(void){


}

TestSuite(observertests, .init=suiteSetup, .fini=suiteTeardown);

Test(observertests, create){
	cr_expect(obs!=NULL, "Observer failed to be created");
}

Test(observertests, update){
	obs->update(obs, 5, obs->impl);
	cr_expect(testInt == 5, "Observer failed to update");
}

Test(observertests, internaldestroy){
	obs->destroy(&obs);
	cr_expect(obs==NULL, "Observer failed to destroy self");
}

Test(observertests, externaldestroy){
	observerDestroy(&obs);
	cr_expect(obs==NULL, "External destroy failed to destroy observer");
}
