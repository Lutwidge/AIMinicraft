#pragma once

#include <type_traits>
#include <typeinfo>

#include <creatures/AICreature.h>

namespace
{
	static constexpr auto COPYCAT_SIGHT_RANGE = 30;
	static constexpr auto COPYCAT_DIR_COUNT = 4;
	static constexpr auto COPYCAT_SPEED = 0.2f;
	static constexpr auto COPYCAT_SATIATION_DECAY = 0.01f;
	static constexpr auto COPYCAT_REPRODUCTION_THRESHOLD = 0.9f;
	static constexpr auto COPYCAT_EAT_GAIN = 0.4f;
	static constexpr auto COPYCAT_MOVEMENT_RANGE = 12;
}

class Copycat : public AICreature
{
protected:
	struct CopycatState : public State
	{
		Copycat* copycat;
		CopycatState(AICreature* creature) : State(creature), copycat(static_cast<Copycat*>(creature)) {}
	};

	struct IdleState : public CopycatState
	{
		IdleState(Copycat* copycat) : CopycatState(copycat) {}

		virtual void enter()
		{
			//printf("%s : Idle \n", copycat->name.c_str());
			copycat->ground();
			copycat->initializePath();
		}

		virtual void update(float elapsed) {

			// Mise à jour de la satiété et check de si on est toujours en vie
			if (copycat->updateSatiation(elapsed)) {
				// Reproduction prioritaire
				if (copycat->canReproduce()) {
					// Si oui, on check s'il y a une target compatible
					AICreature* targetCopycat = copycat->manager->perceptor->creatureSight(copycat, CreatureType::Copycat, COPYCAT_SIGHT_RANGE);
					if (targetCopycat != nullptr) {
						// Si association réussie (pas de partenaire déjà défini pour les deux), on passe en reproduction pour les deux
						if (copycat->setPartner(targetCopycat)) {
							copycat->switchState(new ReproductionState(copycat));
							return;
						}
					}
				}

				AICreature *eatTarget = copycat->manager->perceptor->deadCreatureSight(copycat, COPYCAT_SIGHT_RANGE);
				if (eatTarget != nullptr)
				{
					copycat->setEatTarget(eatTarget);

					if (copycat->isEatTargetValid())
					{
						eatTarget->CadaverBeingTargetted = true;
						copycat->switchState(new EatState(copycat));
						return;
					}
				}

				if (copycat->hasNotReachedTarget()) copycat->move(elapsed);
				else copycat->initializePath();
			}
		}

		virtual void exit() {}
	};

	struct EatState : public CopycatState
	{
		EatState(Copycat* copycat) : CopycatState(copycat) {}

		virtual void enter()
		{
			//printf("%s : Eat \n", copycat->name.c_str());
			copycat->gotToEatTarget();
		}

		virtual void update(float elapsed)
		{
			// Mise à jour de la satiété et check de si on est toujours en vie
			if (copycat->updateSatiation(elapsed))
			{
				if (copycat->isEatTargetValid())
				{
					if (copycat->hasNotReachedTarget())
						copycat->move(elapsed);
					else {
						copycat->eat();
						return;
					}
				}
				// Sinon, retour à l'état idle
				else {
					copycat->switchState(new IdleState(copycat));
					return;
				}
			}
		}

		virtual void exit() {}
	};

	struct ReproductionState : public CopycatState
	{
		ReproductionState(Copycat* copycat) : CopycatState(copycat) {}

		virtual void enter() {
			//printf("%s : Reproduction \n", copycat->name.c_str());
			// Définition point de rencontre valide avec le reprodTarget du Rat
			YVec3f meetingPoint = (copycat->position + ((Copycat*)copycat->partner)->position) / 2;
			meetingPoint = YVec3f((int)meetingPoint.X, (int)meetingPoint.Y, (int)meetingPoint.Z);
			copycat->goTo(YVec3f(meetingPoint.X, meetingPoint.Y, copycat->world->getHighestPoint(meetingPoint.X, meetingPoint.Y)));
		}

