#pragma once

#include "../AICreature.h"
#include <typeinfo>
#include <type_traits>

#define SPEED 4
#define DECAY_SATIETY 0.02
#define REPRO_THRESHOLD 0.8
#define OWL_SIGHT_RANGE 15

class Snake;

class Owl : AICreature
{
public:
#pragma region States
	struct OwlState : State
	{
		Owl* owl;
		OwlState(Owl* owl) : State(creature), owl(owl) {}

		void enter()
		{

		}

		void update(float elapsed)
		{

		}

		void exit()
		{

		}
	};

	//Quitte l'idle quand il a reperé un serpent ou quand il meurt de fin
	struct IdleState : public OwlState
	{
		IdleState(Owl* owl, bool onBranch) : OwlState(owl) 
		{
			owl->isOnBranch = onBranch;
		}
		float timeBeforeSearch;
		float time;

		void enter()
		{
			YLog::log(YLog::MSG_TYPE::ENGINE_INFO, "owl rentre dans idle");
			timeBeforeSearch = 2;
			time = timeBeforeSearch;
		}

		void update(float elapsed)
		{
			time -= elapsed;
			if (owl->updateSatiation(elapsed))
			{
				if (owl->getSatiation() < 0.2f)
				{
					owl->switchState(new LookingForFoodState(owl));
					return;
				}

				if (time < 0)
				{
					time = timeBeforeSearch;

					AICreature* crea = owl->manager->perceptor->creatureSight(owl, CreatureType::Snake, OWL_SIGHT_RANGE);
					if (crea != nullptr)
					{
						owl->target = crea;
						owl->switchState(new ChaseState(owl));
					}					
				}				
			}
		}

		void exit()
		{
			owl->isOnBranch = false;
		}
	};

	struct LookingForFoodState : public OwlState
	{

		LookingForFoodState(Owl* owl) : OwlState(owl) {}
		YVec3f positionTarget;
		float speed = 2.0;

		void enter()
		{
			YLog::log(YLog::MSG_TYPE::ENGINE_INFO, "owl rentre LFF");
			//conter anti boucle infinie
			int counter = 0;
			bool found = false;
			YVec3f hitPosition;
			while (!found && counter < 20)
			{
				YVec3f directionRandom(rand() % 2 - 1, rand() % 2 - 1, rand() % 2 - 1);
				//Aucune collision , on peut voler par là
				if (!owl->manager->perceptor->raycast(owl->position, directionRandom,owl->sightRange, hitPosition))
				{
					found = true;
					positionTarget = owl->position + (directionRandom * owl->sightRange);
					owl->forward = directionRandom;
				}
				counter++;
			}

			if (!found)
			{
				owl->switchState(new IdleState(owl, false));
			}
		}

		void update(float elapsed)
		{
			AICreature* targetSnake = owl->manager->perceptor->creatureSight(owl, CreatureType::Snake, OWL_SIGHT_RANGE);

			if (targetSnake != nullptr)
			{
				owl->setEatTarget(targetSnake->position);
				owl->target = targetSnake;
				owl->switchState(new ChaseState(owl));				
			}
			else
			{
				owl->position += owl->forward * speed * elapsed;

				//Point depassé
				if ((positionTarget - owl->position).dot(owl->forward) < 0)
				{
					owl->switchState(new LookingForFoodState(owl));
				}
			}
		}
	};

	struct ChaseState : OwlState
	{
		ChaseState(Owl* owl) : OwlState(owl) {}

		void enter()
		{
			YLog::log(YLog::MSG_TYPE::ENGINE_INFO, "owl rentre dans Chase");
		}

		void update(float elapsed)
		{
			if (owl->updateSatiation(elapsed))
			{
				YVec3f toSnake(owl->target->position - owl->position);
				YVec3f pos;

				//Rien ne gene la vue
				if (!owl->manager->perceptor->raycast(owl->position, toSnake, toSnake.getSize(), pos))
				{
					owl->position += toSnake.normalize() * owl->timeBetweenMoves;

					//Depassé (donc chopppé)
					if (toSnake.dot(owl->forward) < 0)
					{
						owl->eat();

						//Rassasié , va chercher un arbre
						owl->switchState(new LookingForTreeState(owl));
					}
				}
				else
				{
					//Perdu de vue
					owl->switchState(new LookingForFoodState(owl));
				}
			}
		}

		void exit()
		{
			owl->target = nullptr;
		}
	};

	struct LookingForTreeState : OwlState
	{
		LookingForTreeState(Owl* owl) : OwlState(owl) {}
		YVec3f treePos;
		float timeBeforeRetry = 1.0;

		void enter()
		{
			YLog::log(YLog::MSG_TYPE::ENGINE_INFO, "owl rentre dans tree research");
			int counter = 0;
			bool found = false;
			float range = 10;
			while (!found && counter < 20)
			{
				YVec3f directionRandom(rand() % 2 - 1, rand() % 2 - 1, rand() % 2 - 1);
				YVec3f pos;

				//Arbre vue 
				if (owl->manager->perceptor->blockSight(owl,MCube::MCubeType::CUBE_BRANCHES,owl->sightRange,pos))
				{
					treePos = pos;
					owl->switchState(new IdleState(owl, false));
				}
				counter++;
			}
		}
	};

	struct GoToTreeState : OwlState
	{
		GoToTreeState(Owl* owl, YVec3f target) : OwlState(owl) , treePos(target){}
		YVec3f treePos;

