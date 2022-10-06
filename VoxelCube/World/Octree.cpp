#include "Octree.h"
#include <random>

Octree::Octree() {
	root = new OctreeNode(nullptr, (uint32_t)(1));	// Zero initialize octree
	root->LocCode = 1;	// 0...0001. A depth of 0
}

Octree::~Octree() {
	DeleteNode(root);	// Delete root
	return;
}

void Octree::DeleteNode(uint32_t LocCode) {
	OctreeNode* node = GetNode(LocCode);
	DeleteNode(node);
	return;
}

void Octree::DeleteNode(OctreeNode* node) {
	if (node == nullptr) {
		return;
	}

	for (int i = 0; i < 8; i++) {	// Post-order traversal
		if (node->Children[i] != nullptr) {
			DeleteNode(node->Children[i]);
		}
	}
	delete node;
	node = nullptr;
}

OctreeNode* Octree::GetNode(uint32_t LocCode) {
	if (LocCode == NULL) {
		return nullptr;
	}

	uint32_t depth = GetLocDepth(LocCode);
	unsigned short shift = depth * 3 - 3;
	OctreeNode* currentNode = root;
	// Work our way downs. The bitwise shift just extracts the relevant index
	for (int i = 0; i < depth; i++) {
		OctreeNode* nextNode = currentNode->Children[(LocCode >> shift) & 7];
		if (nextNode == nullptr) {
			return nullptr;
		}
		currentNode = nextNode;
		shift -= 3;
	}
	return currentNode;
}

