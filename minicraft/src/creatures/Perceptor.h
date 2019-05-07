#pragma once

#include <typeindex>
#include <unordered_map>
#include "../engine/utils/types_3d.h"
#include "../cube.h"

template<typename T>
class SimpleList;
class AICreature;
class MWorld;
class CreatureManager;

class Perceptor {
public:
	Perceptor(CreatureManager* manager, MWorld* world);
	~Perceptor();

	void registerType(std::type_index type);
	AICreature* creatureSight(AICreature* caller, std::type_index desiredType, float range);
	bool blockSight(AICreature* caller, MCube::MCubeType type, float range, YVec3f& pos);
	void nextFrame();

private:
	SimpleList<std::type_index*>* types = nullptr;
	std::unordered_map<AICreature*, AICreature*> previousTargets;
	unsigned int currentTypeIndex = 0;
	MWorld* world = nullptr;
	CreatureManager* manager = nullptr;
};

