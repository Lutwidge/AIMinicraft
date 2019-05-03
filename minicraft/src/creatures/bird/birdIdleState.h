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
		// On r�initialise la spirale
		bird->initializeSpiralPath();
	}

	virtual void update(Bird *bird, float elapsed)
	{
		// Mise � jour de la sati�t� et check de si on est toujours en vie
		if (bird->updateEatGauge(elapsed))
		{
			//// Appel perception pour voir si on voit un predateur : (� voir si on garde cet �tat)
			//// bird->setState(Bird::fleeState);
			//// Sinon, on check si on a atteint la limite de reproduction
			//if (bird->canReproduce())
			//{
			//	// Si oui, on check s'il y a une target compatible, et si oui, on r�cup�re sa ref et on l'associe � ce bird
			//	if (bird->setPartner(targetBird)) // Si association r�ussie (pas de partenaire d�j� d�fini pour les deux)
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