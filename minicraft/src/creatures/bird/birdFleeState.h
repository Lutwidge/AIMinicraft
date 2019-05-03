#pragma once

#include "bird.h"
#include "../creatureState.h"

class BirdFleeState : public CreatureState
{
public:
	virtual void enter(Bird *bird)
	{
		// On retire tout partenaire potentiel
		bird->resetPartner();
	}

	virtual void update(Bird *bird, float elapsed)
	{
	}
};
