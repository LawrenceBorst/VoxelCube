#pragma once
#include <iostream>

class Command
{
public:
	virtual ~Command() {}
	//virtual void execute(Entity& actor) = 0;
private:
	void isPressed();
};




