#pragma once

#include "glm/glm.hpp";

class Entity
{
public:
	Entity();

	//void update(World& world, Graphics& graphics);

	glm::vec3 pos();
	glm::vec3 vel();

	void setPos(glm::vec3);
	void setVel(glm::vec3);

private:
	glm::vec3 pos_;
	glm::vec3 vel_;
	glm::vec3 rot_;
};

