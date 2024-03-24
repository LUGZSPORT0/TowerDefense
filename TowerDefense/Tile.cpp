#include "Tile.h"
#include "SpriteComponent.h"
#include "Game.h"

Tile::Tile(class Game* game)
	:Actor(game)
	, mParent(nullptr)
	, f(0.0f)
	, g(0.0f)
	, h(0.0f)
	, mBlocked(false)
	, mSprite(nullptr)
	, mTileState(EDefault)
	, mSelected(false)
{
	mSprite = new SpriteComponent(this);
	UpdateTexture();
}

void Tile::SetTileState(TileState state)
{
	mTileState = state;
	UpdateTexture();
}

void Tile::ToggleSelect()
{
	if (!sameSelected)
	{
		mSelected = !mSelected;
	}
	UpdateTexture();
}

void Tile::UpdateTexture()
{
	std::string text;
	switch (mTileState)
	{
	case EStart:
		text = "Assets/TileTan.png";
		break;
	case EBase:
		class SpriteComponent* baseBottom;
		baseBottom = new SpriteComponent(this, 10);
		text = "Assets/TileGreen.png";
		baseBottom->SetTexture(GetGame()->GetTexture(text));
		text = "Assets/Base.png";
		break;
	case EPath:
		if (mSelected)
			text = "Assets/TileGreySelected.png";
		else
			text = "Assets/TileGrey.png";
		break;
	case EDefault:
	default:
		if (mSelected)
			text = "Assets/TileBrownSelected.png";
		else
			text = "Assets/TileBrown.png";
		break;
	}
	mSprite->SetTexture(GetGame()->GetTexture(text));
}