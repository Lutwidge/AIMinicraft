#pragma once

#include <creatures/AICreature.h>

namespace {
	static constexpr auto BEAR_SIGHT_RANGE = 5;
	static constexpr auto BEAR_DIR_COUNT = 4;
	static constexpr auto BEAR_SPEED = 0.05f;
	static constexpr auto BEAR_SATIATION_DECAY = 0.01f;
	static constexpr auto BEAR_REPRODUCTION_THRESHOLD = 0.75f;
	static constexpr auto BEAR_EAT_GAIN = 0.4f;
}

class Bear : public AICreature {
protected:
	struct BearState : public State {
		Bear* bear;
		BearState(AICreature* creature) : State(creature), bear(static_cast<Bear*>(creature)) {}
	};

	struct IdleState : public BearState {
		IdleState(Bear* bear) : BearState(bear) {}

		virtual void enter() {
			//printf("%s : Idle \n", bear->name.c_str());
			// On réinitialise la spirale
			bear->initializeSpiralPath();
		}

		virtual void update(float elapsed) {

			// Mise à jour de la satiété et check de si on est toujours en vie
			if (bear->updateSatiation(elapsed)) {
				// Si on voit un prédateur, on fuit
				if (bear->manager->perceptor->creatureSight(bear, CreatureType::Snake, BEAR_SIGHT_RANGE) != nullptr) {
					bear->switchState(new FleeState(bear));
					return;
				}
				else {
					// Reproduction prioritaire
					if (bear->canReproduce()) {
						// Si oui, on check s'il y a une target compatible
						AICreature* targetBird = bear->manager->perceptor->creatureSight(bear, CreatureType::Bear, BEAR_SIGHT_RANGE);
						if (targetBird != nullptr) {
							// Si association réussie (pas de partenaire déjà défini pour les deux), on passe en reproduction pour les deux
							if (bear->setPartner(targetBird)) {
								bear->switchState(new ReproductionState(bear));
								return;
							}
						}
					}

					// Sinon, on regarde si on voit un fruit
					YVec3f fruit;
					if (bear->manager->perceptor->blockSight(bear, MCube::CUBE_FRUIT, BEAR_SIGHT_RANGE, fruit)) {
						if (bear->setEatTarget(fruit)) {
							bear->switchState(new EatState(bear));
							return;
						}
					}
				}

				// Sinon, mouvement en spirale
				if (bear->hasNotReachedTarget())
					bear->move(elapsed);
				else
					bear->incrementSpiralPath();
			}
		}

		virtual void exit() {}
	};

	struct EatState : public BearState
	{
		EatState(Bear* bear) : BearState(bear) {}

		virtual void enter() {
			//printf("%s : Eat \n", bear->name.c_str());
			bear->gotToEatTarget();
		}

		virtual void update(float elapsed) {
			// Mise à jour de la satiété et check de si on est toujours en vie
			if (bear->updateSatiation(elapsed)) {
				// Si on voit un prédateur, on fuit
				if (bear->manager->perceptor->creatureSight(bear, CreatureType::Ocelot, BEAR_SIGHT_RANGE) != nullptr) {
					bear->switchState(new FleeState(bear));
					return;
				}
				else {
					// Sinon si le fruit est toujours là, on continue vers lui jusqu'à l'atteindre
					if (bear->isEatTargetValid()) {
						if (bear->hasNotReachedTarget())
							bear->move(elapsed);
						else {
							bear->eat();
							return;
						}
					}
					// Sinon, retour à l'état idle
					else {
						bear->switchState(new IdleState(bear));
						return;
					}
				}
			}
		}

		virtual void exit() {}
	};

	struct FleeState : public BearState {
		FleeState(Bear* bear) : BearState(bear) {}

		virtual void enter() {

		}

		virtual void update(float elapsed) {}

		virtual void exit() {}
	};

	struct ReproductionState : public BearState {
		ReproductionState(Bear* bear) : BearState(bear) {}