		void enter()
		{
			owl->forward = (treePos - owl->position).normalize();
			YLog::log(YLog::MSG_TYPE::ENGINE_INFO, "owl rentre dans gotree");
		}

		void update(float elapsed)
		{
			YVec3f toTarget = treePos - owl->position;
			owl->position += toTarget.normalize() * owl->timeBetweenMoves;

			//Depassé , on se repose sur l'arbre
			if (owl->forward.dot(treePos - owl->position) < 0)
			{
				owl->switchState(new IdleState(owl, true));
			}
		}
	};

	struct FleeState : OwlState
	{
		YVec3f predatorPos;
		YVec3f fleePosition;
		FleeState(Owl* owl, YVec3f predator) : OwlState(owl), predatorPos(predator){}

		void enter()
		{

			YLog::log(YLog::MSG_TYPE::ENGINE_INFO, "owl rentre dans flee");
			YVec3f directionFlee = (owl->position - predatorPos).normalize();
			directionFlee.rotate(YVec3f(0, 0, 1), rand() % 20 - 10);

			int counter = 20;
			YVec3f pos;

			//Champ libre , on y va
			while (!owl->manager->perceptor->raycast(owl->position, directionFlee, 10, pos) && --counter > 0)
			{				
				directionFlee.rotate(YVec3f(0, 0, 1), rand() % 20 - 10);
				directionFlee.rotate(YVec3f(0, 1, 0), rand() % 20 - 10);
				directionFlee.rotate(YVec3f(1, 0, 0), rand() % 20 - 10);
			}

			if (counter)
			{
				fleePosition = owl->position + (directionFlee * 10);
				owl->forward = directionFlee;
			}
			else
			{
				owl->switchState(new IdleState(owl, true));
			}
		}	

		void update(float elapsed)
		{
			owl->position += owl->forward * owl->timeBetweenMoves * elapsed;
			if (owl->forward.dot(fleePosition - owl->position) < 0)
			{
				owl->switchState(new IdleState(owl, false));
			}
		}
	};

	struct GoPechoState : OwlState
	{
		GoPechoState(Owl * owl, Owl * target) : OwlState(owl)
		{
			target = potentialPartner;
		}
		Owl* potentialPartner;

		void enter()
		{
			YLog::log(YLog::MSG_TYPE::ENGINE_INFO, "owl rentre dans Chase");
			owl->forward = (potentialPartner->position - owl->position).normalize();
		}

		void update(float elapsed)
		{
			YVec3f toPartner = potentialPartner->position - owl->position;

			if (owl->updateSatiation(elapsed))
			{
				YVec3f pos;

				//Rien ne gene la vue
				if (!owl->manager->perceptor->raycast(owl->position, toPartner, toPartner.getSize(), pos))
				{

					//Depassé (donc touché), on verifie que le partenaire est toujours valide
					if (toPartner.dot(owl->forward) < 0 && potentialPartner->isPartnerValid())
					{
						
					}
					else
					{
						owl->position += toPartner.normalize() * owl->timeBetweenMoves;
					}
				}
				else
				{
					//Perdu de vue
					owl->switchState(new LookingForTreeState(owl));
				}
			}
		}

		void exit()
		{
			owl->target = nullptr;
		}
	};

	struct ReproState : OwlState
	{
		float timerBeforeReproduce;
		Owl* part;
		ReproState(Owl* owl, Owl * partner) : OwlState(owl), part(partner)
		{
			owl->setPartner(partner);

			//Delai aleatoire pour eviter la double reproduction
			timerBeforeReproduce = rand() % 3;
		}

		void update(float elapsed)
		{
			timerBeforeReproduce -= elapsed;
			if (timerBeforeReproduce < 0)
			{
				//Reproduction en soi
				owl->reproduce();
				owl->switchState(new IdleState(owl, true));
				part->switchState(new IdleState(part, true));
			}
		}

		void exit()
		{
			owl->resetPartner();
		}
	};

#pragma endregion

	Owl(string name, MWorld* world, CreatureManager* manager, YVec3f pos) :
		AICreature(name, world, manager, pos, true, SPEED, DECAY_SATIETY, REPRO_THRESHOLD)
	{
		manager->registerCreature(this);
		sightRange = OWL_SIGHT_RANGE;
		switchState(new IdleState(this, true));
	}

	~Owl()
	{
		manager->unregisterCreature(this);
		delete state;
	}

	bool isPartnerValid() override
	{
		return (partner != NULL && partner->canReproduce() && dynamic_cast<Owl*>(partner)->isOnBranch);
	}

	bool setPartner(AICreature * newPartner) override
	{
		//Already have a partner
		if (partner != nullptr)
			return false;

		Owl* owl = dynamic_cast<Owl*>(newPartner);
		if (owl != nullptr)
		{
			partner = owl;
			return partner->isPartnerValid();
		}
		//Not an owl
		return false;
	}

	void eat() override
	{
		target->die();
		satiation += 0.5;
		target = nullptr;
	}

	void die()
	{
		YLog::log(YLog::MSG_TYPE::ENGINE_INFO, "owl est mort");
		AICreature::die();
	}

	void reproduce() override
	{
		YVec3f spawnPosition = (partner->position + position) / 2;
		new Owl("Owl enfant ", world, manager, spawnPosition);
	}

	bool isEatTargetValid() override
	{
		return true;
	}

	float getSatiation()
	{
		return satiation;
	}

	CreatureType* getType()
	{
		return CreatureType::Owl;
	}

	protected :
		AICreature* target;
		float sightRange;
		bool isOnBranch;
};