		virtual void update(float elapsed)
		{
			// Mise à jour de la satiété et check de si on est toujours en vie
			if (copycat->updateSatiation(elapsed))
			{
				// Sinon si la target est toujours en reproduction, on bouge jusqu'à atteindre la cible
				if (copycat->isPartnerValid())
				{
					if (copycat->hasNotReachedTarget())
					{
						copycat->move(elapsed);
						return;
					}
					// Reproduction seulement si les deux sont arrivés
					else if (!copycat->partner->hasNotReachedTarget())
					{
						copycat->reproduce();
						return;
					}
				}
				else // Sinon retour à idle
				{
					copycat->resetPartner(); // On retire tout partenaire potentiel
					copycat->switchState(new IdleState(copycat));
					return;
				}
			}
		}

		virtual void exit() {}
	};
#pragma endregion

	int pathLength;
	YVec3f directions[COPYCAT_DIR_COUNT] = { YVec3f(1, 0, 0), YVec3f(-1, 0, 0), YVec3f(0, -1, 0), YVec3f(0, 1, 0) };
	int curDirIndex;
	YVec3f realEatTarget;

public:
	Copycat(string name, MWorld *world, CreatureManager* cm, YVec3f pos) : AICreature(name, world, cm, pos, false, COPYCAT_SPEED, COPYCAT_SATIATION_DECAY, COPYCAT_REPRODUCTION_THRESHOLD) {
		manager->registerCreature(this);
		switchState(new IdleState(this));
	}

	/* EATING */
	AICreature* preyCreature;

	virtual void setEatTarget(AICreature* creature)
	{
		preyCreature = creature;
	}

	virtual bool isEatTargetValid()
	{
		return preyCreature != nullptr;
	}

	virtual void eat()
	{
		delete preyCreature;
		satiation += COPYCAT_EAT_GAIN;
		preyCreature = nullptr;
	}

	/* IDLE */

	virtual void ground()
	{
		position.Z = world->getSurface(position.X, position.Y);
	}

	float normalX()
	{
		float u = (rand() % 100) * 0.01;
		float v = (rand() % 100) * 0.01;

		float x = sqrt(-2 * log(u)) * cos(2 * 3.141592 * v);

		return x;
	}

	float normalY()
	{
		float u = (rand() % 100) * 0.01;
		float v = (rand() % 100) * 0.01;

		float y = sqrt(-2 * log(u)) * sin(2 * 3.141592 * v);

		return y;
	}

	virtual void initializePath()
	{
		int x = normalX() * COPYCAT_MOVEMENT_RANGE;
		int y = normalY() * COPYCAT_MOVEMENT_RANGE;

		x += position.X;
		y += position.Y;

		YVec3f target = YVec3f(x, y, world->getSurface(x, y));
		goTo(target);
	}

	/* REPRODUCTION */
	virtual bool setPartner(AICreature* newPartner)
	{
		Copycat* copycatPartner = nullptr;
		if (newPartner->getType() == CreatureType::Copycat) {
			copycatPartner = (Copycat*)newPartner; // Casting to Rat pointer to access protected members

			// Verification qu'il n'y a pas déjà de partenaires définis
			if (partner == nullptr && copycatPartner->partner == nullptr) {
				partner = copycatPartner;
				copycatPartner->partner = this;
				copycatPartner->switchState(new ReproductionState(copycatPartner));
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
		new Copycat("Copycat", world, manager, position);
		// On empêche ensuite le partenaire de créer un autre enfant
		((Copycat*)partner)->satiation -= 0.3f;
		satiation -= 0.3f;
		partner->switchState(new IdleState((Copycat*)partner));
		switchState(new IdleState(this));
		partner->resetPartner();
	}

	virtual CreatureType* getType() {
		return CreatureType::Copycat;
	}
};