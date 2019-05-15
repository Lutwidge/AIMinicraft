#pragma once

#include <unordered_map>
#include "../engine/utils/types_3d.h"
#include "../cube.h"
#include "CreatureType.h"

template<typename T>
class SimpleList;
class AICreature;
class MWorld;
class CreatureManager;

class Perceptor {
public:
	Perceptor(CreatureManager* manager, MWorld* world);
	~Perceptor();

	AICreature* creatureSight(AICreature* caller, CreatureType* desiredType, float range);
	AICreature* deadCreatureSight(AICreature* caller, float range);
	bool blockSight(AICreature* caller, MCube::MCubeType type, float range, YVec3f& pos);
	bool raycast(YVec3f position, YVec3f direction, float range, YVec3f& pos);
	void nextFrame();
	void removeFromHistory(AICreature* creature);

private:
	SimpleList<CreatureType*>* types = nullptr;
	std::unordered_map<AICreature*, unordered_map<CreatureType*, AICreature*>*> previousTargets;
	unsigned int currentTypeIndex = 0;
	MWorld* world = nullptr;
	CreatureManager* manager = nullptr;
};

