#pragma once

#include <type_traits>
#include <typeinfo>

#include <creatures/AICreature.h>

namespace
{
	static constexpr auto SNAKE_SIGHT_RANGE = 5;
	static constexpr auto SNAKE_DIR_COUNT = 4;
	static constexpr auto SNAKE_SPEED = 0.2f;
	static constexpr auto SNAKE_SATIATION_DECAY = 0.01f;
	static constexpr auto SNAKE_REPRODUCTION_THRESHOLD = 0.8f;
	static constexpr auto SNAKE_EAT_GAIN = 0.4f;
	static constexpr auto SNAKE_MOVEMENT_RANGE = 8;
	static constexpr auto SNAKE_FLEE_DISTANCE = 6;
}

class Snake : public AICreature
{
protected:
	struct SnakeState : public State
	{
		Snake* snake;
		SnakeState(AICreature* creature) : State(creature), snake(static_cast<Snake*>(creature)) {}
	};

	struct IdleState : public SnakeState
	{
		IdleState(Snake* snake) : SnakeState(snake) {}

		virtual void enter()
		{
			snake->ground();
			snake->initializePath();
		}

		virtual void update(float elapsed) {

			// Mise à jour de la satiété et check de si on est toujours en vie
			if (snake->updateSatiation(elapsed)) {
				// Si on voit un prédateur, on fuit
				snake->predator = snake->manager->perceptor->creatureSight(snake, CreatureType::Owl, SNAKE_SIGHT_RANGE);
				if (snake->predator != nullptr) {
					snake->switchState(new FleeState(snake));
					return;
				}
				else {
					// Reproduction prioritaire
					if (snake->canReproduce()) {
						// Si oui, on check s'il y a une target compatible
						AICreature* targetSnake = snake->manager->perceptor->creatureSight(snake, CreatureType::Snake, SNAKE_SIGHT_RANGE);
						if (targetSnake != nullptr) {
							// Si association réussie (pas de partenaire déjà défini pour les deux), on passe en reproduction pour les deux
							if (snake->setPartner(targetSnake)) {
								snake->switchState(new ReproductionState(snake));
								return;
							}
						}
					}

					if (snake->manager->perceptor->creatureSight(snake, CreatureType::Rat, 15) != nullptr)
					{
						snake->setEatTarget(snake->manager->perceptor->creatureSight(snake, CreatureType::Rat, 15));

						if (snake->isEatTargetValid())
						{
							snake->switchState(new EatState(snake));
							return;
						}
					}
				}

				if (snake->hasNotReachedTarget()) snake->move(elapsed);
				else snake->initializePath();
			}
		}

		virtual void exit() {}
	};

	struct EatState : public SnakeState
	{
		EatState(Snake* snake) : SnakeState(snake) {}

		virtual void enter()
		{
			//printf("%s : Eat \n", snake->name.c_str());
			snake->gotToEatTarget();
		}

		virtual void update(float elapsed)
		{
			// Mise à jour de la satiété et check de si on est toujours en vie
			if (snake->updateSatiation(elapsed))
			{
				// Si on voit un prédateur, on fuit
				snake->predator = snake->manager->perceptor->creatureSight(snake, CreatureType::Owl, SNAKE_SIGHT_RANGE);
				if (snake->predator != nullptr) {
					snake->switchState(new FleeState(snake));
					return;
				}
				else {
					// Sinon si la proie est valide, on continue vers lui jusqu'à l'atteindre
					if (snake->isEatTargetValid())
					{
						if (snake->hasNotReachedTarget())
							snake->move(elapsed);
						else {
							snake->eat();
							return;
						}
					}
					// Sinon, retour à l'état idle
					else {
						snake->switchState(new IdleState(snake));
						return;
					}
				}
			}
		}

		virtual void exit() {}
	};

	struct FleeState : public SnakeState
	{
		FleeState(Snake* snake) : SnakeState(snake) {}

		virtual void enter()
		{
			// Definir target de fuite
			YVec3f fleeTarget = snake->position + (snake->position - snake->predator->position).normalize() * SNAKE_FLEE_DISTANCE;
			// On s'assure que la target de fuite est valide
			fleeTarget = snake->world->getNearestAirCube(fleeTarget.X, fleeTarget.Y, fleeTarget.Z);
			snake->goTo(fleeTarget);
		}

