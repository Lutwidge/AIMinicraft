#pragma once

//#include <type_traits>
//#include <typeinfo>
#include "../AICreature.h"
#include "../owl/owl.h"


#define GRIFF_DIR_COUNT 4
#define GRIFF_SPEED 0.1f
#define GRIFF_SATIATION_DECAY 0.01f
#define GRIFF_DW_TIRENESS_THRESHOLD 0.5f
#define GRIFF_UP_TIRENESS_THRESHOLD 2.5f
#define GRIFF_TIRENESS_DECAY 0.1f
#define GRIFF_TIRENESS_BONUS 0.2f
#define GRIFF_REPRODUCTION_THRESHOLD 0.8f
#define GRIFF_SIGHT_RANGE 30
#define GRIFF_IDLE_HEIGHT 8
#define GRIFF_EAT_GAIN 0.1f

class Griffin : public AICreature
{
protected:
#pragma region States
	struct GriffinState : public State
	{
		Griffin* griffin;
		GriffinState(AICreature* creature) : State(creature), griffin((Griffin*)creature) {}
	};

	struct IdleState : public GriffinState
	{
		IdleState(Griffin* griffin) : GriffinState(griffin) {}

		virtual void enter()
		{
			//printf("%s : Idle \n", griffin->name.c_str());
			// On réinitialise la spirale
			griffin->initializeSpiralPath();
		}

		virtual void update(float elapsed) {

			// Check de si on est toujours en vie
			if (griffin->updateSatiation(elapsed) && griffin->updateTireness(elapsed))
			{
				// Reproduction prioritaire
				if (griffin->canReproduce()) {
					// Si oui, on check s'il y a une target compatible
					AICreature* targetGriffin = griffin->manager->perceptor->creatureSight(griffin, CreatureType::Griffin, GRIFF_SIGHT_RANGE);
					if (targetGriffin != nullptr) {
						// Si association réussie (pas de partenaire déjà défini pour les deux), on passe en reproduction pour les deux
						if (griffin->setPartner(targetGriffin)) {
							griffin->switchState(new ReproductionState(griffin));
							return;
						}
					}
				}

				if (griffin->tireness < GRIFF_DW_TIRENESS_THRESHOLD) {
					//printf("%s : Too tire, I have to go down \n", griffin->name.c_str());

					YVec3f dest = YVec3f(griffin->position.X, griffin->position.Y, griffin->world->getSurface(griffin->position.X, griffin->position.Y));
					griffin->goTo(dest);

					griffin->switchState(new GoingDownState(griffin));
					return;
				}

				// Sinon, on regarde si on voit un fruit
				AICreature *owl = griffin->manager->perceptor->creatureSight(griffin, CreatureType::Owl, GRIFF_SIGHT_RANGE);
				if (owl != nullptr) {
					//printf("%s : Owl in sight ! \n", griffin->name.c_str());
					griffin->setTarget(owl);

					griffin->switchState(new EatState(griffin));
					return;
				}

				// Sinon, mouvement en spirale
				if (griffin->hasNotReachedTarget()) {
					griffin->move(elapsed);
				}
				else
					griffin->incrementSpiralPath();
			}
		}

		virtual void exit() {}
	};

	struct EatState : public GriffinState
	{
		EatState(Griffin* griffin) : GriffinState(griffin) {}

		virtual void enter()
		{
			//printf("%s : I'm chasing that owl  \n", griffin->name.c_str());
			griffin->gotToEatTarget();
			griffin->chaseTime = 0;
		}

