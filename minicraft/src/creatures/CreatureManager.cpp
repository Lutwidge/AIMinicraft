#include "CreatureManager.h"

#include <type_traits>
#include <typeinfo>
#include "AICreature.h"
#include "../engine_minicraft.h"
#include "../engine/render/vbo.h"
#include "../SimpleList.h"
#include "Perceptor.h"


CreatureManager::CreatureManager(MWorld* world) {
	perceptor = new Perceptor(world);
}

CreatureManager::~CreatureManager() {
	// TODO cleanup creatures map
	delete perceptor;
}

void CreatureManager::registerCreature(AICreature* creature) {
	SimpleList<AICreature*>* typeVec;
	std::type_index type = std::type_index(typeid(creature));
	auto search = creatures.find(type);
	if (search != creatures.end()) {
		typeVec = search->second;
	} else {
		typeVec = new SimpleList<AICreature*>(16, 16);
		creatures[type] = typeVec;
		perceptor->registerType(type);
	}
	typeVec->add(creature);
	
}

void CreatureManager::unregisterCreature(AICreature* creature) {
	SimpleList<AICreature*>* typeVec;
	auto search = creatures.find(std::type_index(typeid(creature)));
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

void CreatureManager::update(float dt) {
	for (std::pair<std::type_index, SimpleList<AICreature*>*> typePair : creatures) {
		SimpleList<AICreature*>* typeCreatures = typePair.second;
		for (unsigned int i = 0; i < typeCreatures->count; i++) {
			typeCreatures->arr[i]->update(dt);
		}
	}
}

void CreatureManager::render(MEngineMinicraft* engine, GLuint shader, YVbo* vbo) {
	for (std::pair<std::type_index, SimpleList<AICreature*>*> typePair : creatures) {
		SimpleList<AICreature*>* typeCreatures = typePair.second;
		for (unsigned int i = 0; i < typeCreatures->count; i++) {
			AICreature* creature = typeCreatures->arr[i];
			glPushMatrix();
			glUseProgram(shader);
			glTranslatef(creature->position.X + MCube::CUBE_SIZE / 2.0f, creature->position.Y + MCube::CUBE_SIZE / 2.0f, creature->position.Z + MCube::CUBE_SIZE / 2.0f);
			engine->Renderer->updateMatricesFromOgl();
			engine->Renderer->sendMatricesToShader(shader);
			vbo->render();
			glPopMatrix();
		}
	}
}

SimpleList<AICreature*>* CreatureManager::getCreaturesOfType(std::type_index type) {
	auto search = creatures.find(type);
	if (search != creatures.end()) {
		return search->second;
	}
	return nullptr;
}
