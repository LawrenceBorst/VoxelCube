#pragma once

#include <GLFW/glfw3.h>
#include "Command.h";

class InputHandler
{
public:
	Command* handleInput(GLFWwindow* window);

private:
	Command* buttonW_;
	Command* buttonA_;
	Command* buttonS_;
	Command* buttonD_;
};

