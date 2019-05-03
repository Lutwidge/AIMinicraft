#pragma once

#include "bird.h"
#include "../creatureState.h"
#include "birdFleeState.h"
#include "birdEatState.h"

class BirdIdleState : public CreatureState
{
public:
	virtual void enter(Bird *bird)
	{
		// On réinitialise la spirale
		bird->initializeSpiralPath();
	}

	virtual void update(Bird *bird, float elapsed)
	{
		// Mise à jour de la satiété et check de si on est toujours en vie
		if (bird->updateEatGauge(elapsed))
		{
			//// Appel perception pour voir si on voit un predateur : (à voir si on garde cet état)
			//// bird->setState(Bird::fleeState);
			//// Sinon, on check si on a atteint la limite de reproduction
			//if (bird->canReproduce())
			//{
			//	// Si oui, on check s'il y a une target compatible, et si oui, on récupère sa ref et on l'associe à ce bird
			//	if (bird->setPartner(targetBird)) // Si association réussie (pas de partenaire déjà défini pour les deux)
			//	{
			//		bird->setState(Bird::reprodState);
			//	}
			//	else
			//	{
			//		// Appel perception pour voir si on voit un fruit
			//		// Si oui :
			//		// bird->setEatTarget(fruit);
			//		// bird->setState(Bird::eatState);
			//		// Sinon, logique de mouvement en spirale de la recherche de cible
			//		if (!bird->hasReachedTarget())
			//			bird->move(elapsed);
			//		else
			//			bird->incrementSpiralPath();
			//	}
			//}
			//else
			//{
			//	// Appel perception pour voir si on voit un fruit
			//	// Si oui :
			//	// bird->setEatTarget(fruit);
			//	// bird->setState(Bird::eatState);
			//	// Sinon, logique de mouvement en spirale de la recherche de cible
			//	if (!bird->hasReachedTarget())
			//		bird->move(elapsed);
			//	else
			//		bird->incrementSpiralPath();
			//}
		}
	}
};