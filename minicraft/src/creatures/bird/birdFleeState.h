#pragma once

#include "bird.h"
#include "../creatureState.h"

class BirdFleeState : public CreatureState
{
public:
	virtual void enter(Bird* bird)
	{
	}

	virtual void update(Bird* bird, float elapsed)
	{
	}
};
