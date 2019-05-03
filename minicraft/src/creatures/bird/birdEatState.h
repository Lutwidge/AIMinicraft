#pragma once

#include "bird.h"
#include "../creatureState.h"
#include "birdIdleState.h"

class BirdEatState : public CreatureState
{
public:
	virtual void enter(Bird *bird)
	{
		bird->gotToEatTarget();
	}

	virtual void update(Bird *bird, float elapsed)
	{
		// Mise à jour de la satiété et check de si on est toujours en vie
		if (bird->updateEatGauge(elapsed))
		{
			// On continue d'avancer tant que la target n'est pas atteinte, que le fruit est toujours là, et qu'il n'y a pas de prédateur proche
			// Appel perception
			// Si on voit prédateur
			// bird->setState(Bird::fleeState);
			if (bird->isEatTargetValid()) // Sinon si le fruit est toujours là
			{
				if (!bird->hasReachedTarget())
					bird->move(elapsed);
				else
					bird->eat();
			}
			else // Sinon, retour à l'état idle
				bird->setState(Bird::idleState);
		}
	}
};