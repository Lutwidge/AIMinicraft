#pragma once

#include <type_traits>
#include <typeinfo>

#include <creatures/AICreature.h>

namespace
{
    static constexpr auto RAT_SIGHT_RANGE = 5;
    static constexpr auto RAT_DIR_COUNT = 4;
    static constexpr auto RAT_SPEED = 2.0f;
    static constexpr auto RAT_SATIATION_DECAY = 0.01f;
    static constexpr auto RAT_REPRODUCTION_THRESHOLD = 1.75f;
    static constexpr auto RAT_EAT_GAIN = 0.4f;
}

class Rat : public AICreature
{
protected:
	struct RatState : public State
	{
		Rat* rat;
		RatState(AICreature* creature) : State(creature), rat( static_cast<Rat*>( creature ) ) {}
	};

	struct IdleState : public RatState
	{
		IdleState(Rat* rat) : RatState(rat) {}

		virtual void enter()
		{
			//printf("%s : Idle \n", rat->name.c_str());
			// On r�initialise la spirale
			rat->initializeSpiralPath();
		}

		virtual void update(float elapsed) {

			// Mise � jour de la sati�t� et check de si on est toujours en vie
			if (rat->updateSatiation(elapsed)) {
				// Si on voit un pr�dateur, on fuit
				if (rat->manager->perceptor->creatureSight(rat, CreatureType::Snake, RAT_SIGHT_RANGE ) != nullptr) {
					rat->switchState(new FleeState(rat));
					return;
				}
				else {
					// Reproduction prioritaire
					if (rat->canReproduce()) {
						// Si oui, on check s'il y a une target compatible
						AICreature* targetBird = rat->manager->perceptor->creatureSight(rat, CreatureType::Rat, RAT_SIGHT_RANGE);
						if (targetBird != nullptr) {
							// Si association r�ussie (pas de partenaire d�j� d�fini pour les deux), on passe en reproduction pour les deux
							if (rat->setPartner(targetBird)) {
								rat->switchState(new ReproductionState(rat));
								return;
							}
						}
					}

					// Sinon, on regarde si on voit un fruit
					YVec3f fruit;
					if (rat->manager->perceptor->blockSight(rat, MCube::CUBE_FRUIT, RAT_SIGHT_RANGE, fruit)) {
						if (rat->setEatTarget(fruit)) {
							rat->switchState(new EatState(rat));
							return;
						}
					}
				}

				// Sinon, mouvement en spirale
				if (rat->hasNotReachedTarget())
					rat->move(elapsed);
				else
					rat->incrementSpiralPath();
			}
		}

		virtual void exit() {}
	};

	struct EatState : public RatState
	{
		EatState(Rat* rat) : RatState(rat) {}

		virtual void enter()
		{
			//printf("%s : Eat \n", rat->name.c_str());
			rat->gotToEatTarget();
		}

		virtual void update(float elapsed)
		{
			// Mise � jour de la sati�t� et check de si on est toujours en vie
			if (rat->updateSatiation(elapsed))
			{
				// Si on voit un pr�dateur, on fuit
				if (rat->manager->perceptor->creatureSight(rat, CreatureType::Ocelot, RAT_SIGHT_RANGE) != nullptr) {
					rat->switchState(new FleeState(rat));
					return;
				}
				else {
					// Sinon si le fruit est toujours l�, on continue vers lui jusqu'� l'atteindre
					if (rat->isEatTargetValid()) 
					{
						if (rat->hasNotReachedTarget())
							rat->move(elapsed);
						else {
							rat->eat();
							return;
						}
					} 
					// Sinon, retour � l'�tat idle
					else {
						rat->switchState(new IdleState(rat));
						return;
					}
				}
			}
		}

		virtual void exit() {}
	};

	struct FleeState : public RatState
	{
		FleeState(Rat* rat) : RatState(rat) {}

		virtual void enter()
		{

		}

		virtual void update(float elapsed) {}

		virtual void exit() {}
	};

