#pragma once

#include "BlockShader.cpp"
#include "Camera.h"

class Renderer
{
public:
	Renderer();

	/* TODO: Allow several of these (in an array perhaps?) to prevent risk of multiple access and allow multithreading */
	std::vector<int> vertexArray;

	/* Stages the VAO and VBO for a voxel mesh with **size** triangles */
	void StageMesh(unsigned int VAO, unsigned int VBO);

	/* Adds a cube to the vertex array */
	void CreateCube(uint32_t x, uint32_t y, uint32_t z, uint32_t width);

	/* For now, does multiple things. TODO: Split this up into smaller bits */
	void RenderMesh(BlockShader shader, Camera camera, const unsigned int WIDTH, const unsigned int HEIGHT);

	/* Unbind VAO and VBO */
	void UnbindMesh();
private:
	// TODO: Allow multithreading for this as well
	unsigned int VAO;
	unsigned int VBO;
};
