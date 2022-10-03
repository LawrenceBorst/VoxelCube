#pragma once
class Block
{
	const static enum BlockType {
		BlockType_Air = 0,
		BlockType_Grass = 1,
		BlockType_Dirt = 2,
		BlockType_Water = 3,
		BlockType_Stone = 4,
		BlockType_Wood = 5,
		BlockType_Sand = 6,
	};

public:
	Block();
	~Block();
	bool IsVisible();
	void SetVisible(bool flag);
private:
	bool visible;
	BlockType blockType;
};

