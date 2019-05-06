#pragma once

#include <type_traits>
#include <typeinfo>
#include "../AICreature.h"

#define DIR_COUNT 4
#define BIRD_SPEED 0.2f
#define BIRD_SATIATION_DECAY 0.02f
#define BIRD_REPRODUCTION_THRESHOLD 0.8f

class Bird : public AICreature
{
protected:
	#pragma region States
	struct BirdState : public State
	{
		Bird* bird;
		BirdState(AICreature* creature) : State(creature), bird((Bird*) creature) {}
	};

	struct IdleState : public BirdState
	{
		IdleState(Bird* bird) : BirdState(bird) {}

		virtual void enter()
		{
			// On réinitialise la spirale
			bird->initializeSpiralPath();
		}

		virtual void update(float elapsed) {

			if (bird->hasNotReachedTarget())
				bird->move(elapsed);
			else
				bird->incrementSpiralPath();

			// Mise à jour de la satiété et check de si on est toujours en vie
			//if (bird->updateSatiation(elapsed)) {
				//// Appel perception pour voir si on voit un predateur : (à voir si on garde cet état)
				//// bird->switchState(new FleeState(this));
				//// Sinon, on check si on a atteint la limite de reproduction
				//if (bird->canReproduce())
				//{
				//	// Si oui, on check s'il y a une target compatible, et si oui, on récupère sa ref et on l'associe à ce bird
				//	if (bird->setPartner(targetBird)) // Si association réussie (pas de partenaire déjà défini pour les deux)
				//	{
				//		bird->switchState(new ReproductionState(this));
				//	}
				//	else
				//	{
				//		// Appel perception pour voir si on voit un fruit
				//		// Si oui :
				//		// bird->setEatTarget(fruit);
				//		// bird->switchState(new EatState(this));
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
				//	// bird->switchState(new EatState(this));
				//	// Sinon, logique de mouvement en spirale de la recherche de cible
				//	if (!bird->hasReachedTarget())
				//		bird->move(elapsed);
				//	else
				//		bird->incrementSpiralPath();
				//}
			//}
		}

		virtual void exit() {}
	};

	struct EatState : public BirdState
	{
		EatState(Bird* bird) : BirdState(bird) {}

		virtual void enter()
		{
			bird->gotToEatTarget();
		}

		virtual void update(float elapsed)
		{
			// Mise à jour de la satiété et check de si on est toujours en vie
			if (bird->updateSatiation(elapsed))
			{
				// On continue d'avancer tant que la target n'est pas atteinte, que le fruit est toujours là, et qu'il n'y a pas de prédateur proche
				// Appel perception
				// Si on voit prédateur
				// bird->setState(Bird::fleeState);
				if (bird->isEatTargetValid()) // Sinon si le fruit est toujours là
				{
					if (bird->hasNotReachedTarget())
						bird->move(elapsed);
					else
						bird->eat();
				} else // Sinon, retour à l'état idle
					bird->switchState(new IdleState(bird));
			}
		}

		virtual void exit() {}
	};

	struct FleeState : public BirdState
	{
		FleeState(Bird* bird) : BirdState(bird) {}

		virtual void enter()
		{

		}

		virtual void update(float elapsed) {}

		virtual void exit() {}
	};

	struct ReproductionState : public BirdState
	{
		ReproductionState(Bird* bird) : BirdState(bird) {}

		virtual void enter() {
			// Définition point de rencontre valide avec le reprodTarget du Bird
			YVec3f meetingPoint = (bird->position + ((Bird*) bird->partner)->position) / 2;
			bird->goTo(YVec3f(meetingPoint.X, meetingPoint.Y, bird->world->getHighestPoint(meetingPoint.X, meetingPoint.Y)));
		}

