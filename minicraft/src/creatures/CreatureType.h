#pragma once

#include "external/gl/glew.h"
#include "external/gl/freeglut.h"

#define CREATURE_TYPE_COUNT 12

class YVbo;

class CreatureType {
public:
	static CreatureType*const Bird;
	static CreatureType*const Ocelot;
	static CreatureType*const Bear;
	static CreatureType*const Copycat;
	static CreatureType*const Wolf;
	static CreatureType*const Rat;
	static CreatureType*const Owl;
	static CreatureType*const Trap;
	static CreatureType*const Griffin;
	static CreatureType*const Snake;
	static CreatureType*const Goat;
	static CreatureType*const Elephant;

	static CreatureType*const all[CREATURE_TYPE_COUNT];

	static void initMeshes(YVbo* vbo, GLuint shader);

	const int id;
	YVbo* vbo;
	GLuint shader;

	CreatureType() = delete;
	CreatureType(const CreatureType&) = delete;

private:
	CreatureType(int id) : id(id), vbo(nullptr), shader(0) {}
};
