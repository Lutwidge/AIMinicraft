#pragma once

#include <type_traits>
#include <typeinfo>
#include "../AICreature.h"

#define DIR_COUNT  4
#define WOLF_SPEED  0.2f
#define WOLF_SATIATION_DELAY  0.01f
#define WOLF_REPRODUCTION_THRESHOLD  0.8f
#define WOLF_REPRODUCTION_COUNT  0.4f
#define WOLF_SIGHT_RANGE  15
#define WOLF_EAT_GAIN  0.3f
#define WOLF_MOVEMENT_RANGE  8
#define WOLF_FLEE_RANGE  6

class Wolf : public AICreature 
{
protected:
	#pragma region States
	struct WolfState : public State {
		Wolf* wolf;
		WolfState(AICreature* creature): State(creature), wolf((Wolf*) creature) {}
	};

	struct IdleState : public WolfState
	{
		IdleState(Wolf* wolf): WolfState(wolf) {}

		virtual void enter() {
			//YLog::log(YLog::USER_INFO, toString("[WOLF] Idle State Enter").c_str());
			wolf->initializePath();
		}

		virtual void update(float elapsed) 
		{
			if (wolf->updateSatiation(elapsed)) 
			{	
				//wolf->predator = wolf->manager->perceptor->creatureSight(wolf, CreatureType::Trap, WOLF_SIGHT_RANGE);
				//if (wolf->predator != nullptr)
				//{
				//	wolf->switchState(new FleeState(wolf));
				//	return;
				//}
				//else
				//{
					if (wolf->canReproduce())
					{
						AICreature* targetWolf = wolf->manager->perceptor->creatureSight(wolf, CreatureType::Wolf, WOLF_SIGHT_RANGE);
						if (targetWolf != nullptr)
						{
							if (wolf->setPartner(targetWolf))
							{
								wolf->switchState(new ReproductionState(wolf));
								return;
							}
						}
						//return;
					}
				//}

				AICreature* target = wolf->manager->perceptor->creatureSight(wolf, CreatureType::Goat, WOLF_SIGHT_RANGE);
				if (target != nullptr)
				{
					wolf->setEatTarget(target);

					if (wolf->isEatTargetValid()) 
					{
						//YLog::log(YLog::USER_INFO, toString("[WOLF] Found prey !").c_str());
						wolf->switchState(new EatState(wolf));
						return;
					}
				}

				if (wolf->hasNotReachedTarget()) wolf->move(elapsed);
				else wolf->initializePath();
			}
		}

		virtual void exit() {};
	};

	struct ReproductionState : public WolfState 
	{
		ReproductionState(Wolf* wolf) : WolfState(wolf) {}

		virtual void enter() 
		{
			//YLog::log(YLog::USER_INFO, toString("[WOLF] Reproduce State Enter").c_str());
			YVec3f meetingPoint = (wolf->position + ((Wolf*)wolf->partner)->position) / 2;
			meetingPoint = YVec3f((int)meetingPoint.X, (int)meetingPoint.Y, (int)meetingPoint.Z);
			wolf->goTo(YVec3f(meetingPoint.X, meetingPoint.Y, wolf->world->getSurface(meetingPoint.X, meetingPoint.Y)));
		}

		virtual void update(float elapsed) 
		{
			if (wolf->updateSatiation(elapsed))
			{
				//wolf->predator = wolf->manager->perceptor->creatureSight(wolf, CreatureType::Trap, WOLF_SIGHT_RANGE);
				//if (wolf->predator != nullptr)
				//{
				//	wolf->resetPartner();
				//	wolf->switchState(new FleeState(wolf));
				//	return;
				//}

				//else
				//{
					if (wolf->isPartnerValid())
					{
						if (wolf->hasNotReachedTarget())
						{
							wolf->move(elapsed);
							return;
						}

						else if (!wolf->hasNotReachedTarget() && !wolf->partner->hasNotReachedTarget())
						{
							wolf->reproduce();
							return;
						}

						else
						{
							wolf->resetPartner();
							wolf->switchState(new IdleState(wolf));
							return;
						}
					}
				//}
			}

		}

		virtual void exit() {}
	};

	struct FleeState : public WolfState 
	{
		FleeState(Wolf* wolf) : WolfState(wolf) {}

		virtual void enter()
		{
			//YLog::log(YLog::USER_INFO, toString("[WOLF] Flee State Enter").c_str());
			YVec3f fleeTarget = wolf->position + (wolf->position - wolf->predator->position).normalize() * WOLF_FLEE_RANGE;

			fleeTarget = wolf->world->getNearestAirCube(fleeTarget.X, fleeTarget.Y, fleeTarget.Z);
			wolf->goTo(fleeTarget);
		}

