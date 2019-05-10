#pragma once

#include "../AICreature.h"
#include <typeinfo>
#include <type_traits>

#define SPEED 4
#define DECAY_SATIETY 0.02
#define REPRO_THRESHOLD 0.8
#define OWL_SIGHT_RANGE 15

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

	//Quitte l'idle quand il a reperé un serpent ou quand il meurt de fin
	struct IdleState : public OwlState
	{
		IdleState(Owl* owl) : OwlState(owl) {}

		void enter()
		{

		}

		void update(float elapsed)
		{
			if (owl->updateSatiation(elapsed))
			{
				if (owl->getSatiation() < 0.2f)
				{
					owl->switchState(new LookingForFood(owl));
				}

				if (owl->manager->perceptor->creatureSight(owl, CreatureType::Owl, OWL_SIGHT_RANGE))
				{
					//Recherche de bouffe
				}
			}
		}

		void exit()
		{

		}
	};

	struct LookingForFood : public OwlState
	{
		LookingForFood(Owl* owl) : OwlState(owl) {}
		YVec3f positionTarget;

		void enter()
		{
			//conter anti boucle infinie
			int counter = 0;
			bool found = false;
			float range = 10;
			YVec3f hitPosition;
			while (!found && counter < 20)
			{
				YVec3f directionRandom(rand() % 2 - 1, rand() % 2 - 1, rand() % 2 - 1);
				//Aucune collision , on peut voler par là
				if (!owl->manager->perceptor->raycast(owl->position, directionRandom,range, hitPosition))
				{
					found = true;
					positionTarget = owl->position + (directionRandom * range);
				}
				counter++;
			}
		}

		void update(float elapsed)
		{

		}

		void exit()
		{

		}
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

	float getSatiation()
	{
		return satiation;
	}

	CreatureType getType()
	{
		return CreatureType::Owl;
	}

protected:

	//Caught but still not dead
	Snake * holdedFood;
	Snake* target;
};
