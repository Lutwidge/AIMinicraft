#include "CreatureManager.h"

#include "AICreature.h"
#include "../engine_minicraft.h"
#include "../engine/render/vbo.h"
#include "../SimpleList.h"
#include "Perceptor.h"


CreatureManager::CreatureManager(MWorld* world) {
	perceptor = new Perceptor(this, world);
	deadCreatures = new SimpleList<AICreature*>(4, 4);
}

CreatureManager::~CreatureManager() {
	// TODO cleanup creatures map
	delete perceptor;
}

void CreatureManager::registerCreature(AICreature* creature) {
	SimpleList<AICreature*>* typeVec;
	CreatureType* type = creature->getType();
	auto search = creatures.find(type);
	if (search != creatures.end()) {
		typeVec = search->second;
	} else {
		typeVec = new SimpleList<AICreature*>(16, 16);
		creatures[type] = typeVec;
	}
	typeVec->add(creature);	
}

void CreatureManager::unregisterCreature(AICreature* creature) {
	SimpleList<AICreature*>* typeVec;
	auto search = creatures.find(creature->getType());
	if (search != creatures.end()) {
		typeVec = search->second;
		for (int i = 0; i < typeVec->count; i++) {
			if (typeVec->arr[i] == creature) {
				typeVec->remove(i);
				return;
			}
		}
	}
}

void CreatureManager::registerDeadCreature(AICreature* creature) {
	unregisterCreature(creature);
	perceptor->removeFromHistory(creature);
	deadCreatures->add(creature);
}

void CreatureManager::unregisterDeadCreature(AICreature* creature) {
	for (int i = 0; i < deadCreatures->count; i++) {
		if (deadCreatures->arr[i] == creature) {
			deadCreatures->remove(i);
			return;
		}
	}
}

void CreatureManager::update(float dt) {
	for (std::pair<CreatureType*, SimpleList<AICreature*>*> typePair : creatures) {
		SimpleList<AICreature*>* typeCreatures = typePair.second;
		for (unsigned int i = 0; i < typeCreatures->count; i++) {
			typeCreatures->arr[i]->update(dt);
		}
	}

	for (int i = 0; i < deadCreatures->count; i++)
	{
		deadCreatures->arr[i]->update(dt);
	}

	perceptor->nextFrame();
}

void CreatureManager::render(MEngineMinicraft* engine) {
	// Render alive creatures
	for (std::pair<CreatureType*, SimpleList<AICreature*>*> typePair : creatures) {
		SimpleList<AICreature*>* typeCreatures = typePair.second;
		for (unsigned int i = 0; i < typeCreatures->count; i++) {
			typeCreatures->arr[i]->render(engine);
		}
	}

	// Render dead creatures
	for (int i = 0; i < deadCreatures->count; i++)
	{
		deadCreatures->arr[i]->render(engine);
	}
}

SimpleList<AICreature*>* CreatureManager::getCreaturesOfType(CreatureType* type) {
	auto search = creatures.find(type);
	if (search != creatures.end()) {
		return search->second;
	}
	return nullptr;
}

SimpleList<AICreature*>* CreatureManager::getDeadCreatures() {
	return deadCreatures;
}