	struct ReproductionState : public RatState
	{
		ReproductionState(Rat* rat) : RatState(rat) {}

		virtual void enter() {
			//printf("%s : Reproduction \n", rat->name.c_str());
			// D�finition point de rencontre valide avec le reprodTarget du Rat
			YVec3f meetingPoint = (rat->position + ((Rat*) rat->partner)->position) / 2;
			meetingPoint = YVec3f((int) meetingPoint.X, (int) meetingPoint.Y, (int) meetingPoint.Z);
			rat->goTo(YVec3f(meetingPoint.X, meetingPoint.Y, rat->world->getHighestPoint(meetingPoint.X, meetingPoint.Y)));
		}

		virtual void update(float elapsed)
		{
			// Mise � jour de la sati�t� et check de si on est toujours en vie
			if (rat->updateSatiation(elapsed))
			{
				// Si on voit un pr�dateur, on fuit
				if (rat->manager->perceptor->creatureSight(rat, CreatureType::Ocelot, RAT_SIGHT_RANGE) != nullptr)
				{
					rat->resetPartner(); // On retire tout partenaire potentiel
					rat->switchState(new FleeState(rat));
					return;
				}
				else
				{
					// Sinon si la target est toujours en reproduction, on bouge jusqu'� atteindre la cible
					if (rat->isPartnerValid())
					{
						if (rat->hasNotReachedTarget())
						{
							rat->move(elapsed);
							return;
						}
						// Reproduction seulement si les deux sont arriv�s
						else if (!rat->partner->hasNotReachedTarget())
						{
							rat->reproduce();
							return;
						}
					} 
					else // Sinon retour � idle
					{
						rat->resetPartner(); // On retire tout partenaire potentiel
						rat->switchState(new IdleState(rat));
						return;
					}
				}
			}
		}

		virtual void exit() {}
	};
	#pragma endregion

	int pathLength;
	YVec3f directions[RAT_DIR_COUNT] = { YVec3f(1, 0, 0), YVec3f(-1, 0, 0), YVec3f(0, -1, 0), YVec3f(0, 1, 0) };
	int curDirIndex;
	YVec3f realEatTarget;

	virtual void setSpiralPath(int length, int index) {
		YVec3f addedDir = directions[index] * length;
		YVec3f target = YVec3f(position.X + addedDir.X, position.Y + addedDir.Y, world->getHighestPoint(position.X + addedDir.X, position.Y + addedDir.Y));

		goTo(target);
	}

public:
	Rat(string name, MWorld *world, CreatureManager* cm, YVec3f pos) : AICreature(name, world, cm, pos, true, RAT_SPEED, RAT_SATIATION_DECAY, RAT_REPRODUCTION_THRESHOLD) {
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
		// Reg�n�rer le monde (mais co�teux... comme le picking)
		world->updateCube((int) realEatTarget.X, (int) realEatTarget.Y, (int) realEatTarget.Z);
		satiation += RAT_EAT_GAIN;
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
		curDirIndex = curDirIndex % RAT_DIR_COUNT;
		setSpiralPath(pathLength, curDirIndex);
	}

	/* REPRODUCTION */
	virtual bool setPartner(AICreature* newPartner)
	{
		Rat* birdPartner = nullptr;
		if (newPartner->getType() == CreatureType::Rat) {
			birdPartner = (Rat*) newPartner; // Casting to Rat pointer to access protected members

			// Verification qu'il n'y a pas d�j� de partenaires d�finis
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
		// On cr�e une nouvelle instance, elle se register elle-m�me aupr�s du CreatureManager dans son constructeur
		new Rat("Rat", world, manager, position);
		// On emp�che ensuite le partenaire de cr�er un autre enfant
		((Rat*) partner)->satiation -= 0.3f;
		satiation -= 0.3f;
		partner->switchState(new IdleState((Rat*) partner));
		switchState(new IdleState(this));
		partner->resetPartner();
	}

	virtual CreatureType* getType() {
		return CreatureType::Rat;
	}
};