		virtual void update(float elapsed) 
		{
			if (wolf->updateSatiation(elapsed))
			{
				if (wolf->hasNotReachedTarget()) wolf->move(elapsed);
				else wolf->switchState(new IdleState(wolf));
			}
		}

		virtual void exit() {}
	};

	struct EatState : public WolfState 
	{
		EatState(Wolf* wolf) : WolfState(wolf) {}

		virtual void enter() 
		{
			wolf->eatTarget = wolf->preyCreature->position;
			//YLog::log(YLog::USER_INFO, toString("[WOLF] Go to prey !").c_str());
			wolf->gotToEatTarget();
		}

		virtual void update(float elapsed) 
		{
			if (wolf->updateSatiation(elapsed))
			{
				//wolf->predator = wolf->manager->perceptor->creatureSight(wolf, CreatureType::Trap, WOLF_SIGHT_RANGE);
				//if (wolf->predator != nullptr)
				//{
				//	wolf->switchState(new FleeState(wolf));
				//	return;
				//}
				//else
				//{
					if (wolf->isEatTargetValid())
					{
						if (wolf->hasNotReachedTarget()) wolf->move(elapsed);
						else
						{
							wolf->eat();
							return;
						}
					}

					else
					{
						wolf->switchState(new IdleState(wolf));
						return;
					}
				//}
			}
		}

		virtual void exit() {}
	};
	#pragma endregion

public:
	Wolf(string name, MWorld* world, CreatureManager* cm, YVec3f pos) : AICreature(name, world, cm, pos, true, WOLF_SPEED, WOLF_SATIATION_DELAY, WOLF_REPRODUCTION_THRESHOLD)
	{
		manager->registerCreature(this);
		switchState(new IdleState(this));
	}

	//EATING FUNCTIONS
	AICreature* preyCreature;

	virtual void setEatTarget(AICreature* creature)
	{
		preyCreature = creature;
	}

	virtual bool isEatTargetValid() {
		if (preyCreature != nullptr)
		{
			if (!preyCreature->IsDead)
				return true;
		}
		return false;
	}

	virtual void eat()
	{
		preyCreature->die();

		satiation += WOLF_EAT_GAIN;
		if (satiation > 1.0f) satiation = 1.0f;

		preyCreature = nullptr;
	}

	//IDLE FUNCTIONS
	int pathLength;
	int curDirIndex;
	YVec3f directions[4] = { YVec3f(1, 0, 0), YVec3f(-1, 0, 0), YVec3f(0, -1, 0), YVec3f(0, 1, 0) };

	virtual void initializePath()
	{
		int randomIndex = rand() % 3;

		YVec3f randomDirection = directions[randomIndex];
		int x = randomDirection.X * WOLF_MOVEMENT_RANGE;
		int y = randomDirection.Y * WOLF_MOVEMENT_RANGE;

		x += position.X;
		y += position.Y;

		if (x > world->MAT_SIZE_METERS) x -= randomDirection.X * WOLF_MOVEMENT_RANGE * 2;

		if (x < 0) x += randomDirection.X * WOLF_MOVEMENT_RANGE * 2;

		if (y > world->MAT_SIZE_METERS) y -= randomDirection.Y * WOLF_MOVEMENT_RANGE * 2;

		if (y < 0) y += randomDirection.X * WOLF_MOVEMENT_RANGE * 2;

		YVec3f target = YVec3f(x, y, world->getSurface(x, y));
		goTo(target);
	}

	//REPRODUCTION FUNCTIONS
	virtual bool setPartner(AICreature* newPartner)
	{
		Wolf* wolfPartner = nullptr;
		if (newPartner->getType() == CreatureType::Wolf)
		{
			wolfPartner = (Wolf*)newPartner;
			if (partner == nullptr && wolfPartner->partner == nullptr)
			{
				partner = wolfPartner;
				wolfPartner->partner = this;
				wolfPartner->switchState(new ReproductionState(wolfPartner));
				return true;
			}
		}
		return false;
	}

	virtual bool isPartnerValid()
	{
		ReproductionState* partnerReprodState = dynamic_cast<ReproductionState*>(partner->state);
		if (partnerReprodState != nullptr) return true;
		return false;

	}

	virtual void reproduce()
	{
		new Wolf("Wolf", world, manager, position);
		((Wolf*)partner)->satiation -= WOLF_REPRODUCTION_COUNT;
		satiation -= WOLF_REPRODUCTION_COUNT;
		partner->switchState(new IdleState((Wolf*)partner));
		switchState(new IdleState(this));
		partner->resetPartner();
	}

	virtual CreatureType* getType()
	{
		return CreatureType::Wolf;
	}
};