		virtual void update(float elapsed) 
		{
			// Mise à jour de la satiété et check de si on est toujours en vie
			if (snake->updateSatiation(elapsed))
			{
				// On avance jusqu'à fuir à la target définie
				if (snake->hasNotReachedTarget())
					snake->move(elapsed);
				else
					snake->switchState(new IdleState(snake));
			}
		}

		virtual void exit() {}
	};

	struct ReproductionState : public SnakeState
	{
		ReproductionState(Snake* snake) : SnakeState(snake) {}

		virtual void enter() {
			//printf("%s : Reproduction \n", snake->name.c_str());
			// Définition point de rencontre valide avec le reprodTarget du Rat
			YVec3f meetingPoint = (snake->position + ((Snake*)snake->partner)->position) / 2;
			meetingPoint = YVec3f((int)meetingPoint.X, (int)meetingPoint.Y, (int)meetingPoint.Z);
			snake->goTo(YVec3f(meetingPoint.X, meetingPoint.Y, snake->world->getHighestPoint(meetingPoint.X, meetingPoint.Y)));
		}

		virtual void update(float elapsed)
		{
			// Mise à jour de la satiété et check de si on est toujours en vie
			if (snake->updateSatiation(elapsed))
			{
				// Si on voit un prédateur, on fuit
				snake->predator = snake->manager->perceptor->creatureSight(snake, CreatureType::Owl, SNAKE_SIGHT_RANGE);
				if (snake->predator != nullptr)
				{
					snake->resetPartner(); // On retire tout partenaire potentiel
					snake->switchState(new FleeState(snake));
					return;
				}
				else
				{
					// Sinon si la target est toujours en reproduction, on bouge jusqu'à atteindre la cible
					if (snake->isPartnerValid())
					{
						if (snake->hasNotReachedTarget())
						{
							snake->move(elapsed);
							return;
						}
						// Reproduction seulement si les deux sont arrivés
						else if (!snake->partner->hasNotReachedTarget())
						{
							snake->reproduce();
							return;
						}
					}
					else // Sinon retour à idle
					{
						snake->resetPartner(); // On retire tout partenaire potentiel
						snake->switchState(new IdleState(snake));
						return;
					}
				}
			}
		}

		virtual void exit() {}
	};
#pragma endregion

	int pathLength;
	YVec3f directions[SNAKE_DIR_COUNT] = { YVec3f(1, 0, 0), YVec3f(-1, 0, 0), YVec3f(0, -1, 0), YVec3f(0, 1, 0) };
	int curDirIndex;
	YVec3f realEatTarget;

public:
	Snake(string name, MWorld *world, CreatureManager* cm, YVec3f pos) : AICreature(name, world, cm, pos, false, SNAKE_SPEED, SNAKE_SATIATION_DECAY, SNAKE_REPRODUCTION_THRESHOLD) {
		manager->registerCreature(this);
		switchState(new IdleState(this));
	}

	/* EATING */
	AICreature* preyCreature;

	virtual void setEatTarget(AICreature* creature)
	{
		preyCreature = creature;
	}

	virtual bool isEatTargetValid() {
		if (preyCreature != nullptr)
		{
			if (!preyCreature->IsDead)
				return true;
		}
		return false;
	}

	virtual void eat()
	{
		preyCreature->die();
		satiation += SNAKE_EAT_GAIN;
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
		int x = normalX() * SNAKE_MOVEMENT_RANGE;
		int y = normalY() * SNAKE_MOVEMENT_RANGE;

		x += position.X;
		y += position.Y;

		YVec3f target = YVec3f(x, y, world->getSurface(x, y));
		goTo(target);
	}

	/* REPRODUCTION */
	virtual bool setPartner(AICreature* newPartner)
	{
		Snake* snakePartner = nullptr;
		if (newPartner->getType() == CreatureType::Snake) {
			snakePartner = (Snake*)newPartner; // Casting to Rat pointer to access protected members

			// Verification qu'il n'y a pas déjà de partenaires définis
			if (partner == nullptr && snakePartner->partner == nullptr) {
				partner = snakePartner;
				snakePartner->partner = this;
				snakePartner->switchState(new ReproductionState(snakePartner));
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
		new Snake("Snake", world, manager, position);
		// On empêche ensuite le partenaire de créer un autre enfant
		((Snake*)partner)->satiation -= 0.3f;
		satiation -= 0.3f;
		partner->switchState(new IdleState((Snake*)partner));
		switchState(new IdleState(this));
		partner->resetPartner();
	}

	virtual CreatureType* getType() {
		return CreatureType::Snake;
	}
};