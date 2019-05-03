#pragma once

#include "bird.h"
#include "../creatureState.h"
#include "birdFleeState.h"
#include "birdEatState.h"

class BirdIdleState : public CreatureState
{
public:
	virtual void enter(Bird* bird)
	{
		// On réinitialise la spirale
		bird->initializeSpiralPath();
	}

	virtual void update(Bird* bird, float elapsed)
	{
		// Mise à jour de la satiété et check de si on est toujours en vie
		if (bird->updateEatGauge(elapsed))
		{
			// Appel perception
			// Si on voit un predateur : (à voir si on garde cet état)
			//bird->setState(Bird::fleeState);
			// Sinon si on voit un fruit :
			//bird->setEatTarget(fruit);
			//bird->setState(Bird::eatState);
			// Sinon, logique de mouvement en spirale de la recherche de cible
			if (!bird->hasReachedTarget())
				bird->move(elapsed);
			else
				bird->incrementSpiralPath();
		}
	}
};