		virtual void update(float elapsed)
		{
			//Check if not dead
			if (griffin->updateSatiation(elapsed) && griffin->updateTireness(elapsed))
			{
				if (griffin->tireness < GRIFF_DW_TIRENESS_THRESHOLD) {
					//printf("%s : Too tire, I have to go down \n", griffin->name.c_str());

					YVec3f dest = YVec3f(griffin->position.X, griffin->position.Y, griffin->world->getSurface(griffin->position.X, griffin->position.Y));
					griffin->goTo(dest);

					griffin->switchState(new GoingDownState(griffin));
					return;
				}

				griffin->chaseTime += elapsed;
				if (griffin->chaseTime > 1) {
					//reset owl position. Else, chase the last position of the owl
					griffin->setEatTarget(griffin->targetCreature->position);
					griffin->chaseTime = 0;
					//printf("%s : Where are you ? %d size we are at %d\n", griffin->name.c_str(), griffin->pathToTarget.size(), griffin->currentMoveIndex);
				}


				//check if owl not too far away
				//float dist = (griffin->position.X - griffin->targetCreature->position.X) * (griffin->position.X - griffin->targetCreature->position.X);
				//dist += (griffin->position.Y - griffin->targetCreature->position.Y) * (griffin->position.Y - griffin->targetCreature->position.Y);

				/*if (dist * dist > GRIFF_SIGHT_RANGE * GRIFF_SIGHT_RANGE)
				{
					printf("%s : This owl was too quick !! \n", griffin->name.c_str());
					griffin->switchState(new IdleState(griffin));
					return;
				}
				//check if owl still alive
				else*/ if (griffin->isEatTargetValid())
				{
					if (griffin->hasNotReachedTarget()) {
						/*printf("%s : I'm reaching for this owl. I'm at %d, %d and is at %d, %d \n", griffin->name.c_str(),
							griffin->position.X, griffin->targetCreature->position.X,
							griffin->position.Y, griffin->targetCreature->position.Y);*/
						griffin->move(elapsed);
					}
					else {
						//printf("%s : Reach !\n", griffin->name.c_str());
						griffin->eat();
						griffin->switchState(new IdleState(griffin));
						return;
					}
				}
				// Sinon, retour à l'état idle
				else {
					//printf("%s : Owl perime \n", griffin->name.c_str());
					griffin->switchState(new IdleState(griffin));
					return;
				}
					
			}
		}

		virtual void exit() {}
	};

	struct GoingDownState : public GriffinState
	{
		GoingDownState(Griffin* griffin) : GriffinState(griffin) {}

		virtual void enter()
		{
			//printf("%s : GoingDown \n", griffin->name.c_str());

		}

		virtual void update(float elapsed) {

			// Check de si on est toujours en vie
			if (griffin->updateSatiation(elapsed)) {

				if (griffin->updateTireness(elapsed)) {
					if (griffin->hasNotReachedTargetInZ()) {
						griffin->move(elapsed);
					}
					else {
						//printf("%s : GoingDown to rest !    ", griffin->name.c_str());
						griffin->switchState(new RestState(griffin));
						return;
					}

				}
			}
		}

		virtual void exit() {}
	};

	struct RestState : public GriffinState
	{
		RestState(Griffin* griffin) : GriffinState(griffin) {}

		virtual void enter()
		{
			//printf("%s : Rest \n", griffin->name.c_str());
		}

		virtual void update(float elapsed) {

			// Check de si on est toujours en vie
			if (griffin->updateSatiation(elapsed)) {
				
				if (griffin->tireness > GRIFF_UP_TIRENESS_THRESHOLD) {
					//printf("%s : I'm back !  ", griffin->name.c_str());

					YVec3f dest = YVec3f(griffin->position.X, griffin->position.Y, griffin->position.Z + 12);
					griffin->goTo(dest);

					griffin->switchState(new GoingUpState(griffin));
					return;
				}
				else {
					griffin->tireness += GRIFF_TIRENESS_BONUS * elapsed;
				}
			}
		}

		virtual void exit() {}
	};

	struct GoingUpState : public GriffinState
	{
		GoingUpState(Griffin* griffin) : GriffinState(griffin) {}

		virtual void enter()
		{
			//printf("%s : Going Up ! \n", griffin->name.c_str());
		}

		virtual void update(float elapsed) {

			// Check de si on est toujours en vie
			if (griffin->updateSatiation(elapsed) /*&& griffin->updateTireness(elapsed)*/) {

				if (griffin->updateTireness(elapsed)) {
					if (griffin->hasNotReachedTargetInZ()) {
						griffin->move(elapsed);
					}
					else {
						//printf("%s : Up to idle   ", griffin->name.c_str());
						griffin->switchState(new IdleState(griffin));
						return;
					}

				}
			}
		}

		virtual void exit() {}
	};
	   
	struct ReproductionState : public GriffinState
	{
		ReproductionState(Griffin* griffin) : GriffinState(griffin) {}

		virtual void enter() {
			//printf("%s : Reproduction \n", griffin->name.c_str());
			// Définition point de rencontre valide avec le reprodTarget du Griffin
			YVec3f meetingPoint = (griffin->position + ((Griffin*)griffin->partner)->position) / 2;
			meetingPoint = YVec3f((int)meetingPoint.X, (int)meetingPoint.Y, (int)meetingPoint.Z);
			griffin->goTo(YVec3f(meetingPoint.X, meetingPoint.Y, griffin->world->getHighestPoint(meetingPoint.X, meetingPoint.Y)));
		}

