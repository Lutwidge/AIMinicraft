#pragma once

#include <type_traits>
#include <typeinfo>
#include "../AICreature.h"
//#include "../trap/trap.h"

#define DIR_COUNT 4
#define GOAT_DIR_COUNT 4
#define GOAT_SPEED 0.05f
#define GOAT_SATIATION_DECAY 0.01f
#define GOAT_REPRODUCTION_THRESHOLD 0.8f
#define GOAT_SIGHT_RANGE 15
#define GOAT_SPIRAL_PATH_INCREMENT 4
#define GOAT_EAT_GAIN 0.1f
#define GOAT_FLEE_DISTANCE 6

class Goat : public AICreature
{
protected:
#pragma region States
	struct GoatState : public State
	{
		Goat* goat;
		GoatState(AICreature* creature) : State(creature), goat((Goat*)creature) {}
	};

	struct IdleState : public GoatState
	{
		IdleState(Goat* goat) : GoatState(goat) {}

		virtual void enter()
		{
			//printf("%s : Idle \n", goat->name.c_str());
			// On réinitialise la spirale
			goat->initializeSpiralPath();
		}

		virtual void update(float elapsed) {

			// Mise à jour de la satiété et check de si on est toujours en vie
			if (goat->updateSatiation(elapsed)) {
				// Si on voit un prédateur, on fuit
				goat->predator = goat->manager->perceptor->creatureSight(goat, CreatureType::Wolf, GOAT_SIGHT_RANGE);
				if (goat->predator != nullptr) {
					goat->switchState(new FleeState(goat));
					return;
				}
				else {
					// Reproduction prioritaire
					if (goat->canReproduce()) {
						// Si oui, on check s'il y a une target compatible
						AICreature* targetGoat = goat->manager->perceptor->creatureSight(goat, CreatureType::Goat, GOAT_SIGHT_RANGE);
						if (targetGoat != nullptr) {
							// Si association réussie (pas de partenaire déjà défini pour les deux), on passe en reproduction pour les deux
							if (goat->setPartner(targetGoat)) {
								goat->switchState(new ReproductionState(goat));
								return;
							}
						}
					}

					// Sinon, on regarde si on voit un fruit
					YVec3f fruit;
					AICreature* targetTrap;
					if (goat->manager->perceptor->blockSight(goat, MCube::CUBE_FRUIT, GOAT_SIGHT_RANGE, fruit)) {
						if (goat->setEatTarget(fruit)) {
							goat->switchState(new EatState(goat));
							return;
						}
					}
					//Sinon on regarde si on voit un piège
					else if (nullptr != (targetTrap = goat->manager->perceptor->creatureSight(goat, CreatureType::Trap, GOAT_SIGHT_RANGE)))
					{
						if (goat->setEatTarget(targetTrap->position)) {
							goat->switchState(new EatState(goat));
							return;
						}
					}
				}

				// Sinon, mouvement en spirale
				if (goat->hasNotReachedTarget())
					goat->move(elapsed);
				else
					goat->incrementSpiralPath();
			}
		}

		virtual void exit() {}
	};

	struct EatState : public GoatState
	{
		EatState(Goat* goat) : GoatState(goat) {}

		virtual void enter()
		{
			//printf("%s : Eat \n", goat->name.c_str());
			goat->gotToEatTarget();
		}

		virtual void update(float elapsed)
		{
			// Mise à jour de la satiété et check de si on est toujours en vie
			if (goat->updateSatiation(elapsed))
			{
				// Si on voit un prédateur, on fuit
				goat->predator = goat->manager->perceptor->creatureSight(goat, CreatureType::Ocelot, GOAT_SIGHT_RANGE);
				if (goat->predator != nullptr) {
					goat->switchState(new FleeState(goat));
					return;
				}
				else {
					// Sinon si le fruit est toujours là, on continue vers lui jusqu'à l'atteindre
					if (goat->isEatTargetValid())
					{
						if (goat->hasNotReachedTarget())
							goat->move(elapsed);
						else {
							goat->eat();
							return;
						}
					}
					// Sinon, retour à l'état idle
					else {
						goat->switchState(new IdleState(goat));
						return;
					}
				}
			}
		}

		virtual void exit() {}
	};

	struct FleeState : public GoatState
	{
		FleeState(Goat* goat) : GoatState(goat) {}

		virtual void enter()
		{
			// Definir target de fuite
			YVec3f fleeTarget = goat->position + (goat->position - goat->predator->position).normalize() * GOAT_FLEE_DISTANCE;
			// On s'assure que la target de fuite est valide
			fleeTarget = goat->world->getNearestAirCube(fleeTarget.X, fleeTarget.Y, fleeTarget.Z);
			goat->goTo(fleeTarget);
		}

