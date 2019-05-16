#pragma once

#include <type_traits>
#include <typeinfo>
#include "../AICreature.h"
#include "../ocelot/ocelot.h"

namespace
{
	static constexpr auto BIRD_SIGHT_RANGE = 15;
	static constexpr auto BIRD_DIR_COUNT = 4;
	static constexpr auto BIRD_SPEED = 0.1f;
	static constexpr auto BIRD_SATIATION_DECAY = 0.02f;
	static constexpr auto BIRD_REPRODUCTION_THRESHOLD = 0.8f;
	static constexpr auto BIRD_EAT_GAIN = 0.1f;
	static constexpr auto BIRD_MOVEMENT_RANGE = 8;
	static constexpr auto BIRD_FLEE_DISTANCE = 6;
	static constexpr auto BIRD_IDLE_HEIGHT = 8;
	static constexpr auto BIRD_SPIRAL_PATH_INCREMENT = 4;
}

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
			//printf("%s : Idle \n", bird->name.c_str());
			// On réinitialise la spirale
			bird->initializeSpiralPath();
		}

		virtual void update(float elapsed) {

			// Mise à jour de la satiété et check de si on est toujours en vie
			if (bird->updateSatiation(elapsed)) {
				// Si on voit un prédateur, on fuit
				bird->predator = bird->manager->perceptor->creatureSight(bird, CreatureType::Ocelot, BIRD_SIGHT_RANGE);
				if (bird->predator != nullptr) {
					bird->switchState(new FleeState(bird));
					return;
				}
				else {
					// Reproduction prioritaire
					if (bird->canReproduce()) {
						// Si oui, on check s'il y a une target compatible
						AICreature* targetBird = bird->manager->perceptor->creatureSight(bird, CreatureType::Bird, BIRD_SIGHT_RANGE);
						if (targetBird != nullptr) {
							// Si association réussie (pas de partenaire déjà défini pour les deux), on passe en reproduction pour les deux
							if (bird->setPartner(targetBird)) {
								bird->switchState(new ReproductionState(bird));
								return;
							}
						}
					}
					else {
						// Sinon, on regarde si on voit un fruit
						YVec3f fruit;
						if (bird->manager->perceptor->blockSight(bird, MCube::CUBE_FRUIT, BIRD_SIGHT_RANGE, fruit)) {
							if (bird->setEatTarget(fruit)) {
								bird->switchState(new EatState(bird));
								return;
							}
						}
					}
				}

				// Sinon, mouvement en spirale (y compris quand on cherche à se reproduire : priorité de la recherche de partenaire)
				if (bird->hasNotReachedTarget())
					bird->move(elapsed);
				else
					bird->incrementSpiralPath();
			}
		}

		virtual void exit() {}
	};

	struct EatState : public BirdState
	{
		EatState(Bird* bird) : BirdState(bird) {}

		virtual void enter()
		{
			//printf("%s : Eat \n", bird->name.c_str());
			bird->gotToEatTarget();
		}

		virtual void update(float elapsed)
		{
			// Mise à jour de la satiété et check de si on est toujours en vie
			if (bird->updateSatiation(elapsed))
			{
				// Si on voit un prédateur, on fuit
				bird->predator = bird->manager->perceptor->creatureSight(bird, CreatureType::Ocelot, BIRD_SIGHT_RANGE);
				if (bird->predator != nullptr) {
					bird->switchState(new FleeState(bird));
					return;
				}
				else {
					// Sinon si le fruit est toujours là, on continue vers lui jusqu'à l'atteindre
					if (bird->isEatTargetValid()) 
					{
						if (bird->hasNotReachedTarget())
							bird->move(elapsed);
						else {
							bird->eat();
							bird->switchState(new IdleState(bird));
							return;
						}
					} 
					// Sinon, retour à l'état idle
					else {
						bird->switchState(new IdleState(bird));
						return;
					}
				}
			}
		}

		virtual void exit() {}
	};

	struct FleeState : public BirdState
	{
		FleeState(Bird* bird) : BirdState(bird) {}

		virtual void enter()
		{
			// Definir target de fuite
			YVec3f fleeTarget = bird->position + (bird->position - bird->predator->position).normalize() * BIRD_FLEE_DISTANCE;
			// On s'assure que la target de fuite est valide
			fleeTarget = bird->world->getNearestAirCube(fleeTarget.X, fleeTarget.Y, fleeTarget.Z);
			bird->goTo(fleeTarget);
		}

		virtual void update(float elapsed)
		{
			// Mise à jour de la satiété et check de si on est toujours en vie
			if (bird->updateSatiation(elapsed))
			{
				// On avance jusqu'à fuir à la target définie
				if (bird->hasNotReachedTarget())
					bird->move(elapsed);
				else
					bird->switchState(new IdleState(bird));
			}
		}

		virtual void exit() {}
	};

	struct ReproductionState : public BirdState
	{
		ReproductionState(Bird* bird) : BirdState(bird) {}

		virtual void enter() {
			//printf("%s : Reproduction \n", bird->name.c_str());
			// Définition point de rencontre valide avec le reprodTarget du Bird
			YVec3f meetingPoint = (bird->position + ((Bird*) bird->partner)->position) / 2;
			meetingPoint = YVec3f((int) meetingPoint.X, (int) meetingPoint.Y, (int) meetingPoint.Z);
			bird->goTo(YVec3f(meetingPoint.X, meetingPoint.Y, bird->world->getHighestPoint(meetingPoint.X, meetingPoint.Y)));
		}

		virtual void update(float elapsed)
		{
			// Mise à jour de la satiété et check de si on est toujours en vie
			if (bird->updateSatiation(elapsed))
			{
				bird->predator = bird->manager->perceptor->creatureSight(bird, CreatureType::Ocelot, BIRD_SIGHT_RANGE);
				if (bird->predator != nullptr)
				{
					bird->resetPartner(); // On retire tout partenaire potentiel
					bird->switchState(new FleeState(bird));
					return;
				}
				else
				{
					// Sinon si la target est toujours en reproduction, on bouge jusqu'à atteindre la cible
					if (bird->isPartnerValid())
					{
						if (bird->hasNotReachedTarget())
						{
							bird->move(elapsed);
							return;
						}
						// Reproduction seulement si les deux sont arrivés
						else if (!bird->partner->hasNotReachedTarget())
						{
							bird->reproduce();
							return;
						}
					} 
					else // Sinon retour à idle
					{
						bird->resetPartner(); // On retire tout partenaire potentiel
						bird->switchState(new IdleState(bird));
						return;
					}
				}
			}
		}

		virtual void exit() {}
	};
	#pragma endregion

	int pathLength;
	YVec3f directions[DIR_COUNT] = { YVec3f(1, 0, 0), YVec3f(0, 1, 0), YVec3f(-1, 0, 0), YVec3f(0, -1, 0) };
	int curDirIndex;
	YVec3f realEatTarget;

	virtual void setSpiralPath(int length, int index) {
		YVec3f addedDir = directions[index] * length;
		YVec3f target = YVec3f(position.X + addedDir.X, position.Y + addedDir.Y, world->getHighestPoint(position.X + addedDir.X, position.Y + addedDir.Y) + BIRD_IDLE_HEIGHT);

		goTo(target);
	}

