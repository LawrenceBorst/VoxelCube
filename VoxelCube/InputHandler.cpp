#include "InputHandler.h";

Command* InputHandler::handleInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_A)) return buttonA_;
	if (glfwGetKey(window, GLFW_KEY_D)) return buttonD_;
	if (glfwGetKey(window, GLFW_KEY_S)) return buttonS_;
	if (glfwGetKey(window, GLFW_KEY_W)) return buttonW_;
	// Nothing pressed, do nothing
	return NULL;
}