		virtual void update(float elapsed)
		{
			// Mise à jour de la satiété et check de si on est toujours en vie
			if (goat->updateSatiation(elapsed))
			{
				// On avance jusqu'à fuir à la target définie
				if (goat->hasNotReachedTarget())
					goat->move(elapsed);
				else
					goat->switchState(new IdleState(goat));
			}
		}

		virtual void exit() {}
	};

	struct ReproductionState : public GoatState
	{
		ReproductionState(Goat* goat) : GoatState(goat) {}

		virtual void enter() {
			//printf("%s : Reproduction \n", goat->name.c_str());
			// Définition point de rencontre valide avec le reprodTarget du Goat
			YVec3f meetingPoint = (goat->position + ((Goat*)goat->partner)->position) / 2;
			meetingPoint = YVec3f((int)meetingPoint.X, (int)meetingPoint.Y, (int)meetingPoint.Z);
			goat->goTo(YVec3f(meetingPoint.X, meetingPoint.Y, goat->world->getHighestPoint(meetingPoint.X, meetingPoint.Y)));
		}

		virtual void update(float elapsed)
		{
			// Mise à jour de la satiété et check de si on est toujours en vie
			if (goat->updateSatiation(elapsed))
			{
				goat->predator = goat->manager->perceptor->creatureSight(goat, CreatureType::Ocelot, GOAT_SIGHT_RANGE);
				if (goat->predator != nullptr)
				{
					goat->resetPartner(); // On retire tout partenaire potentiel
					goat->switchState(new FleeState(goat));
					return;
				}
				else
				{
					// Sinon si la target est toujours en reproduction, on bouge jusqu'à atteindre la cible
					if (goat->isPartnerValid())
					{
						if (goat->hasNotReachedTarget())
						{
							goat->move(elapsed);
							return;
						}
						// Reproduction seulement si les deux sont arrivés
						else if (!goat->partner->hasNotReachedTarget())
						{
							goat->reproduce();
							return;
						}
					}
					else // Sinon retour à idle
					{
						goat->resetPartner(); // On retire tout partenaire potentiel
						goat->switchState(new IdleState(goat));
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
		YVec3f target = YVec3f(position.X + addedDir.X, position.Y + addedDir.Y, world->getHighestPoint(position.X + addedDir.X, position.Y + addedDir.Y));

		goTo(target);
	}

public:
	Goat(string name, MWorld * world, CreatureManager * cm, YVec3f pos) : AICreature(name, world, cm, pos, false, GOAT_SPEED, GOAT_SATIATION_DECAY, GOAT_REPRODUCTION_THRESHOLD) {
		manager->registerCreature(this);
		switchState(new IdleState(this));
	}

	/* EATING */
	virtual bool isEatTargetValid() {
		return world->getCube((int)realEatTarget.X, (int)realEatTarget.Y, (int)realEatTarget.Z)->isFruit();
	}

	virtual void eat()
	{
		world->getCube((int)realEatTarget.X, (int)realEatTarget.Y, (int)realEatTarget.Z)->setType(MCube::CUBE_BRANCHES);
		world->respawnFruit();
		// Regénérer le monde (mais coûteux... comme le picking)
		//world->updateCube((int) realEatTarget.X, (int) realEatTarget.Y, (int) realEatTarget.Z);
		satiation += GOAT_EAT_GAIN;
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
		pathLength += GOAT_SPIRAL_PATH_INCREMENT;
		curDirIndex++;
		curDirIndex = curDirIndex % DIR_COUNT;
		setSpiralPath(pathLength, curDirIndex);
	}

	/* REPRODUCTION */
	virtual bool setPartner(AICreature * newPartner)
	{
		Goat* goatPartner = nullptr;
		if (newPartner->getType() == CreatureType::Goat) {
			goatPartner = (Goat*)newPartner; // Cast en Goat pointer pour accéder aux membres protégés

			// Verification qu'il n'y a pas déjà de partenaires définis
			if (partner == nullptr && goatPartner->partner == nullptr) {
				partner = goatPartner;
				goatPartner->partner = this;
				goatPartner->switchState(new ReproductionState(goatPartner));
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
		new Goat("Goat", world, manager, position);
		// On empêche ensuite le partenaire de créer un autre enfant
		((Goat*)partner)->satiation -= 0.3f;
		satiation -= 0.3f;
		partner->switchState(new IdleState((Goat*)partner));
		switchState(new IdleState(this));
		partner->resetPartner();
	}

	virtual CreatureType* getType() {
		return CreatureType::Goat;
	}
};