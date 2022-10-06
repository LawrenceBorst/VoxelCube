#pragma once

/* (Note: I define a chunk to mean any node... A chunk could be any of 1^3, 16^3, or 512^3 in size)
* 
* Octree using a node-based implementation. Node-based to easily construct DAGs from it
* Each entry in the octree also has an ID computed as SUM (all block values in node) mod 65536
* This ID is used to aid in the process of adding/removing blocks
* 
* The DAGhash tells us, given an ID, candidate chunks.
* 
* The location code is borrowed from linear hashed octrees, and it implicitly stores the depth of a node
* A location is an element in 1{0, 1}^{3n}, where going right in the code corresponds to a higher level of detail
*/

#include "glm/glm.hpp"
#include <cstdint>
#include <unordered_map>
#include "../Core/Renderer.h"

/* Not compact, but elegant enough */
struct OctreeNode {
	OctreeNode* Children[8] = { nullptr };
	OctreeNode* Parent = { nullptr };
	uint16_t id = 0;	// Implicitly contains the block code at leafs. Otherwise the sum of all block codes in the node.
	uint32_t LocCode;
	glm::vec4 color;
	bool isLeaf = false;
	uint8_t visibility = (uint8_t)(255);	// Visibility bitmask. Order is: all_faces, at_least_one_face, x_small, x_big, y_small, y_big, z_small, z_big
	OctreeNode(OctreeNode* p, uint32_t LocCode) : Parent(p),LocCode(LocCode) { };
};

class Octree {
static const unsigned short MAXDEPTH = 10;
BlockShader blockShader = BlockShader("Core/VertexShader.txt", "Core/FragmentShader.txt");

public:
	Octree();
	~Octree();

	void DeleteNode(uint32_t LocCode);

	void DeleteNode(OctreeNode* node);

	/* Gets a node. If the node does not exist, returns a nullptr */
	OctreeNode* GetNode(uint32_t LocCode);

	/* Insert a node into the octree. NOTE: Currently overwrites existing nodes */
	void InsertNode(uint32_t LocCode, glm::vec4 color);

	/* Remove nodes. TODO: Implement this*/
	void RemoveNode(uint32_t LocCode);

	/* Adds a mesh at a given level of detail. A detail of 0 means just one block for this node.
	A detail of 1 means 8 blocks inside the node are also seen etc. */
	void CreateMesh(Renderer * renderer, uint32_t LocCode, size_t detail);
	
	/* Add a block to the renderer, without considering child nodes */
	void CreateMesh(Renderer * renderer, uint32_t LocCode);
	
	/* Stage the mesh... NOTE: this appends(!) to the mesh vector*/
	void StageMesh(Renderer* renderer);

	/* Here for testing purposes. Creates random tree at the given height */
	void InsertRandomNodes(Renderer * renderer, OctreeNode* node, size_t depth);
	void InsertRandomNodes(Renderer* renderer, size_t depth);

	/* Render the blocks that were staged by createMesh() */
	void Render(Renderer* renderer, Camera camera, const unsigned int WIDTH, const unsigned int HEIGHT);

	/* Updates the visibility of a node at the LocCode */
	void UpdateVisibility(uint32_t LocCode);

	/* Get a position from a location code */
	inline glm::u32vec3 LocCodeToPos(uint32_t LocCode);

	/* Gets a location code from a position
	Recall that different sized blocks can be found at a position, hence the need for the depth
	NOTE: Requires a valid LocCode to work, otherwise returns garbage */
	inline uint32_t PosToLocCode(glm::u32vec3, size_t depth);
	
private:
	OctreeNode * root;

	/* Get the depth of the voxel corresponding to the location code
	The depth is relative to the root node, which has depth 0
	Example: GetLocDepth(...0001011001) = 2*/
	uint32_t GetLocDepth(uint32_t LocCode);
	std::unordered_map<uint32_t, OctreeNode*> DAGhash;

	/* Update the visibility bitmap
	Sets the nth bit of it to val */
	inline void UpdateVisibilityCode(uint8_t &visibility, uint8_t n, bool val);

	/* Get the nth bit of the a visibility bitmap */
	inline bool GetVisibilityCode(uint8_t& visibility, uint8_t n);
};