void Octree::InsertNode(uint32_t LocCode, glm::vec4 color) {
	uint32_t depth = GetLocDepth(LocCode);
	unsigned short shift = 3 * depth - 3;
	OctreeNode* currentNode = root;
	// Work our way down. The bitwise shift just extracts the relevant index
	for (int i = 0; i < depth; i++) {
		// Need to create the child if it doesn't exist
		if (currentNode->Children[(LocCode >> shift & 7)] == nullptr) {
			currentNode->Children[(LocCode >> shift & 7)] = new OctreeNode(currentNode, ((currentNode->LocCode) << 3) + ((LocCode >> shift) & 7));
			// TODO: Update visibility of child here
		}
		OctreeNode* nextNode = currentNode->Children[(LocCode >> shift & 7)];
		
		currentNode = nextNode;
		// Now we also need to cull the face later, so change the bitmap of the node as well as the surrounding nodes
		UpdateVisibility(currentNode->LocCode);

		shift -= 3;
	}

	currentNode->color = color;
	currentNode->isLeaf = true;
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

void Octree::UpdateVisibility(uint32_t LocCode) {
	// TODO: might not need this node here. Could create an overriding updatevisibility(loccode) function instead
	OctreeNode* node = GetNode(LocCode);
	if (node == nullptr) return;

	size_t depth = GetLocDepth(LocCode);
	glm::u32vec3 pos = LocCodeToPos(LocCode);
	uint32_t size = pow(2, (Octree::MAXDEPTH - GetLocDepth(LocCode)));	// Size of the cube

	OctreeNode* neighbor;

	// Front x neighbor
	neighbor = GetNode(PosToLocCode(glm::u32vec3(pos.x + size, pos.y, pos.z), depth));
	if (neighbor != nullptr) {
		UpdateVisibilityCode(node->visibility, 4, 0);
		UpdateVisibilityCode(neighbor->visibility, 5, 0);
	}
	
	// Back x neighbor
	neighbor = GetNode(PosToLocCode(glm::u32vec3(pos.x - size, pos.y, pos.z), depth));
	if (neighbor != nullptr) {
		UpdateVisibilityCode(node->visibility, 5, 0);
		UpdateVisibilityCode(neighbor->visibility, 4, 0);
	}

	// Front y neighbor
	neighbor = GetNode(PosToLocCode(glm::u32vec3(pos.x, pos.y + size, pos.z), depth));
	if (neighbor != nullptr) {
		UpdateVisibilityCode(node->visibility, 2, 0);
		UpdateVisibilityCode(neighbor->visibility, 3, 0);
	}
	
	// Back y neighbor
	neighbor = GetNode(PosToLocCode(glm::u32vec3(pos.x, pos.y - size, pos.z), depth));
	if (neighbor != nullptr) {
		UpdateVisibilityCode(node->visibility, 3, 0);
		UpdateVisibilityCode(neighbor->visibility, 2, 0);
	}

	// Front z neighbor
	neighbor = GetNode(PosToLocCode(glm::u32vec3(pos.x, pos.y, pos.z + size), depth));
	if (neighbor != nullptr) {
		UpdateVisibilityCode(node->visibility, 0, 0);
		UpdateVisibilityCode(neighbor->visibility, 1, 0);
	}

	// Back z neighbor
	neighbor = GetNode(PosToLocCode(glm::u32vec3(pos.x, pos.y, pos.z - size), depth));
	if (neighbor != nullptr) {
		UpdateVisibilityCode(node->visibility, 1, 0);
		UpdateVisibilityCode(neighbor->visibility, 0, 0);
	}
}

inline void Octree::UpdateVisibilityCode(uint8_t &visibility, uint8_t n, bool val) {
	visibility ^= (-val ^ visibility) & (1UL << n);
	
	// Update at least one face set bit
	if (visibility & 63UL) {		// This would mean at least one face is set
		visibility |= (1UL << 6);	// Set "at least one face" to true
	}
	else {
		visibility = (uint8_t)0;
	}

	// Update at least all faces set bit
	if ((visibility & 63UL) == 63) {	// This would mean all faces are set
		visibility = 255;
		return;
	}
	else {
		visibility &= ~((uint8_t)1 << 7);	// Set "all faces" to false
	}
}

inline bool Octree::GetVisibilityCode(uint8_t& visibility, uint8_t n) {
	return (visibility >> n) & 1U;
}

void Octree::CreateMesh(Renderer * renderer, uint32_t LocCode, size_t detail) {
	size_t depth = GetLocDepth(LocCode);
	if (depth == Octree::MAXDEPTH) {
		CreateMesh(renderer, LocCode);
		return;
	}

	// If we're at the required LOD, render
	if (detail == 0) {
		CreateMesh(renderer, LocCode);
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

	// If this is a leaf, we create the block
	if (!hasChildren) {
		CreateMesh(renderer, LocCode);
		return;
	}

	// Finally, if still not rendered, then clearly we must be rendering the available children at a higher LOD
	for (int i = 0; i < 8; i++) {
		if (node->Children[i] == nullptr) { continue; }
		Octree::CreateMesh(renderer, node->Children[i]->LocCode, detail - 1);
	}
}

void Octree::CreateMesh(Renderer * renderer, uint32_t LocCode) {
	uint32_t size = pow(2, (Octree::MAXDEPTH - GetLocDepth(LocCode)));	// Size of the cube

	glm::vec3 pos = LocCodeToPos(LocCode);

	OctreeNode* node = GetNode(LocCode);

	// TODO: see if you really need to search for the node again here
	renderer->CreateCube(pos.x, pos.y, pos.z, size, node->visibility);
}

void Octree::StageMesh(Renderer * renderer) {
	renderer->StageMesh(1, 1);
}

void Octree::Render(Renderer * renderer, Camera camera, const unsigned int WIDTH, const unsigned int HEIGHT) {
	renderer->RenderMesh(&blockShader, camera, WIDTH, HEIGHT);
}

void Octree::InsertRandomNodes(Renderer* renderer, size_t depth=Octree::MAXDEPTH) {
	InsertRandomNodes(renderer, root, depth);
}

void Octree::InsertRandomNodes(Renderer * renderer, OctreeNode* node, size_t depth=Octree::MAXDEPTH) {
	if (GetLocDepth(node->LocCode) == depth) {
		return;
	}
	
	int r;
	for (int i = 0; i < 5; i++) {
		/*
		srand(node->LocCode * i);
		r = rand() % 8;
		if (node->Children[r] == nullptr) {
			node->Children[r] = new OctreeNode();
			node->Children[r]->LocCode = ((node->LocCode) << 3) + (r);
		}
		*/

		if (node->Children[i] == nullptr) {
			//node->Children[i] = new OctreeNode();
			uint32_t LocCode = ((node->LocCode) << 3) + (i);
			InsertNode(LocCode, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		}
	}

	for (int i = 0; i < 8; i++) {
		if (node->Children[i] != nullptr) {
			InsertRandomNodes(renderer, node->Children[i], depth);
		}
	}
}

inline glm::u32vec3 Octree::LocCodeToPos(uint32_t LocCode) {
	uint32_t x = 0, y = 0, z = 0;
	size_t depth = GetLocDepth(LocCode);
	for (int i = 0; i < depth; i++) {
		x += (bool)(LocCode & (1 << (i * 3 + 2))) << (Octree::MAXDEPTH - depth + i);
		y += (bool)(LocCode & (1 << (i * 3 + 1))) << (Octree::MAXDEPTH - depth + i);
		z += (bool)(LocCode & (1 << (i * 3 + 0))) << (Octree::MAXDEPTH - depth + i);
	}
	return glm::u32vec3(x, y, z);
}

inline uint32_t Octree::PosToLocCode(glm::u32vec3 pos, size_t depth) {
	if (std::max({ pos.x, pos.y, pos.z }) >= pow(2, Octree::MAXDEPTH + 1)) {
		return NULL;
	}

	if (std::min({ pos.x, pos.y, pos.z }) < 0) {
		return NULL;
	}

	uint32_t LocCode = 1 << depth * 3;

	for (int i = 0; i < depth; i++) {
		LocCode += (1 << (depth * 3 - 3 * i - 1)) * bool(1 & (pos.x >> (10 - i - 1)));
		LocCode += (1 << (depth * 3 - 3 * i - 2)) * bool(1 & (pos.y >> (10 - i - 1)));
		LocCode += (1 << (depth * 3 - 3 * i - 3)) * bool(1 & (pos.z >> (10 - i - 1)));
	}

	return LocCode;
}
