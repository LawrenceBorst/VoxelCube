#include "Octree.h"

Octree::Octree() {
	OctreeNode node = OctreeNode();	// Zero initialize octree
	node.LocCode = 1;	// 0...0001. A depth of 0
}

Octree::~Octree() {
	return;
}

OctreeNode* Octree::GetNode(uint32_t LocCode) {
	uint32_t depth = GetLocDepth(LocCode);
	unsigned short shift = depth * 3;
	OctreeNode* currentNode = &root;
	// Work our way downs. The bitwise shift just extracts the relevant index
	for (int i = 0; i < depth; i++) {
		OctreeNode* nextNode = currentNode->Children[(LocCode >> shift) & 7];
		currentNode = nextNode;
		shift -= 3;
	}
	return currentNode;
}

void Octree::InsertNode(uint32_t LocCode, glm::vec4 color) {
	uint32_t depth = GetLocDepth(LocCode);
	unsigned short shift = 3;
	OctreeNode* currentNode = &root;
	// Work our way downs. The bitwise shift just extracts the relevant index
	for (int i = 0; i < depth; i++) {
		// Need to create the child if it doesn't exist
		if (currentNode->Children[(LocCode >> shift) & 7] == nullptr) {
			currentNode->Children[(LocCode >> shift) & 7] = new OctreeNode();	// TODO: need to delete this as well eventually
			currentNode->Children[(LocCode >> shift) & 7]->Parent = currentNode;
		}
		
		// Work our way down one LOD
		OctreeNode* nextNode = currentNode->Children[(LocCode >> shift) & 7];
		currentNode = nextNode;
		shift += 3;
	}
	currentNode->color = color;
}

void Octree::RemoveNode(uint32_t LocCode) {
	return;
}

uint32_t Octree::GetLocDepth(uint32_t LocCode) {
#if defined(__GNUC__)
    return (31 - __builtin_clz(node->LocCode)) / 3;
#elif defined(_MSC_VER)
	unsigned long msb;
	_BitScanReverse(&msb, LocCode);
	return msb / 3;
#endif
}

void Octree::RenderNode(Renderer renderer, uint32_t LocCode, size_t detail) {
	size_t depth = GetLocDepth(LocCode);
	// If we're at the required LOD, render
	if (detail == 0) {
		RenderNode(renderer, LocCode);
		return;
	}

	// Else, move on and look for children. If no children, we render
	OctreeNode* node = GetNode(LocCode);

	// Check if this node is a leaf
	bool hasChildren = false;
	for (int i = 0; i < 8; i++) {
		if (node->Children[i] != nullptr) {
			hasChildren = true;
		}
	}

	// If this is a leaf, we render
	if (!hasChildren) {
		return;
	}

	// Finally, if still not rendered, then clearly we must be rendering the available children at a higher LOD
	for (int i = 0; i < 8; i++) {
		if (node->Children[i] == nullptr) { continue; }
		Octree::RenderNode(renderer, node->Children[i]->LocCode, detail - 1);
	}
}

void Octree::RenderNode(Renderer renderer, uint32_t LocCode) {
	size_t depth = GetLocDepth(LocCode);
	uint32_t size = 2 ^ (Octree::MAXDEPTH - depth);	// Size of the cube
	uint32_t x=0, y=0, z=0;

	for (int i = 0; i < depth; i++) {
		x += ((bool)(LocCode & (1 << (i * 3 + 2)))) * (2 ^ (Octree::MAXDEPTH - depth + i));
		y += ((bool)(LocCode & (1 << (i * 3 + 1)))) * (2 ^ (Octree::MAXDEPTH - depth + i));
		z += ((bool)(LocCode & (1 << (i * 3 + 0)))) * (2 ^ (Octree::MAXDEPTH - depth + i));
	}


}