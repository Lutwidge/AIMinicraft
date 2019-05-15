#include "AICreature.h"

#include "engine_minicraft.h"

void AICreature::render(MEngineMinicraft* engine) {
	glPushMatrix();
	glUseProgram(getType()->shader);
	GLuint colorLocation = glGetUniformLocation(getType()->shader, "Color");
	glUniform3f(colorLocation, getType()->color.R, getType()->color.V, getType()->color.B);
	glTranslatef(position.X + MCube::CUBE_SIZE / 2.0f, position.Y + MCube::CUBE_SIZE / 2.0f, position.Z + MCube::CUBE_SIZE / 2.0f);
	engine->Renderer->updateMatricesFromOgl();
	engine->Renderer->sendMatricesToShader(getType()->shader);
	getType()->vbo->render();
	glPopMatrix();
}
