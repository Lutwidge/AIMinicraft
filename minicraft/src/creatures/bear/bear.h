#pragma once

#include <creatures/AICreature.h>

namespace {
	static constexpr auto BEAR_SIGHT_RANGE = 5;
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
			printf("%s : Idle \n", bear->name.c_str());
			// On réinitialise la chemin
			bear->waypoint = getNewWaypoint();
		}

		virtual void update(float elapsed) {

			// Mise à jour de la satiété et check de si on est toujours en vie
			if (bear->updateSatiation(elapsed)) {
				// Reproduction prioritaire
				if (bear->canReproduce()) {
					// Si oui, on check s'il y a une target compatible
					AICreature* targetBear = bear->manager->perceptor->creatureSight(bear, CreatureType::Bear, BEAR_SIGHT_RANGE);
					if (targetBear != nullptr) {
						// Si association réussie (pas de partenaire déjà défini pour les deux), on passe en reproduction pour les deux
						if (bear->setPartner(targetBear)) {
							bear->switchState(new ReproductionState(bear));
							return;
						}
					}
				}

				// Sinon, on regarde si on voit un ocelot
				YVec3f fruit;
				AICreature* ocelot = bear->manager->perceptor->creatureSight(bear, CreatureType::Ocelot, BEAR_SIGHT_RANGE);
				if (ocelot != nullptr) {
					if (bear->setEatTarget(ocelot->position)) {
						bear->switchState(new EatState(bear));
						return;
					}
				}

				// Sinon, mouvement en spirale
				if (bear->hasNotReachedTarget())
					bear->move(elapsed);
				else
					bear->waypoint = getNewWaypoint();
			}
		}

		virtual void exit() {}

	private:
		YVec3f getNewWaypoint() {
			return YVec3f(0, 0, 0);
		}
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

		virtual void exit() {}
	};
#pragma endregion

	YVec3f waypoint;

public:
	Bear(string name, MWorld *world, CreatureManager* cm, YVec3f pos) : AICreature(name, world, cm, pos, true, BEAR_SPEED, BEAR_SATIATION_DECAY, BEAR_REPRODUCTION_THRESHOLD) {
		manager->registerCreature(this);
		switchState(new IdleState(this));
	}

	/* EATING */
	virtual bool isEatTargetValid() {
		return true; // TODO implement
	}

	virtual void eat() {
		satiation += BEAR_EAT_GAIN;
		if (satiation > 1.0f)
			satiation = 1.0f;
	}

	virtual bool setEatTarget(YVec3f target) {
		eatTarget = target;
		return true;
	}

	/* REPRODUCTION */
	virtual bool setPartner(AICreature* newPartner) {
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

	virtual bool isPartnerValid() {
		ReproductionState* partnerReprodState = dynamic_cast<ReproductionState*>(partner->state);
		if (partnerReprodState != nullptr) {
			return true;
		}
		return false;
	}

	virtual void reproduce() {
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
