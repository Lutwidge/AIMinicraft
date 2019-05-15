#pragma once

#include <type_traits>
#include <typeinfo>
#include "../AICreature.h"
#include "../../SimpleList.h"

#define ELEPHANT_SPEED 0.5f
#define ELEPHANT_SIGHT_RANGE 10

#define ELEPHANT_REPRODUCTION_THRESHOLD 0.8f

#define ELEPHANT_EAT_GAIN 0.3f
#define ELEPHANT_SATIATION_DECAY 0.005f
#define ELEPHANT_HUNGER_THRESHOLD 0.4f;

class Elephant : public AICreature
{
protected:
#pragma region States
	struct ElephantState : public State
	{
		Elephant* elephant;
		ElephantState(AICreature* creature) : State(creature), elephant((Elephant*)creature) {}
	};

	struct IdleState : public ElephantState
	{
		IdleState(Elephant* elephant) : ElephantState(elephant) {}

		virtual void enter()
		{
			printf("%s: Idle\n", elephant->name.c_str());
			elephant->wanderingForFruit = false;
			elephant->wanderingForPartner = false;
		}

		virtual void update(float elapsed)
		{
			if (elephant->updateSatiation(elapsed))
			{
				YVec3f fruit;

				// Reproduction
				if (elephant->canReproduce())
				{
					SimpleList<AICreature*>* elephants = elephant->manager->getCreaturesOfType(CreatureType::Elephant);
					for (unsigned int i = 0; i < elephants->count; i++)
					{
						AICreature* potentialPartner = elephants->arr[i];
						if (elephant->setPartner(potentialPartner))
						{
							elephant->switchState(new ReproductionState(elephant));
							return;
						}
					}

					if (!elephant->wanderingForPartner)
					{
						elephant->wanderingForFruit = false;
						elephant->wanderingForPartner = true;
						elephant->wander(elapsed, true);
					}
					else
						elephant->wander(elapsed, false);
				}
				// Eating
				else
				{
					if (elephant->manager->perceptor->blockSight(elephant, MCube::CUBE_FRUIT, ELEPHANT_SIGHT_RANGE, fruit))
					{
						if (elephant->setEatTarget(fruit))
						{
							elephant->switchState(new EatState(elephant));
							return;
						}
					}
					else if (!elephant->wanderingForFruit)
					{
						elephant->wanderingForFruit = true;
						elephant->wanderingForPartner = false;
						elephant->wander(elapsed, true);
					}
					else
						elephant->wander(elapsed, false);
				}
			}
		}

		virtual void exit() {}
	};

	struct EatState : public ElephantState
	{
		EatState(Elephant* elephant) : ElephantState(elephant) {}

		virtual void enter()
		{
			elephant->gotToEatTarget();
			//printf(("Fruit at (" + toString(elephant->targetPos.X) + ", " + toString(elephant->targetPos.Y) + ", " + toString(elephant->targetPos.Z) + ")\n").c_str());
		}

		virtual void update(float elapsed)
		{
			if (elephant->updateSatiation(elapsed))
			{
				if (elephant->isEatTargetValid())
				{
					if (elephant->hasNotReachedTarget())
						elephant->move(elapsed);
					else
					{
						printf("%s: Eat\n", elephant->name.c_str());
						//printf((elephant->name + " at (" + toString(elephant->position.X) + ", " + toString(elephant->position.Y) + ", " + toString(elephant->position.Z) + ")\n").c_str());
						elephant->eat();
						//printf((elephant->name + " satiation: " + toString(elephant->satiation) + "\n").c_str());
						return;
					}
				}
				else
				{
					elephant->switchState(new IdleState(elephant));
					return;
				}
			}
		}

		virtual void exit() {}
	};

	struct ReproductionState : public ElephantState
	{
		ReproductionState(Elephant* elephant) : ElephantState(elephant) {}

		virtual void enter()
		{
			YVec3f meetingPoint;
			MCube::MCubeType cubeTypeUnder;
			int xOffset = 0;

			do {
				meetingPoint = (elephant->position + ((Elephant*)elephant->partner)->position) / 2;
				meetingPoint = YVec3f((int)meetingPoint.X, (int)meetingPoint.Y, (int)meetingPoint.Z);
				meetingPoint.X += xOffset;
				xOffset++;
				meetingPoint.Z = elephant->world->getHighestPoint(meetingPoint.X, meetingPoint.Y);
				cubeTypeUnder = creature->world->getCube(meetingPoint.X, meetingPoint.Y, meetingPoint.Z - 1)->getType();
			} while (!AStar::isTargetValid(elephant->position, meetingPoint, elephant->world, elephant->canFly));
			
			elephant->goTo(meetingPoint);
		}

