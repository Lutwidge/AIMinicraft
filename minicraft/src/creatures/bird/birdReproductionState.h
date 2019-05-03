#pragma once

#include "bird.h"
#include "../creatureState.h"
#include "birdFleeState.h"

class BirdReproductionState : public CreatureState
{
public:
	virtual void enter(Bird *bird)
	{
		// TODO : D�finition point de rencontre valide avec le reprodTarget du Bird
	}

	virtual void update(Bird *bird, float elapsed)
	{
		// Mise � jour de la sati�t� et check de si on est toujours en vie
		if (bird->updateEatGauge(elapsed))
		{
			// Appel perception pour voir si on voit un predateur : (� voir si on garde cet �tat)
			bird->resetPartner(); // On retire tout partenaire potentiel
			bird->setState(Bird::fleeState);

			// Sinon si la target est toujours en reproduction
			/* else */if (bird->isPartnerValid())
			{
				if (bird->hasReachedTarget())
				{
					bird->move(elapsed);
				}
				else // Reproduction
				{
					bird->reproduce();
					bird->resetPartner(); // On retire tout partenaire potentiel
					bird->setState(Bird::idleState);
				}
			}
			else // Sinon retour � idle
			{
				bird->resetPartner(); // On retire tout partenaire potentiel
				bird->setState(Bird::idleState);
			}
		}
	}
};
