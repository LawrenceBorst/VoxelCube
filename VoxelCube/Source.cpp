#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <filesystem>
#include "Core/Camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "InputHandler.h";
#include "Command.h";
#include "Core/EntityCoordinator.h"
#include "Core/BlockShader.cpp"
#include "World/Octree.h"

// Input callbacks, mainly navigation and window-related
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// Screen settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

int main() {
    // Initialize window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Create window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "VoxelCube", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Tell GLFW to capture the mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Load OPENGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Configure global opengl state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);

    // Build and compile shader
    BlockShader lightingShader("Core/VertexShader.txt", "Core/FragmentShader.txt");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    int vertices[] = {
        // positions (3) normals (3)
        -1, -1, -1, 0, 0, -1,
         1,  1, -1, 0, 0, -1,
         1, -1, -1, 0, 0, -1,
         1,  1, -1, 0, 0, -1,
        -1, -1, -1, 0, 0, -1,
        -1,  1, -1, 0, 0, -1,

        -1, -1,  1, 0, 0, 1,
         1, -1,  1, 0, 0, 1,
         1,  1,  1, 0, 0, 1,
         1,  1,  1, 0, 0, 1,
        -1,  1,  1, 0, 0, 1,
        -1, -1,  1, 0, 0, 1,

        -1,  1,  1, -1, 0, 0,
        -1,  1, -1, -1, 0, 0,
        -1, -1, -1, -1, 0, 0,
        -1, -1, -1, -1, 0, 0,
        -1, -1,  1, -1, 0, 0,
        -1,  1,  1, -1, 0, 0,

         1,  1,  1, 1, 0, 0,
         1, -1, -1, 1, 0, 0,
         1,  1, -1, 1, 0, 0,
         1, -1, -1, 1, 0, 0,
         1,  1,  1, 1, 0, 0,
         1, -1,  1, 1, 0, 0,

        -1, -1, -1, 0, -1, 0,
         1, -1, -1, 0, -1, 0,
         1, -1,  1, 0, -1, 0,
         1, -1,  1, 0, -1, 0,
        -1, -1,  1, 0, -1, 0,
        -1, -1, -1, 0, -1, 0,

        -1,  1, -1, 0, 1, 0,
         1,  1,  1, 0, 1, 0,
         1,  1, -1, 0, 1, 0,
         1,  1,  1, 0, 1, 0,
        -1,  1, -1, 0, 1, 0,
        -1,  1,  1, 0, 1, 0,
    };

    // Cube VBO and VAO
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 3, GL_INT, GL_FALSE, 6 * sizeof(int), (void*)0);    // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_INT, GL_TRUE, 6 * sizeof(int), (void*)(3 * sizeof(int)));    // normals
    glEnableVertexAttribArray(1);


    glBindBuffer(GL_ARRAY_BUFFER, VBO);


    /******************
     * MAIN GAME LOOP *
     ******************/

    // Time handling for setting up a steady 60fps experience
    auto MS_PER_FRAME = 60;
    auto previous = std::chrono::system_clock::now();
    std::chrono::duration<double> lag = std::chrono::milliseconds::zero();

    while (!glfwWindowShouldClose(window))
    {
        // TODO: Make this part of lag and previous
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        auto current = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed = current - previous;
        previous = current;
        lag += elapsed;

        /***************
        * HANDLE INPUT *
        ****************/
        processInput(window);
        //InputHandler inputHandler= new InputHandler();

        //Entity player = new Entity();

        //Command* command = inputHandler.handleInput();

        //if (command) {
            //command->execute(player);
        //}

        /******************
        * HANDLE ENTITIES *
        *******************/
        EntityCoordinator gEntityCoordinator = EntityCoordinator();

        while (lag >= std::chrono::milliseconds(MS_PER_FRAME))
        {
            /***************
            * UPDATE CYCLE *
            ****************/



            lag -= std::chrono::milliseconds(MS_PER_FRAME);
        }

        /*********
        * RENDER *
        **********/
        glClearColor(0.44f, 0.73f, 0.87f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // be sure to activate shader when setting uniforms/drawing objects
        lightingShader.use();
        lightingShader.setVec3("objectColor", 1.0f, 0.8f, 0.5f);
        lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        lightingShader.setMat4("model", model);

        // render the cube
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}