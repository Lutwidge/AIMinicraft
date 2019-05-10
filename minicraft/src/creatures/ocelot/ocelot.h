#pragma once

#include <type_traits>
#include <typeinfo>
#include "../AICreature.h"

#define DIR_COUNT 4
#define OCELOT_SPEED 0.08f
#define OCELOT_SATIATION_DECAY 0.01f
#define OCELOT_REPRODUCTION_THRESHOLD 0.8f
#define OCELOT_SIGHT_RANGE 15
#define OCELOT_MOVEMENT_RANGE 8
#define OCELOT_EAT_GAIN 0.5f

class Ocelot : public AICreature {
protected:
#pragma region States
	struct OcelotState : public State
	{
		Ocelot * ocelot;
		OcelotState(AICreature* creature) : State(creature), ocelot((Ocelot*)creature) {}
	};

	struct IdleState : public OcelotState {
		IdleState(Ocelot * ocelot) : OcelotState(ocelot) {

		}

		virtual void enter() {
			ocelot->ground();
			ocelot->initializePath();
		}

		virtual void update(float elapsed) {
			if (ocelot->updateSatiation(elapsed)) {

				ocelot->setEatTarget(ocelot->manager->perceptor->creatureSight(ocelot, CreatureType::Bird, OCELOT_SIGHT_RANGE));
				if (ocelot->isEatTargetValid()) {
					YLog::log(YLog::USER_INFO, toString("[OCELOT] Found prey !").c_str());
					ocelot->switchState(new EatState(ocelot));
				}

				ocelot->ground();
				if (ocelot->hasNotReachedTarget()) {
					ocelot->move(elapsed);
				}
				else {
					ocelot->initializePath();
				}
			}
		}

		virtual void exit() {

		}
	};

	struct EatState : public OcelotState
	{
		EatState(Ocelot * ocelot) : OcelotState(ocelot) {

		}

		virtual void enter()
		{
			ocelot->eatTarget = ocelot->prey->position;
			ocelot->gotToEatTarget();
		}

		virtual void update(float elapsed) {
			if (ocelot->updateSatiation(elapsed)) {
				ocelot->move(elapsed);
			}
		}

		virtual void exit() {
			ocelot->lastPrey = ocelot->prey;
			ocelot->prey = nullptr;
		}
	};

	struct ReproductionState : public OcelotState {
		ReproductionState(Ocelot* ocelot) : OcelotState(ocelot) {

		}

		virtual void enter() {
			// MeetingPoint pour les ocelots au sol 
			YVec3f meetingPoint = (ocelot->position + ((Ocelot*)ocelot->partner)->position) / 2;
			ocelot->goTo(YVec3f(meetingPoint.X, meetingPoint.Y, ocelot->world->getSurface(meetingPoint.X, meetingPoint.Y)));
		}

		virtual void update(float elapsed) {
			if (ocelot->updateSatiation(elapsed)) {

				if (ocelot->isPartnerValid()) {
					if (ocelot->hasNotReachedTarget()) {
						ocelot->move(elapsed);
					}
					//Si les ocelots sont arrivés au MeetingPoint
					else if (!ocelot->hasNotReachedTarget() && !ocelot->partner->hasNotReachedTarget()) {
						ocelot->reproduce();
						ocelot->resetPartner();
						ocelot->switchState(new IdleState(ocelot));
					}
				}
				else {
					ocelot->resetPartner();
					ocelot->switchState(new IdleState(ocelot));
				}
			}
		}

		virtual void exit() {

		}
	};

#pragma endregion

	AICreature * prey;
	AICreature * lastPrey;

	virtual void setRandomPath() {
		int x = rand() % world->MAT_SIZE_METERS;
		int y = rand() % world->MAT_SIZE_METERS;
		YVec3f target = YVec3f(x, y, world->getSurface(x, y));
		goTo(target);
	}

	float normalX() {
		float u = (rand() % 100) * 0.01;
		float v = (rand() % 100) * 0.01;

		float x = sqrt(-2 * log(u)) * cos(2 * 3.141592 * v);

		return x;
	}

	float normalY() {
		float u = (rand() % 100) * 0.01;
		float v = (rand() % 100) * 0.01;

		float y = sqrt(-2 * log(u)) * sin(2 * 3.141592 * v);

		return y;
	}

	// Random walk
	virtual void setPathArroundLastPrey() {
		int x = normalX() * OCELOT_MOVEMENT_RANGE;
		int y = normalY() * OCELOT_MOVEMENT_RANGE;
		if (lastPrey != nullptr) {
			// turn arround last prey
			x += lastPrey->position.X;
			y += lastPrey->position.Y;
		}
		else {
			// turn arround myself
			x += position.X;
			y += position.Y;
		}
		YVec3f target = YVec3f(x, y, world->getSurface(x, y));
		goTo(target);
	}

public:

	Ocelot(string name, MWorld * world, CreatureManager * cm, YVec3f pos) : AICreature(name, world, cm, pos, false, OCELOT_SPEED, OCELOT_SATIATION_DECAY, OCELOT_REPRODUCTION_THRESHOLD) {
		manager->registerCreature(this);
		switchState(new IdleState(this));
	}

	/* MOVEMENT */
	virtual void ground() {
		position.Z = world->getSurface(position.X, position.Y);
	}

	/* EATING */
	virtual bool isEatTargetValid() {
		return prey != nullptr;
	}

	virtual void setEatTarget(AICreature * creature) {
		prey = creature;
	}

	virtual void eat() {
		manager->unregisterCreature(prey);
		satiation += OCELOT_EAT_GAIN;
	}

	virtual bool setPartner(AICreature* newPartner) {
		Ocelot * ocelotPartner = nullptr;
		if (typeid(newPartner) == typeid(Ocelot*)) {
			ocelotPartner = (Ocelot*)newPartner; // Casting to Ocelot* to access protected members
		}
		// If both ocelots don't already have a partner
		if (partner == nullptr && ocelotPartner->partner == nullptr) {
			partner = ocelotPartner;
			ocelotPartner->partner = this;
			ocelotPartner->switchState(new ReproductionState(ocelotPartner));
			return true;
		}
		return false;
	}

	virtual void initializeRandomPath() {
		setRandomPath();
	}

	virtual void initializePath() {
		setPathArroundLastPrey();
	}



	virtual bool isPartnerValid() {
		return false;
	}

	virtual void reproduce() {
		// Création d'un ocelot qui s'enregistre lui-même auprès du CreatureManager
		new Ocelot("Ocelot", world, manager, position);

		satiation -= 0.3f;

		partner->resetPartner();
		partner->switchState(new IdleState((Ocelot *)partner)); // --> new IdleState(this) ?
	}

	virtual CreatureType getType() {
		return CreatureType::Ocelot;
	}
};