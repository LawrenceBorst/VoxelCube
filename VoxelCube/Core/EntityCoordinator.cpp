/* For better data handling, I want to keep the array packed at all times 
* I use two hashmaps for this purpose
* TODO: Keep the data packed */

#include "EntityCoordinator.h"

EntityCoordinator::EntityCoordinator() {
	entities = new Entity[EntityCoordinator::MIN_ENTITIES];
	numEntities = 0;
	entitiesSize = 1;
}

void EntityCoordinator::AddData(Entity &entity) {
	entities[entitiesSize - 1] = entity;

	if (numEntities == entitiesSize) {	// Array needs resizing
		size_t newSize = entitiesSize * 2;
		Entity* newEntities = new Entity[newSize];
		memcpy(newEntities, entities, entitiesSize * sizeof(Entity));
		entitiesSize = newSize;
		delete [] entities;
		entities = newEntities;
	}
}

Entity& EntityCoordinator::GetData(size_t entityID) {
	return entities[entityID];
}