		virtual void enter() {
			//printf("%s : Reproduction \n", bear->name.c_str());
			// Définition point de rencontre valide avec le reprodTarget du Bear
			YVec3f meetingPoint = (bear->position + ((Bear*)bear->partner)->position) / 2;
			meetingPoint = YVec3f((int)meetingPoint.X, (int)meetingPoint.Y, (int)meetingPoint.Z);
			bear->goTo(YVec3f(meetingPoint.X, meetingPoint.Y, bear->world->getHighestPoint(meetingPoint.X, meetingPoint.Y)));
		}

		virtual void update(float elapsed) {
			// Mise à jour de la satiété et check de si on est toujours en vie
			if (bear->updateSatiation(elapsed)) {
				// Si on voit un prédateur, on fuit
				if (bear->manager->perceptor->creatureSight(bear, CreatureType::Ocelot, BEAR_SIGHT_RANGE) != nullptr) {
					bear->resetPartner(); // On retire tout partenaire potentiel
					bear->switchState(new FleeState(bear));
					return;
				}
				else {
					// Sinon si la target est toujours en reproduction, on bouge jusqu'à atteindre la cible
					if (bear->isPartnerValid()) {
						if (bear->hasNotReachedTarget()) {
							bear->move(elapsed);
							return;
						}
						// Reproduction seulement si les deux sont arrivés
						else if (!bear->partner->hasNotReachedTarget()) {
							bear->reproduce();
							return;
						}
					}
					else // Sinon retour à idle
					{
						bear->resetPartner(); // On retire tout partenaire potentiel
						bear->switchState(new IdleState(bear));
						return;
					}
				}
			}
		}

		virtual void exit() {}
	};
#pragma endregion

	int pathLength;
	YVec3f directions[BEAR_DIR_COUNT] = { YVec3f(1, 0, 0), YVec3f(-1, 0, 0), YVec3f(0, -1, 0), YVec3f(0, 1, 0) };
	int curDirIndex;
	YVec3f realEatTarget;

	virtual void setSpiralPath(int length, int index) {
		YVec3f addedDir = directions[index] * length;
		YVec3f target = YVec3f(position.X + addedDir.X, position.Y + addedDir.Y, world->getHighestPoint(position.X + addedDir.X, position.Y + addedDir.Y));

		goTo(target);
	}

public:
	Bear(string name, MWorld *world, CreatureManager* cm, YVec3f pos) : AICreature(name, world, cm, pos, true, BEAR_SPEED, BEAR_SATIATION_DECAY, BEAR_REPRODUCTION_THRESHOLD) {
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
		world->updateCube((int)realEatTarget.X, (int)realEatTarget.Y, (int)realEatTarget.Z);
		satiation += BEAR_EAT_GAIN;
		if (satiation > 1.0f)
			satiation = 1.0f;
	}

	virtual bool setEatTarget(YVec3f target) {
		realEatTarget = target;
		// Define eat target as the nearest air cube near the fruit
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
		pathLength += 1;
		curDirIndex++;
		curDirIndex = curDirIndex % BEAR_DIR_COUNT;
		setSpiralPath(pathLength, curDirIndex);
	}

	/* REPRODUCTION */
	virtual bool setPartner(AICreature* newPartner)
	{
		Bear* bearPartner = nullptr;
		if (newPartner->getType() == CreatureType::Bear) {
			bearPartner = (Bear*)newPartner; // Casting to Bear pointer to access protected members

			// Verification qu'il n'y a pas déjà de partenaires définis
			if (partner == nullptr && bearPartner->partner == nullptr) {
				partner = bearPartner;
				bearPartner->partner = this;
				bearPartner->switchState(new ReproductionState(bearPartner));
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
		new Bear("Bear", world, manager, position);
		// On empêche ensuite le partenaire de créer un autre enfant
		((Bear*)partner)->satiation -= 0.3f;
		satiation -= 0.3f;
		partner->switchState(new IdleState((Bear*)partner));
		switchState(new IdleState(this));
		partner->resetPartner();
	}

	virtual CreatureType* getType() {
		return CreatureType::Bear;
	}
};
