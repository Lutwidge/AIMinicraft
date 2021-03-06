#pragma once

#include <unordered_map>
#include <vector>
#include "../external/gl/glew.h"
#include "../external/gl/freeglut.h"
#include "CreatureType.h"

class AICreature;
class MEngineMinicraft;
class YVbo;
template<typename T>
class SimpleList;
class Perceptor;
class MWorld;

class CreatureManager {
public:
	Perceptor* perceptor;

	CreatureManager(MWorld* world);
	~CreatureManager();

	void registerCreature(AICreature* creature);
	void unregisterCreature(AICreature* creature);
	void registerDeadCreature(AICreature* creature);
	void unregisterDeadCreature(AICreature* creature);
	void update(float dt);
	void render(MEngineMinicraft* engine);
	SimpleList<AICreature*>* getCreaturesOfType(CreatureType* type);
	SimpleList<AICreature*>* getDeadCreatures();

private:
	std::unordered_map<CreatureType*, SimpleList<AICreature*>*> creatures;
	SimpleList<AICreature*>* deadCreatures;
};
