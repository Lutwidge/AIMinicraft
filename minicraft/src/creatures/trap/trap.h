#pragma once

#include <type_traits>
#include <typeinfo>
#include "../AICreature.h"
#include "../ocelot/ocelot.h"

#define TRAP_DIR_COUNT 4
#define TRAP_SPEED 0.f
#define TRAP_SATIATION_DECAY 0.f
#define TRAP_REPRODUCTION_THRESHOLD 0.f
#define TRAP_SIGHT_RANGE 15
#define TRAP_IDLE_HEIGHT 8
#define TRAP_EAT_GAIN 0.1f
#define TRAP_FLEE_DISTANCE 6

class Trap : public AICreature
{
protected:
#pragma region States
	struct trapState : public State
	{
		Trap* trap;
		trapState(AICreature* creature) : State(creature), trap((Trap*)creature) {}
	};

	struct IdleState : public trapState
	{
		IdleState(Trap* trap) : trapState(trap) {}

		virtual void enter()
		{
			//printf("%s : Idle \n", trap->name.c_str());
		}

		virtual void update(float elapsed) {

			// Mise à jour de la satiété et check de si on est toujours en vie
			if (trap->updateSatiation(elapsed)) 
			{
				// Si on voit un prédateur, on le bouffe
				trap->predator = trap->manager->perceptor->creatureSight(trap, CreatureType::Wolf, TRAP_SIGHT_RANGE);
				if (trap->predator != nullptr) 
				{
					trap->switchState(new EatState(trap, trap->predator));
					return;
				}
			}
		}

		virtual void exit() {}
	};

	struct EatState : public trapState
	{
		AICreature* toKill;

		EatState(Trap* trap) : trapState(trap) {}
		
		EatState(Trap* trap, AICreature* toKill) :trapState(trap)
		{
			this->toKill = toKill;
		}

		virtual void enter()
		{
			//printf("%s : Eat \n", trap->name.c_str());
			trap->eat();
			trap->switchState(new IdleState(trap));
		}

		virtual void update(float elapsed)
		{
	
		}

		virtual void exit() {}
	};

#pragma endregion

	int pathLength;
	YVec3f directions[DIR_COUNT] = { YVec3f(1, 0, 0), YVec3f(0, 1, 0), YVec3f(-1, 0, 0), YVec3f(0, -1, 0) };
	int curDirIndex;
	YVec3f realEatTarget;

public:
	Trap(string name, MWorld * world, CreatureManager * cm, YVec3f pos) : AICreature(name, world, cm, pos, true, TRAP_SPEED, TRAP_SATIATION_DECAY, TRAP_REPRODUCTION_THRESHOLD) 
	{
		manager->registerCreature(this);
		switchState(new IdleState(this));
	}

	/* EATING */
	virtual bool isEatTargetValid() 
	{
		return world->getCube((int)realEatTarget.X, (int)realEatTarget.Y, (int)realEatTarget.Z)->isFruit();
	}

	virtual void eat()
	{
		//world->getCube((int)realEatTarget.X, (int)realEatTarget.Y, (int)realEatTarget.Z)->setType(MCube::CUBE_BRANCHES);
		//world->respawnFruit();
		// Regénérer le monde (mais coûteux... comme le picking)
		//world->updateCube((int) realEatTarget.X, (int) realEatTarget.Y, (int) realEatTarget.Z);
	}

	virtual bool setEatTarget(YVec3f target) 
	{
		realEatTarget = target;
		// Définir la target comme le cube d'air le plus proche du fruit
		eatTarget = world->getNearestAirCube(target.X, target.Y, target.Z);
		if (eatTarget == realEatTarget)
			return false;
		else
			return true;
	}

	bool updateSatiation(float elapsed) override
	{
		return true;
	}

	virtual CreatureType* getType() 
	{
		return CreatureType::Trap;
	}
};