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
		// On r�initialise la spirale
		bird->initializeSpiralPath();
	}

	virtual void update(Bird* bird, float elapsed)
	{
		// Mise � jour de la sati�t� et check de si on est toujours en vie
		if (bird->updateEatGauge(elapsed))
		{
			// Appel perception
			// Si on voit un predateur : (� voir si on garde cet �tat)
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