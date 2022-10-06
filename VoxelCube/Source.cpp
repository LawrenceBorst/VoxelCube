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
#include <vector>

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
Camera camera(glm::vec3(-1.0f, -1.0f, -1.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

// Renderer
Renderer grenderer;

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

    Octree gWorld = Octree();
    glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    
    //gWorld.InsertNode((uint32_t)(103), color);    // 000...01100111
    /*
    gWorld.InsertNode((uint32_t)(pow(2, 30)), color);    // 000...01100111
    gWorld.InsertNode((uint32_t)(pow(2, 30) + pow(2, 2) + pow(2,1)), color);    // 000...01100111
    gWorld.InsertNode((uint32_t)(pow(2, 30) + pow(2, 1)), color);    // 000...01100111
    gWorld.InsertNode((uint32_t)(pow(2, 30) + pow(2, 2) + pow(2, 1)), color);    // 000...01100111
    gWorld.InsertNode((uint32_t)(pow(2, 30) + pow(2, 2) + pow(2, 1)), color);    // 000...01100111
    gWorld.InsertNode((uint32_t)(pow(2, 30) + pow(2, 4) + pow(2, 3)), color);    // 000...01100111
    gWorld.InsertNode((uint32_t)(123), color);    // 000...01100111
    gWorld.InsertNode((uint32_t)(53258), color);    // 000...01100111
    gWorld.InsertNode((uint32_t)(8333), color);    // 000...01100111
    gWorld.InsertNode((uint32_t)(pow(2, 27) + pow(2, 6)), color);    // 000...01100111
    gWorld.InsertNode((uint32_t)(pow(2, 24) + pow(2, 9)), color);    // 000...01100111
    gWorld.InsertNode((uint32_t)(pow(2, 21) + pow(2, 12)), color);    // 000...01100111
    */
    gWorld.InsertRandomNodes(&grenderer, 10);

    gWorld.CreateMesh(&grenderer, (uint32_t)(1), 11);
    gWorld.StageMesh(&grenderer);

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
        glClearColor(0.20f, 0.78f, 0.94f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        gWorld.Render(&grenderer, camera, SCR_WIDTH, SCR_HEIGHT);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    grenderer.UnbindMesh();

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