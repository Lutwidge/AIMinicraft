#pragma once

#include <type_traits>
#include <typeinfo>
#include "../AICreature.h"

#define DIR_COUNT = 4;
#define WOLF_SPEED = 0.1f;
#define WOLF_SATIATION_DELAY = 0.01f;
#define WOLF_REPRODUCTION_THRESHOLD = 0.8f;
#define WOLF_SIGHT_RANGE = 15;
#define WOLF_EAT_GAIN = 0.3f;

class Wolf : public AICreature 
{
	#pragma region States
	struct WolfState : public State {
		Wolf* wolf;
		WolfState(AICreature* creature): State(creature), wolf((Wolf*) creature) {}
	};

	struct IdleState : public WolfState
	{
		IdleState(Wolf* wolf): WolfState(wolf) {}

		virtual void enter() {
			//initializePath
		}

		virtual void update(float elapsed) 
		{
			if (wolf->updateSatiation(elapsed)) 
			{	
				//TO-DO: A CHANGER POUR POINTER VERS TRAP
				if (wolf->manager->perceptor->creatureSight(wolf, CreatureType::Bear, 15) != nullptr) 
				{
					wolf->switchState(new FleeState(wolf));
					return;
				}

				else
				{
					if (wolf->canReproduce())
					{
						AICreature* targetWolf = wolf->manager->perceptor->creatureSight(wolf, CreatureType::Wolf, 15);
						if (targetWolf != nullptr)
						{
							if (wolf->setPartner(targetWolf))
							{
								wolf->switchState(new ReproduceState(wolf));
								return;
							}
						}
						//return;
					}
				}

				if (wolf->manager->perceptor->creatureSight(wolf, CreatureType::Bear, 15) != nullptr)
				{
					wolf->setEatTarget(wolf->manager->perceptor->creatureSight(wolf, CreatureType::Bear, 15));

					if (wolf->isEatTargetValid()) 
					{
						YLog::log(YLog::USER_INFO, toString("[WOLF] Found prey !").c_str());
						wolf->switchState(new EatState(wolf));
						return;
					}

					if (wolf->hasNotReachedTarget()) wolf->move(elapsed);
					//else bouger
				}
			}
		}

		virtual void exit() {};
	};

	struct ReproduceState : public WolfState 
	{
		ReproduceState(Wolf* wolf) : WolfState(wolf) {}

		virtual void enter() 
		{
			YVec3f meetingPoint = (wolf->position + ((Wolf*)wolf->partner)->position) / 2;
			meetingPoint = YVec3f((int)meetingPoint.X, (int)meetingPoint.Y, (int)meetingPoint.Z);
			wolf->goTo(YVec3f(meetingPoint.X, meetingPoint.Y, wolf->world->getSurface(meetingPoint.X, meetingPoint.Y)));
		}

		virtual void update(float elapsed) 
		{
			if (wolf->updateSatiation(elapsed))
			{
				if (wolf->manager->perceptor->creatureSight(wolf, CreatureType::Wolf, WOLF_SIGHT_RANGE) != nullptr)
				{
					wolf->resetPartner();
					wolf->switchState(new FleeState(wolf));
					return;
				}

				else
				{
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
						}

						else
						{
							wolf->resetPartner();
							wolf->switchState(new IdleState(wolf));
							return;
						}
					}
				}
			}

		}

		virtual void exit() {}
	};

	struct FleeState : public WolfState 
	{
		FleeState(Wolf* wolf) : WolfState(wolf) {}

		virtual void enter(){}

		virtual void update(float elapsed) {

		}

		virtual void exit() {}
	};

	struct EatState : public WolfState 
	{
		EatState(Wolf* wolf) : WolfState(wolf) {}

		virtual void enter() 
		{
			wolf->eatTarget = wolf->preyCreature->position;
			YLog::log(YLog::USER_INFO, toString("[WOLF] Go to prey !").c_str());
			wolf->gotToEatTarget();
		}

		virtual void update(float elapsed) 
		{
			if (wolf->updateSatiation(elapsed))
			{
				//TODO: faire une fonction détection
				if (wolf->manager->perceptor->creatureSight(wolf, CreatureType::Bear, 15) != nullptr)
				{
					wolf->switchState(new FleeState(wolf));
					return;
				}
				else
				{
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
				}
			}
		}

		virtual void exit() {}
	};

	//EATING FUNCTIONS
	AICreature* preyCreature;

	virtual void setEatTarget(AICreature* creature) 
	{
		preyCreature = creature;
	}

	virtual bool isEatTargetValid() 
	{
		return preyCreature != nullptr;
	}

	#pragma endregion
};
