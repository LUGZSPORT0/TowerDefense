#pragma once
#include "Actor.h"
#include <vector>

class Tile : public Actor
{
public: 
	// A friend function is a function that isn't a member of a class but has access to the class's private and protected members. 
	// Friend functions aren't considered class members; they're normal external functions that are given special access privileges. 
	friend class Grid;
	enum TileState
	{
		EDefault,
		EPath,
		EStart,
		EBase
	};

	Tile(class Game* game);

	void SetTileState(TileState state);
	TileState GetTileState() const { return mTileState; }
	void ToggleSelect();
	const Tile* GetParent() const { return mParent; }	
private:
	// For pathfinding
	std::vector<Tile*> mAdjacent;
	Tile* mParent;
	float f;
	float g;
	float h;
	bool mInOpenSet;
	bool mInClosedSet;
	bool mBlocked;

	void UpdateTexture();
	class SpriteComponent* mSprite;
	TileState mTileState;
	bool mSelected;
};