		virtual void update(float elapsed)
		{
			if (elephant->updateSatiation(elapsed))
			{
				if (elephant->isPartnerValid())
				{
					if (elephant->hasNotReachedTarget())
					{
						//printf((elephant->name + " at (" + toString(elephant->position.X) + ", " + toString(elephant->position.Y) + ", " + toString(elephant->position.Z) +
						//						") going to (" + toString(elephant->targetPos.X) + ", " + toString(elephant->targetPos.Y) + ", " + toString(elephant->targetPos.Z) + ")\n").c_str());
						elephant->move(elapsed);
						return;
					}
					else if (!elephant->partner->hasNotReachedTarget())
					{
						printf("%s: Reproduction\n", elephant->name.c_str());
						elephant->reproduce();
						return;
					}
				}
				else
				{
					elephant->resetPartner();
					elephant->switchState(new IdleState(elephant));
					return;
				}
			}
		}

		virtual void exit() {}
	};
#pragma endregion

	int pathLength;
	YVec3f realEatTarget;
	static int nbElephants;
	bool wanderingForFruit, wanderingForPartner;

public:
	Elephant(string name, MWorld * world, CreatureManager * cm, YVec3f pos) : AICreature(("Elephant" + toString(nbElephants)).c_str(), world, cm, pos, false, ELEPHANT_SPEED, ELEPHANT_SATIATION_DECAY, ELEPHANT_REPRODUCTION_THRESHOLD) {
		manager->registerCreature(this);
		switchState(new IdleState(this));
		nbElephants++;
	}

	/* WANDERING */
	void wander(float elapsed, bool firstTime)
	{
		if (firstTime || !hasNotReachedTarget())
		{
			findWanderTarget();
			goTo(targetPos);
		}
		else
			move(elapsed);
	}

	void findWanderTarget()
	{
		YVec3f wanderPos;
		MCube::MCubeType cubeTypeUnder;
		int x, y, z;

		do {
			x = rand() % MWorld::MAT_SIZE_CUBES;
			y = rand() % MWorld::MAT_SIZE_CUBES;
			z = world->getHighestPoint(x, y);
			wanderPos = YVec3f(x, y, z);
			cubeTypeUnder = world->getCube(x, y, z - 1)->getType();
		} while (!AStar::isTargetValid(position, wanderPos, world, canFly));

		targetPos = wanderPos;
	}

	/* EATING */
	virtual bool isEatTargetValid()
	{
		return world->getCube((int)realEatTarget.X, (int)realEatTarget.Y, (int)realEatTarget.Z)->isFruit();
	}

	virtual void eat()
	{
		world->getCube((int)realEatTarget.X, (int)realEatTarget.Y, (int)realEatTarget.Z)->setType(MCube::CUBE_BRANCHES);
		world->respawnFruit();
		//world->updateCube((int) realEatTarget.X, (int) realEatTarget.Y, (int) realEatTarget.Z);
		satiation += ELEPHANT_EAT_GAIN;
		if (satiation > 1.0f)
			satiation = 1.0f;
	}

	virtual bool setEatTarget(YVec3f target)
	{
		realEatTarget = target;
		eatTarget = YVec3f(target.X, target.Y, world->getHighestPoint(target.X, target.Y));
		return AStar::isTargetValid(position, eatTarget, world, canFly);
	}

	/* REPRODUCTION */
	virtual bool setPartner(AICreature * newPartner)
	{
		Elephant* elephantPartner = nullptr;
		if (newPartner->getType() == CreatureType::Elephant)
		{
			elephantPartner = (Elephant*)newPartner;

			if (partner == nullptr && elephantPartner != this && elephantPartner->partner == nullptr && elephantPartner->satiation >= 0.5f)
			{
				printf("%s: Going to reproduce with %s\n", name.c_str(), elephantPartner->name.c_str());
				partner = elephantPartner;
				elephantPartner->partner = this;
				elephantPartner->switchState(new ReproductionState(elephantPartner));
				return true;
			}
		}
		return false;
	}

	virtual bool isPartnerValid()
	{
		ReproductionState* partnerReprodState = dynamic_cast<ReproductionState*>(partner->state);
		if (partnerReprodState != nullptr)
		{
			return true;
		}
		return false;
	}

	virtual void reproduce()
	{
		new Elephant("", world, manager, YVec3f(position.X + 1, position.Y + 1, world->getHighestPoint(position.X + 1, position.Y + 1)));
		((Elephant*)partner)->satiation -= 0.3f;
		satiation -= 0.3f;
		partner->switchState(new IdleState((Elephant*)partner));
		switchState(new IdleState(this));
		partner->resetPartner();
	}

	virtual CreatureType * getType() {
		return CreatureType::Elephant;
	}
};