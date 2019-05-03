#pragma once

#include "bird.h"
#include "../creatureState.h"

class BirdReproductionState : public CreatureState
{
public:
	virtual void enter(Bird* bird)
	{
	}

	virtual void update(Bird* bird, float elapsed)
	{
		// Mise à jour de la satiété et check de si on est toujours en vie
		if (bird->updateEatGauge(elapsed))
		{
			// Si on peut toujours se reproduire
			if (bird->canReproduce())
			{
				// Logique à ajouter
			}
		}
	}
};
