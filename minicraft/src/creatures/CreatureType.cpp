#include "CreatureType.h"

CreatureType*const CreatureType::Bird = new CreatureType(0);
CreatureType*const CreatureType::Ocelot = new CreatureType(1);
CreatureType*const CreatureType::Bear = new CreatureType(2);
CreatureType*const CreatureType::Copycat = new CreatureType(3);
CreatureType*const CreatureType::Wolf = new CreatureType(4);
CreatureType*const CreatureType::Rat = new CreatureType(5);
CreatureType*const CreatureType::Owl = new CreatureType(6);
CreatureType*const CreatureType::Trap = new CreatureType(7);
CreatureType*const CreatureType::Griffin = new CreatureType(8);
CreatureType*const CreatureType::Snake = new CreatureType(9);
CreatureType*const CreatureType::Goat = new CreatureType(10);
CreatureType*const CreatureType::Elephant = new CreatureType(11);

CreatureType*const CreatureType::all[CREATURE_TYPE_COUNT] = {
	CreatureType::Bird,
	CreatureType::Ocelot,
	CreatureType::Bear,
	CreatureType::Copycat,
	CreatureType::Wolf,
	CreatureType::Rat,
	CreatureType::Owl,
	CreatureType::Trap,
	CreatureType::Griffin,
	CreatureType::Snake,
	CreatureType::Goat,
	CreatureType::Elephant
};

void CreatureType::initMeshes(YVbo* vbo, GLuint shader) {
	for (int i = 0; i < CREATURE_TYPE_COUNT; i++) {
		all[i]->vbo = vbo;
		all[i]->shader = shader;
	}
}
