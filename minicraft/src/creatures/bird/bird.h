#pragma once

#include "../creature.h"
#include "../creatureState.h"
#include "birdDeadState.h"

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

	virtual void setSpiralPath(int length, int index)
	{
		goTo(YVec3f(position.X, world->getSurface(position.X, position.Y) + idleFlightHeight, position.Z) + directions[index] * length);
	}

public:
	static class BirdDeadState* deadState;
	static class BirdFleeState* fleeState;
	static class BirdEatState* eatState;
	static class BirdReproductionState* reprodState;
	static class BirdIdleState* idleState;

	Bird(MWorld* world, YVec3f pos) : Creature("Bird", world, pos, true, speed, decay)
	{
		// Initialiser à l'état idle
		setState(idleState);
	}

	/* GENERAL */
	virtual void update(float elapsed)
	{
		state->update(this, elapsed);
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

	virtual void setState(CreatureState* newState)
	{
		state = newState;
		state->enter(this);
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
		// TODO : Regénérer le monde (mais coûteux... comme le picking)
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
};