public:
	Bird(string name, MWorld *world, CreatureManager* cm, YVec3f pos) : AICreature(name, world, cm, pos, true, BIRD_SPEED, BIRD_SATIATION_DECAY, BIRD_REPRODUCTION_THRESHOLD) {
		manager->registerCreature(this);
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
		// Regénérer le monde (mais coûteux... comme le picking)
		//world->updateCube((int) realEatTarget.X, (int) realEatTarget.Y, (int) realEatTarget.Z);
		satiation += BIRD_EAT_GAIN;
		if (satiation > 1.0f)
			satiation = 1.0f;
	}

	virtual bool setEatTarget(YVec3f target) {
		realEatTarget = target;
		// Définir la target comme le cube d'air le plus proche du fruit
		eatTarget = world->getNearestAirCube(target.X, target.Y, target.Z);
		if (eatTarget == realEatTarget)
			return false;
		else
			return true;
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
		pathLength += BIRD_SPIRAL_PATH_INCREMENT;
		if (pathLength > MWorld::MAT_SIZE_METERS / 4)
			pathLength = MWorld::MAT_SIZE_METERS / 4;
		curDirIndex++;
		curDirIndex = curDirIndex % DIR_COUNT;
		setSpiralPath(pathLength, curDirIndex);
	}

	/* REPRODUCTION */
	virtual bool setPartner(AICreature* newPartner)
	{
		Bird* birdPartner = nullptr;
		if (newPartner->getType() == CreatureType::Bird) {
			birdPartner = (Bird*) newPartner; // Cast en Bird pointer pour accéder aux membres protégés

			// Verification qu'il n'y a pas déjà de partenaires définis
			if (partner == nullptr && birdPartner->partner == nullptr) {
				partner = birdPartner;
				birdPartner->partner = this;
				birdPartner->switchState(new ReproductionState(birdPartner));
				return true;
			}
		}
		return false;
	}

	virtual bool isPartnerValid()
	{
		ReproductionState* partnerReprodState = dynamic_cast<ReproductionState*>(partner->state);
		if (partnerReprodState != nullptr)
		{
			return true;
		}
		return false;
	}

	virtual void reproduce()
	{
		// On crée une nouvelle instance, elle se register elle-même auprès du CreatureManager dans son constructeur
		new Bird("Bird", world, manager, position);
		// On empêche ensuite le partenaire de créer un autre enfant
		((Bird*) partner)->satiation -= 0.3f;
		satiation -= 0.3f;
		partner->switchState(new IdleState((Bird*) partner));
		switchState(new IdleState(this));
		partner->resetPartner();
	}

	virtual CreatureType* getType() {
		return CreatureType::Bird;
	}
};