		virtual void update(float elapsed)
		{
			// Mise à jour de la satiété et check de si on est toujours en vie
			if (griffin->updateSatiation(elapsed))
			{
				// Sinon si la target est toujours en reproduction, on bouge jusqu'à atteindre la cible
				if (griffin->isPartnerValid())
				{
					if (griffin->hasNotReachedTarget())
					{
						griffin->move(elapsed);
						return;
					}
					// Reproduction seulement si les deux sont arrivés
					else if (!griffin->partner->hasNotReachedTarget())
					{
						griffin->reproduce();
						return;
					}
				}
				else // Sinon retour à idle
				{
					griffin->resetPartner(); // On retire tout partenaire potentiel
					griffin->switchState(new IdleState(griffin));
					return;
				}
			}
		}

		virtual void exit() {}
	};
#pragma endregion

	int pathLength;
	YVec3f directions[GRIFF_DIR_COUNT] = { YVec3f(1, 0, 0), YVec3f(0, 1, 0), YVec3f(-1, 0, 0), YVec3f(0, -1, 0) };
	int curDirIndex;
	YVec3f realEatTarget;

	virtual void setSpiralPath(int length, int index) {
		YVec3f addedDir = directions[index] * length;
		YVec3f target = YVec3f(position.X + addedDir.X, position.Y + addedDir.Y, world->getHighestPoint(position.X + addedDir.X, position.Y + addedDir.Y) + GRIFF_IDLE_HEIGHT);

		goTo(target);
	}

	bool updateTireness(float elapsed) {
		tireness -= GRIFF_TIRENESS_DECAY * elapsed;
		timeBetweenMoves = GRIFF_SPEED * (1 / tireness);
		if (tireness <= 0.0f) {
			die();
			return false;
		}
		return true;
	}

public:
	Griffin(string name, MWorld *world, CreatureManager* cm, YVec3f pos) : AICreature(name, world, cm, pos, true, GRIFF_SPEED, GRIFF_SATIATION_DECAY, GRIFF_REPRODUCTION_THRESHOLD) {
		manager->registerCreature(this);
		tireness = 1.2f;
		chaseTime = 0;
		targetCreature = nullptr;
		switchState(new IdleState(this));
	}

	float tireness;
	float chaseTime;
	AICreature *targetCreature;

	virtual bool isEatTargetValid() {
		if (targetCreature != nullptr)
		{
			if (!targetCreature->IsDead)
				return true;
		}
		return false;
	}

	virtual bool setEatTarget(YVec3f target) {
		realEatTarget = target;
		// Define eat target as the nearest air cube near the fruit
		eatTarget = world->getNearestAirCube(target.X, target.Y, target.Z);
		goTo(eatTarget);
		if (eatTarget == realEatTarget)
			return false;
		else
			return true;
	}

	virtual void setTarget(AICreature* owl) {
		targetCreature = owl;
		setEatTarget(owl->position);
		return;
	}

	virtual void eat()
	{
		//printf("%s : I EAT YOU MowlER FuuhER \n", name.c_str());
		targetCreature->die();
		satiation += GRIFF_EAT_GAIN;
		if (satiation > 1.0f)
			satiation = 1.0f;
	}


	virtual bool hasNotReachedTarget() {
		bool hasReachIt = (abs((position - targetPos).X) < 0.1f && abs((position - targetPos).Y) < 0.1f) || pathToTarget.size() <= 0;
		return !hasReachIt;
	}

	bool hasNotReachedTargetInZ() {
		bool hasReachIt = (abs((position - targetPos).X) < 0.1f && abs((position - targetPos).Y) < 0.1f && abs((position - targetPos).Z) < 0.1f) || pathToTarget.size() <= 0;
		return !hasReachIt;
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
		curDirIndex = curDirIndex % GRIFF_DIR_COUNT;
		setSpiralPath(pathLength, curDirIndex);
	}

	/* REPRODUCTION */
	virtual bool setPartner(AICreature* newPartner)
	{
		Griffin* griffinPartner = nullptr;
		if (newPartner->getType() == CreatureType::Griffin) {
			griffinPartner = (Griffin*)newPartner; // Casting to Griffin pointer to access protected members

			// Verification qu'il n'y a pas déjà de partenaires définis
			if (partner == nullptr && griffinPartner->partner == nullptr) {
				partner = griffinPartner;
				griffinPartner->partner = this;
				griffinPartner->switchState(new ReproductionState(griffinPartner));
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
		new Griffin("Griffin", world, manager, position);
		// On empêche ensuite le partenaire de créer un autre enfant
		((Griffin*)partner)->satiation -= 0.3f;
		satiation -= 0.3f;
		partner->switchState(new IdleState((Griffin*)partner));
		switchState(new IdleState(this));
		partner->resetPartner();
	}

	virtual CreatureType* getType() {
		return CreatureType::Griffin;
	}
};