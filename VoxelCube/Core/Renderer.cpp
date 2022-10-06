#include "Renderer.h"

Renderer::Renderer() {
    //Renderer::vertexArray = {};
}

void Renderer::StageMesh(unsigned int VAO, unsigned int VBO) {
    this->VAO = VAO;
    this->VBO = VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, this->vertexArray.size() * sizeof(int), &(this->vertexArray)[0], GL_STATIC_DRAW);

    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 3, GL_INT, GL_FALSE, 6 * sizeof(int), (void*)0);    // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_INT, GL_TRUE, 6 * sizeof(int), (void*)(3 * sizeof(int)));    // normals
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
}

void Renderer::CreateCube(uint32_t x, uint32_t y, uint32_t z, uint32_t width) {
    // TODO: Separate the normals and positions to use int and unsigned ints separately
    // TODO: Also need to add color data(!)
    int X = (int)x;
    int Y = (int)y;
    int Z = (int)z;
    int WIDTH = (int)width;

    std::vector<int> vertices = {
        // positions (3)        normals (3)
        X,      Y,      Z,      0, 0, -1,
        X+WIDTH,Y+WIDTH,Z,      0, 0, -1,
        X+WIDTH,Y,      Z,      0, 0, -1,
        X+WIDTH,Y+WIDTH,Z,      0, 0, -1,
        X,      Y,      Z,      0, 0, -1,
        X,      Y+WIDTH,Z,      0, 0, -1,

        X,      Y,      Z+WIDTH,0, 0, 1,
        X+WIDTH,Y,      Z+WIDTH,0, 0, 1,
        X+WIDTH,Y+WIDTH,Z+WIDTH,0, 0, 1,
        X+WIDTH,Y+WIDTH,Z+WIDTH,0, 0, 1,
        X,      Y+WIDTH,Z+WIDTH,0, 0, 1,
        X,      Y,      Z+WIDTH,0, 0, 1,

        X,      Y+WIDTH,Z+WIDTH,-1, 0, 0,
        X,      Y+WIDTH,Z,      -1, 0, 0,
        X,      Y,      Z,      -1, 0, 0,
        X,      Y,      Z,      -1, 0, 0,
        X,      Y,      Z+WIDTH,-1, 0, 0,
        X,      Y+WIDTH,Z+WIDTH,-1, 0, 0,

        X+WIDTH,Y+WIDTH,Z+WIDTH,1, 0, 0,
        X+WIDTH,Y,      Z,      1, 0, 0,
        X+WIDTH,Y+WIDTH,Z,      1, 0, 0,
        X+WIDTH,Y,      Z,      1, 0, 0,
        X+WIDTH,Y+WIDTH,Z+WIDTH,1, 0, 0,
        X+WIDTH,Y,      Z+WIDTH,1, 0, 0,

        X,      Y,      Z,      0, -1, 0,
        X+WIDTH,Y,      Z,      0, -1, 0,
        X+WIDTH,Y,      Z+WIDTH,0, -1, 0,
        X+WIDTH,Y,      Z+WIDTH,0, -1, 0,
        X,      Y,      Z+WIDTH,0, -1, 0,
        X,      Y,      Z,      0, -1, 0,

        X,      Y+WIDTH,Z,      0, 1, 0,
        X+WIDTH,Y+WIDTH,Z+WIDTH,0, 1, 0,
        X+WIDTH,Y+WIDTH,Z,      0, 1, 0,
        X+WIDTH,Y+WIDTH,Z+WIDTH,0, 1, 0,
        X,      Y+WIDTH,Z,      0, 1, 0,
        X,      Y+WIDTH,Z+WIDTH,0, 1, 0,
    };

    this->vertexArray.insert(this->vertexArray.end(), vertices.begin(), vertices.end());    // Append vertices above to vertexArray

	return;
};

void Renderer::RenderMesh(BlockShader * shader, Camera camera, const unsigned int WIDTH, const unsigned int HEIGHT) {
    // be sure to activate shader when setting uniforms/drawing objects
    shader->use();
    shader->setVec3("objectColor", 1.0f, 0.8f, 0.5f);
    shader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    shader->setVec3("light.direction", -0.2f, -1.0f, -0.3f);
    shader->setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
    shader->setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
    shader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("material.shininess", 32.0f);
    shader->setInt("material.diffuse", 0);
    shader->setInt("material.specular", 1);

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 10000.0f);
    glm::mat4 view = camera.GetViewMatrix();
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    shader->setVec3("viewPos", camera.Position);


    // world transformation
    glm::mat4 model = glm::mat4(1.0f);
    shader->setMat4("model", model);

    // render the cube
    glBindVertexArray(this->VAO);
    glDrawArrays(GL_TRIANGLES, 0, this->vertexArray.size());
}

void Renderer::UnbindMesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}