		virtual void update(float elapsed) {
			// Mise à jour de la satiété et check de si on est toujours en vie
			if (bird->updateSatiation(elapsed)) {
				// Appel perception pour voir si on voit un predateur : (à voir si on garde cet état)
				bird->resetPartner(); // On retire tout partenaire potentiel
				bird->switchState(new FleeState(bird));

				// Sinon si la target est toujours en reproduction
				/* else */if (bird->isPartnerValid()) {
					if (bird->hasNotReachedTarget()) {
						bird->move(elapsed);
					} else if (!bird->partner->hasNotReachedTarget())// Reproduction si les deux sont arrivés
					{
						bird->reproduce();
						bird->resetPartner(); // On retire tout partenaire potentiel
						bird->switchState(new IdleState(bird));
					}
				} else // Sinon retour à idle
				{
					bird->resetPartner(); // On retire tout partenaire potentiel
					bird->switchState(new IdleState(bird));
				}
			}
		}

		virtual void exit() {}
	};
	#pragma endregion

	const int idleFlightHeight = 4;
	const float fruitSatiationGain = 0.3f;
	int pathLength;
	YVec3f directions[DIR_COUNT] = { YVec3f(1, 0, 0), YVec3f(0, 1, 0), YVec3f(-1, 0, 0), YVec3f(0, -1, 0) };
	int curDirIndex;
	YVec3f realEatTarget;

	virtual void setSpiralPath(int length, int index) {
		YVec3f addedDir = directions[index] * length;
		YVec3f target = YVec3f(position.X + addedDir.X, world->getHighestPoint(position.X + addedDir.X, position.Y + addedDir.Y) + idleFlightHeight, position.Z + addedDir.Z);
		printf("%f \n", position.X + addedDir.X);

		// Verification de si la target est appropriée (pas un arbre), sinon on réduit l'avancée dans la direction définie
		if (!AStar::isTargetValid(target, world, true)) {
			for (int i = 1; i <= length; i++) {
				YVec3f newTarget = target - directions[index] * i;
				if (AStar::isTargetValid(newTarget, world, true)) {
					target = newTarget;
					break;
				}
			}
		}

		goTo(target);
	}

public:
	Bird(MWorld *world, CreatureManager* cm, YVec3f pos) : AICreature("Bird", world, cm, pos, true, BIRD_SPEED, BIRD_SATIATION_DECAY, BIRD_REPRODUCTION_THRESHOLD) {
		switchState(new IdleState(this));
	}

	/* EATING */
	virtual bool isEatTargetValid() {
		return world->getCube((int) realEatTarget.X, (int) realEatTarget.Y, (int) realEatTarget.Z)->isFruit();
	}

	virtual void eat()
	{
		world->getCube((int) realEatTarget.X, (int) realEatTarget.Y, (int) realEatTarget.Z)->setType(MCube::CUBE_BRANCHES);
		world->respawnFruit();
		// TODO : Regénérer le monde (mais coûteux... comme le picking)
		satiation += fruitSatiationGain;
	}

	virtual void setEatTarget(YVec3f target) {
		realEatTarget = target;
		// TODO : Define eat target as the nearest air cube near the fruit

	}

	/* IDLE */
	virtual void initializeSpiralPath()
	{
		pathLength = 2;
		curDirIndex = 0;
		setSpiralPath(pathLength, curDirIndex);
	}

	virtual void incrementSpiralPath()
	{
		pathLength++;
		curDirIndex++;
		curDirIndex = curDirIndex % DIR_COUNT;
		setSpiralPath(pathLength, curDirIndex);
	}

	/* REPRODUCTION */
	virtual bool setPartner(AICreature* newPartner)
	{
		Bird* birdPartner = nullptr;
		if (typeid(newPartner) == typeid(Bird*)) {
			birdPartner = (Bird*) newPartner; // Casting to Bird pointer to access protected members
		}
		// Verification qu'il n'y a pas déjà de partenaires définis
		if (partner == nullptr && birdPartner->partner == nullptr)
		{
			partner = birdPartner;
			birdPartner->partner = this;
			birdPartner->switchState(new ReproductionState(this));
			return true;
		}
		return false;
	}

	virtual bool isPartnerValid()
	{
		if (typeid(partner->state) == typeid(ReproductionState))
		{
			return true;
		}
		return false;
	}

	virtual void reproduce()
	{
		// On crée une nouvelle instance, elle se register elle-même auprès du CreatureManager dans son constructeur
		new Bird(world, manager, position);
		// On empêche ensuite le partenaire de créer un autre enfant
		partner->resetPartner();
		partner->switchState(new IdleState(this));
	}
};