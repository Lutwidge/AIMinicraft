#pragma once

#include <type_traits>
#include <typeinfo>
#include "../AICreature.h"
#include "../ocelot/ocelot.h"

#define TRAP_DIR_COUNT 4
#define TRAP_SPEED 0.f
#define TRAP_SATIATION_DECAY 0.f
#define TRAP_REPRODUCTION_THRESHOLD 0.f
#define TRAP_SIGHT_RANGE 2

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

		virtual void update(float elapsed) 
		{
			trap->predator = trap->manager->perceptor->creatureSight(trap, CreatureType::Wolf, TRAP_SIGHT_RANGE);
			if (trap->predator != nullptr) 
			{
				trap->switchState(new EatState(trap, trap->predator));
				return;
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
			if (toKill != nullptr)
			{
				printf("%s : Trap now ! \n", trap->name.c_str());
				trap->TrapAnimal(toKill);
				trap->switchState(new IdleState(trap));
			}
		}

		virtual void update(float elapsed)
		{
	
		}

		virtual void exit() {}
	};

#pragma endregion


public:
	Trap(string name, MWorld * world, CreatureManager * cm, YVec3f pos) : AICreature(name, world, cm, pos, false, TRAP_SPEED, TRAP_SATIATION_DECAY, TRAP_REPRODUCTION_THRESHOLD) 
	{
		manager->registerCreature(this);
		switchState(new IdleState(this));
	}

	//TRAP ANIMAL 
	virtual void TrapAnimal(AICreature* creatureToKill)
	{
		//creatureToKill->switchState(new DeadState(creatureToKill));
		creatureToKill->die();
	}

	bool updateSatiation(float elapsed) override
	{
		return true;
	}

	virtual CreatureType* getType() 
	{
		return CreatureType::Trap;
	}

	virtual void eat(){}
	virtual void setEatTarget(AICreature* creature){}
	virtual bool isEatTargetValid() { return false; }
	virtual bool setPartner(AICreature* newPartner) { return false; }
	virtual bool isPartnerValid() { return false; }
	virtual void reproduce(){}
};