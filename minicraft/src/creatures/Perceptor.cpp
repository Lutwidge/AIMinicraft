#include "Perceptor.h"

#include <type_traits>
#include <typeinfo>
#include "../SimpleList.h"
#include "AICreature.h"
#include "../my_physics.h"
#include "CreatureManager.h"


Perceptor::Perceptor(CreatureManager* manager, MWorld* world) : manager(manager), world(world), types(new SimpleList<std::type_index*>(4, 4)) {}

Perceptor::~Perceptor() {}


void Perceptor::registerType(std::type_index type) {
	std::type_index* ptType = new std::type_index(type);
	types->add(ptType);
}

void Perceptor::nextFrame() {
	currentTypeIndex++;
	currentTypeIndex = currentTypeIndex % types->count;
}

AICreature* Perceptor::creatureSight(AICreature* caller, std::type_index desiredType, float range) {
	if (std::type_index(typeid(caller)) != *types->arr[currentTypeIndex]) {
		AICreature* prev = nullptr;
		auto search = previousTargets.find(caller);
		if (search != previousTargets.end()) {
			return search->second;
		}
		return nullptr;
	}

	AICreature* nearest = nullptr;
	float nearestDistance = INFINITY;
	SimpleList<AICreature*>* possibleTargets = manager->getCreaturesOfType(desiredType);
	for (unsigned int i = 0; i < possibleTargets->count; i++) {
		AICreature* target = possibleTargets->arr[i];
		YVec3f toTarget = caller->position - target->position;
		if (toTarget.getSize() > range || toTarget.normalize().dot(caller->forward) < 0) { // Discard targets too far or behind
			continue;
		}
		float distance = INFINITY;
		int tx, ty, tz;
		if (MMy_Physics::GetNearestPickableCube(caller->position, target->position, world, distance, tx, ty, tz)) { // Discard targets not in view
			continue;
		}
		if (nearest == nullptr || distance < nearestDistance) {
			nearest = target;
			nearestDistance = distance;
		}
	}

	return nearest;
}

bool Perceptor::blockSight(AICreature* caller, MCube::MCubeType type, float range, YVec3f& pos) {
	int minX = floorf(caller->position.X - range);
	int minY = floorf(caller->position.Y - range);
	int minZ = floorf(caller->position.Z - range);
	int maxX = floorf(caller->position.X + range);
	int maxY = floorf(caller->position.Y + range);
	int maxZ = floorf(caller->position.Z + range);

	MCube* nearest = nullptr;
	YVec3f nearestPosition(0, 0, 0);
	float nearestDistance = INFINITY;
	for (int x = minX; x < maxX; x++) {
		for (int y = minY; y < maxY; y++) {
			for (int z = minZ; z < maxZ; z++) {
				YVec3f blockPos(x, y, z);
				YVec3f toBlock = (blockPos - caller->position);
				if (toBlock.getSize() > range || toBlock.normalize().dot(caller->forward) < 0) { // Discard blocks too far or behind the creature
					continue;
				}
				MCube* cube = world->getCube(x, y, z);
				if (cube->getType() != type) { // Discard blocks of the wrong type
					continue;
				}
				float distance;
				int tx, ty, tz;
				MMy_Physics::GetNearestPickableCube(caller->position, blockPos, world, distance, tx, ty, tz);
				MCube* hit = world->getCube(tx, ty, tz);
				if (hit == cube && (nearest == nullptr || nearestDistance > distance)) {
					nearest = cube;
					nearestDistance = distance;
					nearestPosition = YVec3f(x, y, z);
				}
			}
		}
	}

	if (nearest != nullptr) {
		pos = nearestPosition;
		return true;
	}
	return false;
}
