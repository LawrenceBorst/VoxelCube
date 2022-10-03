#pragma once

/* Handles all entities (players, bots or mobs) in the world
* TODO: Consider using singleton class
*/

#include "Entity.h"
#include "array"
#include <string>

class EntityCoordinator
{
const static size_t MIN_ENTITIES = 1;
private:
	Entity * entities;	// Represents all entities in the game
	size_t numEntities;
	size_t entitiesSize;
public:
	EntityCoordinator();
	void AddData(Entity &entity);
	Entity& GetData(size_t entityID);
};

