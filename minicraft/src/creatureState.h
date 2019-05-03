#pragma once

#include "creatures/creature.h"

class CreatureState
{
public:
	virtual ~CreatureState() {}
	virtual void update(Creature* creature) {}
};