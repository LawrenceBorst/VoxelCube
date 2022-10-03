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

/* Not compact, but elegant enough */
struct OctreeNode {
	OctreeNode* Children[8] = { nullptr };
	OctreeNode* Parent = { nullptr };
	uint32_t id;	// Implicitly contains the block code at leafs. Otherwise the sum of all block codes in the node.
	uint32_t LocCode;
	glm::vec4 color;
	bool isLeaf;
	bool isVisible;
};

class Octree {
static const unsigned short MAXDEPTH = 4;
public:
	Octree();
	~Octree();
	OctreeNode* GetNode(uint32_t LocCode);

	/* Insert a node into the octree. NOTE: Currently overwrites existing nodes */
	void InsertNode(uint32_t LocCode, glm::vec4 color);

	/* Remove nodes. TODO: Implement this*/
	void RemoveNode(uint32_t LocCode);

	/* Renders the node at a given level of detail. A detail of 0 means just one block for this node.
	A detail of 1 means 8 blocks inside the node are also seen etc. */
	void RenderNode(uint32_t LocCode, size_t detail);
	
	/* Renders the node as one big block, without considering child nodes */
	void RenderNode(uint32_t LocCode);
	
private:
	OctreeNode root;

	/* Get the depth of the voxel corresponding to the location code
	The depth is relative to the root node, which has depth 0
	Example: GetLocDepth(...0001011001) = 2*/
	uint32_t GetLocDepth(uint32_t LocCode);
	std::unordered_map<uint32_t, OctreeNode*> DAGhash;
};
