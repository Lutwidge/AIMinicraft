#pragma once

#include "bird.h"
#include "../creatureState.h"

class BirdDeadState : public CreatureState
{
public:
	virtual void enter(Bird* bird)
	{
		// On donne à l'oiseau sa target finale
		bird->goToFallTarget();
	}

	virtual void update(Bird* bird, float elapsed)
	{
		// On fait tomber l'oiseau jusqu'au sol
		if (!bird->hasReachedTarget())
			bird->move(elapsed);
	}
};