#include "Perceptor.h"

#include "../SimpleList.h"
#include "AICreature.h"
#include "../my_physics.h"
#include "CreatureManager.h"


Perceptor::Perceptor(CreatureManager* manager, MWorld* world) : manager(manager), world(world), types(new SimpleList<CreatureType>(4, 4)) {
	for (int i = 0; i < CREATURE_TYPE_COUNT; i++) {
		types->add((CreatureType) i);
	}
}

Perceptor::~Perceptor() {}


void Perceptor::nextFrame() {
	currentTypeIndex++;
	currentTypeIndex = currentTypeIndex % types->count;
}

AICreature* Perceptor::creatureSight(AICreature* caller, CreatureType desiredType, float range) {
	unordered_map<CreatureType, AICreature*>* creaturePrevious;
	if (caller->getType() != types->arr[currentTypeIndex]) {
		AICreature* prev = nullptr;
		auto typeSearch = previousTargets.find(caller);
		if (typeSearch != previousTargets.end()) {
			auto creatureSearch = typeSearch->second->find(desiredType);
			if (creatureSearch != typeSearch->second->end()) {
				return creatureSearch->second;
			}
		} else {
			creaturePrevious = new unordered_map<CreatureType, AICreature*>();
			previousTargets[caller] = creaturePrevious;
		}
		return nullptr;
	}

	AICreature* nearest = nullptr;
	float nearestDistance = INFINITY;
	SimpleList<AICreature*>* possibleTargets = manager->getCreaturesOfType(desiredType);
	if (possibleTargets == nullptr) {
		return nullptr;
	}
	for (unsigned int i = 0; i < possibleTargets->count; i++) {
		AICreature* target = possibleTargets->arr[i];
		if (target == caller) { // Discard self
			continue;
		}
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

	auto typeSearch2 = previousTargets.find(caller);
	if (typeSearch2 != previousTargets.end()) {
		creaturePrevious = typeSearch2->second;
		(*creaturePrevious)[desiredType] = nearest;
	}
	return nearest;
}

bool Perceptor::raycast(YVec3f position, YVec3f direction, float range, YVec3f& pos)
{
	MCube* nearest = nullptr;
	YVec3f nearestPosition(0, 0, 0);
	float nearestDistance = INFINITY;
	YVec3f cursorPosition = position;
	direction = direction.normalize();

	for (float dist = 0; dist < range; dist++, cursorPosition += direction)
	{
		MCube* cb = world->getCube(floorf(cursorPosition.X), floorf(cursorPosition.Y), floorf(cursorPosition.Z));

		//Pas de collision mais on verifie quand meme via picking
		if (cb->getType() == MCube::CUBE_AIR)
		{
			float distance;
			int tx, ty, tz;
			MMy_Physics::GetNearestPickableCube(position, cursorPosition, world, distance, tx, ty, tz);
			MCube* hit = world->getCube(tx, ty, tz);
			if (hit->getType() != MCube::CUBE_AIR && (cursorPosition - YVec3f(tx, ty, tz)).getSize() <= 1)
			{
				pos = YVec3f(tx, ty, tz);
				return true;
			}
		}
		else
		{
			pos = cursorPosition;
			return true;
		}
	}
	return false;
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

