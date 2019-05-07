#pragma once

#include "../AICreature.h"
#include <typeinfo>
#include <type_traits>

#define SPEED 4
#define DECAY_SATIETY 0.02
#define REPRO_THRESHOLD 0.8

class Snake;

class Owl : AICreature
{
public:

#pragma region States
	struct OwlState : State
	{
		Owl* owl;
		OwlState(Owl* owl) : State(creature), owl(owl) {}
	};

#pragma endregion

	Owl(string name, MWorld* world, CreatureManager* manager, YVec3f pos, bool canFly, float speed, float decay, float reproThreshold) :
		AICreature(name, world, manager, pos, true, SPEED, DECAY_SATIETY, REPRO_THRESHOLD)
	{
		holdedFood = nullptr;
		target = nullptr;
	}

	~Owl()
	{
		manager->unregisterCreature(this);
		delete state;
	}

	bool isPartnerValid() override
	{
		return (partner != NULL && partner->canReproduce());
	}

	bool setPartner(AICreature * newPartner) override
	{
		//Already have a partner
		if (partner != nullptr)
			return false;

		Owl* owl = dynamic_cast<Owl*>(newPartner);
		if (owl != nullptr)
		{
			partner = owl;
			return partner->isPartnerValid();
		}
		//Not an owl
		return false;
	}

	void eat() override
	{
		if (holdedFood != nullptr)
		{
			//Die but not implemented
		}
	}

	void reproduce() override
	{
		YVec3f spawnPosition = (partner->position + position) / 2;
	}

	bool isEatTargetValid() override
	{
		return true;
	}

protected:

	//Caught but still not dead
	Snake * holdedFood;
	Snake* target;
};
