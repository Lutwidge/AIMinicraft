#pragma once

#include "../creature.h"
#include "../creatureState.h"
#include "birdDeadState.h"
#include "birdIdleState.h"
#include "birdReproductionState.h"

#define DIR_COUNT 4

class Bird : public Creature
{
protected:
	CreatureState* state;
	const float speed = 0.2f;
	const float decay = 0.02f;
	const float reproductionThreshold = 0.8f;
	const int idleFlightHeight = 4;
	int pathLength;
	YVec3f directions[DIR_COUNT] = { YVec3f(1, 0, 0), YVec3f(0, 1, 0), YVec3f(-1, 0, 0), YVec3f(0, -1, 0) };
	int curDirIndex;
	YVec3f eatTarget;
	float fruitGaugeGain = 0.3f;
	Bird* partner = nullptr;

	virtual void setSpiralPath(int length, int index)
	{
		YVec3f addedDir = directions[index] * length;
		YVec3f target = YVec3f(position.X + addedDir.X, world->getSurface(position.X + addedDir.X, position.Y + addedDir.Y) + idleFlightHeight, position.Z + addedDir.Z);

		// Verification de si la target est appropri�e (pas un arbre), sinon on r�duit l'avanc�e dans la direction d�finie
		if (!AStar::isTargetValid(target, world, true))
		{
			for (int i = 1; i <= length; i++)
			{
				YVec3f newTarget = target - directions[index] * i;
				if (AStar::isTargetValid(newTarget, world, true))
				{
					target = newTarget;
					break;
				}
			}
		}

		goTo(target);
	}

public:
	static class BirdDeadState *deadState;
	static class BirdFleeState *fleeState;
	static class BirdEatState *eatState;
	static class BirdReproductionState *reprodState;
	static class BirdIdleState *idleState;

	Bird(MWorld *world, YVec3f pos) : Creature("Bird", world, pos, true, speed, decay)
	{
		// Initialiser � l'�tat idle
		setState(idleState);
	}

	/* GENERAL */
	virtual void update(float elapsed)
	{
		state->update(this, elapsed);
	}

	virtual void setState(CreatureState* newState)
	{
		state = newState;
		state->enter(this);
	}

	virtual CreatureState* getState()
	{
		return state;
	}

	virtual bool updateEatGauge(float elapsed)
	{
		eatGauge -= gaugeDecay * elapsed;
		if (eatGauge <= 0.0f)
		{
			death();
			return false;
		}
		return true;
	}
	virtual bool canReproduce()
	{
		return eatGauge >= reproductionThreshold;
	}

	/* DEATH */
	virtual void goToFallTarget()
	{
		goTo(YVec3f(position.X, position.Y, world->getSurface(position.X, position.Y)));
	}

	virtual void death()
	{
		state = deadState;
		state->enter(this);
	}

	/* EATING */
	virtual void setEatTarget(YVec3f target)
	{
		eatTarget = target;
	}

	virtual void gotToEatTarget()
	{
		goTo(eatTarget);
	}

	virtual bool isEatTargetValid()
	{
		return world->getCube((int)eatTarget.X, (int)eatTarget.Y, (int)eatTarget.Z)->isFruit();
	}

	virtual void eat()
	{
		world->getCube((int)eatTarget.X, (int)eatTarget.Y, (int)eatTarget.Z)->setType(MCube::CUBE_BRANCHES);
		world->respawnFruit();
		// TODO : Reg�n�rer le monde (mais co�teux... comme le picking)
		eatGauge += fruitGaugeGain;
	}

	/* IDLE */
	virtual void initializeSpiralPath()
	{
		pathLength = 2;
		curDirIndex = 0;
		setSpiralPath(pathLength, curDirIndex);
	}

	virtual void incrementSpiralPath()
	{
		pathLength++;
		curDirIndex++;
		curDirIndex % 4;
		setSpiralPath(pathLength, curDirIndex);
	}

	/* REPRODUCTION */
	virtual bool setPartner(Bird* newPartner)
	{
		// Verification qu'il n'y a pas d�j� de partenaires d�finis
		if (partner != nullptr && newPartner->partner != nullptr)
		{
			partner = newPartner;
			newPartner->partner = this;
			newPartner->setState(reprodState);
			return true;
		}
		return false;
	}

	virtual void resetPartner()
	{
		partner = nullptr;
	}

	virtual bool isPartnerValid()
	{
		if (partner->getState() == Bird::reprodState)
		{
			return true;
		}
		return false;
	}

	virtual void reproduce()
	{
		// TODO : Cr�er une nouvelle instance sur un manager ?
		// On emp�che ensuite le partenaire de cr�er un autre enfant
		partner->resetPartner();
		partner->setState(Bird::idleState);
	}
};