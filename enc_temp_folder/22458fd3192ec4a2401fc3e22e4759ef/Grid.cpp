#include <iostream>
#include "Grid.h"
#include "Tile.h"
#include "Tower.h"
#include "Enemy.h"
#include <algorithm>
#include <iostream>

Grid::Grid(class Game* game)
	:Actor(game)
	, mSelectedTile(nullptr)
{
	// 7 rows, 16 columns
	mTiles.resize(NumRows);
	for (size_t i = 0; i < mTiles.size(); i++)
	{
		mTiles[i].resize(NumCols);
	}

	// Create tiles
	for (size_t i = 0; i < NumRows; i++)
	{
		for (size_t j = 0; j < NumCols; j++)
		{
			mTiles[i][j] = new Tile(GetGame());
			// i = 0: 64.0f / 2.0f + 0 * 64.0f = 32
			// i = 1: (64.0f / 2.0f) + (1 * 64.0f) = 96
			// 
			// i = 0, j = 0, 1, 2, 3..etc : 192.0f + (0 * 64.0f) = 0 for the first 16 cols then
			// i = 1, j = 0, 1, 2, 3..etc : 192.0f + (1 * 64.0f) = 192 for the first 16 cols then

			// Set the first position of x at 32 pixels and for each subsequent + 64 (j * TileSize)
			// Set the first position of y at 192 pixels and until all the x's are drawn and then the next y is prev y + 64 (256) and so forth
			mTiles[i][j]->SetPosition(Vector2((TileSize / 2.0f) + (j * TileSize), StartY + i * TileSize));
		}
	}

	// Set start/end tiles
	// get the start tile mTiles[3][0] and set it as the EStart and apply the appropriate texture, similar for EBase 
	GetStartTile()->SetTileState(Tile::EStart);
	GetEndTile()->SetTileState(Tile::EBase);

	// Set up adjacency lists
	// Basically lists the tiles that are adjacent to it
	// tile[0][0] is adjacent to tile[0][1] and tile[0][1]
	// tile[0][1] is adjacent to tile[0][0] and tile[1][1] and tile[0][2]
	for (size_t i = 0; i < NumRows; i++)
	{
		for (size_t j = 0; j < NumCols; j++)
		{
			if (i > 0)
			{
				mTiles[i][j]->mAdjacent.push_back(mTiles[i - 1][j]);
				std::cout << "mTiles[" << i << "][" << j << "]-->adjacent to: " << "mTiles[" << i - 1 << "][" << j << "]" << "\n";
			}
			if (i < NumRows - 1)
			{
				mTiles[i][j]->mAdjacent.push_back(mTiles[i + 1][j]);
				std::cout << "mTiles[" << i << "][" << j << "]-->adjacent to: " << "mTiles[" << i + 1 << "][" << j << "]" << "\n";
			}
			if (j > 0)
			{
				mTiles[i][j]->mAdjacent.push_back(mTiles[i][j - 1]);
				std::cout << "mTiles[" << i << "][" << j << "]-->adjacent to: " << "mTiles[" << i << "][" << j - 1 << "]" << "\n";
			}
			if (j < NumCols - 1)
			{
				mTiles[i][j]->mAdjacent.push_back(mTiles[i][j + 1]);
				std::cout << "mTiles[" << i << "][" << j << "]-->adjacent to: " << "mTiles[" << i << "][" << j + 1 << "]" << "\n";
			}
		}
	}

	// Find Path in reverse
	FindPath(GetEndTile(), GetStartTile());
	UpdatePathTiles(GetStartTile());

	mNextEnemy = EnemyTime;
}

void Grid::SelectTile(size_t row, size_t col)
{
	// Make sure it's a valid selection
	Tile::TileState tstate = mTiles[row][col]->GetTileState();
	if (tstate != Tile::EStart && tstate != Tile::EBase)
	{
		// Deselect previous one
		if (mSelectedTile)
		{
			mSelectedTile->ToggleSelect();
		}

		// This code stores the current selected tile
		// Then goes into the ToggleSelect from Tile and then sets it new toggle state while updating the texture
		mSelectedTile = mTiles[row][col];
		mSelectedTile->ToggleSelect();
	}
}

void Grid::ProcessClick(int x, int y)
{
	y -= static_cast<int>(StartY - TileSize / 2);
	if (y >= 0)
	{
		x /= static_cast<int>(TileSize);
		y /= static_cast<int>(TileSize);
		if (x >= 0 && x < static_cast<int>(NumCols) && y >= 0 && y < static_cast<int>(NumRows))
		{
			SelectTile(y, x);
		}
	}
}

// Implements A* pathfinding
bool Grid::FindPath(Tile* start, Tile* goal)
{
	for (size_t i = 0; i < NumRows; i++)
	{
		for (size_t j = 0; j < NumCols; j++)
		{
			// g denotes the path-cost of node x
			mTiles[i][j]->g = 0.0f;
			// mInOpenSet is a node that is under consideration
			// mClosedSet is a node that has already been evaluated (will not be evaluated further)
			mTiles[i][j]->mInOpenSet = false;
			mTiles[i][j]->mInClosedSet = false;
		}
	}

	std::vector<Tile*> openSet;

	// Set current node to start, and add to closed set (remember we are starting from goal as start to start as goal)
	Tile* current = start;
	current->mInClosedSet = true;

	do
	{
		// Add adjacent nodes to open set
		for (Tile* neighbor : current->mAdjacent)
		{
			if (neighbor->mBlocked)
			{
				continue;
			}

			// Only check nodes that aren't in the closed set
			if (!neighbor->mInClosedSet)
			{
				if (!neighbor->mInClosedSet)
				{
					// Not in the open set, so set parent
					neighbor->mParent = current;
					// estimated cost from this node to the goal node
					std::cout << "my x position" << neighbor->GetPosition().x << "goal position" << goal->GetPosition().x << "\n";
					std::cout << "my y position" << neighbor->GetPosition().y << "goal position" << goal->GetPosition().y << "\n";
					std::cout <<" estimated cost " << (neighbor->GetPosition() - goal->GetPosition()).Length() << "\n";
					neighbor->h = (neighbor->GetPosition() - goal->GetPosition()).Length();
					// g(x) is the parent's g plus cost of traversing edge
					neighbor->g = current->g + TileSize;
					neighbor->f = neighbor->g + neighbor->h;
					openSet.emplace_back(neighbor);
					neighbor->mInOpenSet = true;
				}
				else
				{
					// Compute g(x) cost if current becomes the parent
					float newG = current->g + TileSize;
					if (newG < neighbor->g)
					{
						// Adopt this node
						neighbor->mParent = current;
						neighbor->g = newG;
						// f(x) changes because g(x) changes
						neighbor->f = neighbor->g + neighbor->h;
					}
				}
			}
		}

		// If open set is empty all possible paths are exhausted
		if (openSet.empty())
		{
			break;
		}

		// Find lowest cost node in open set
		auto iter = std::min_element(openSet.begin(), openSet.end(),
									[](Tile* a, Tile* b) {
										return a->f < b->f;
									});

		// Set to current and move from open to closed
		current = *iter;
		openSet.erase(iter);
		current->mInOpenSet = false;
		current->mInClosedSet = true;
	}
	while (current != goal);
	
	// Did we find a path?
	return (current == goal) ? true : false;
}

void Grid::UpdatePathTiles(class Tile* start)
{
	// Reset all tiles to normal (except for start/end)
	for (size_t i = 0; i < NumRows; i++)
	{
		for (size_t j = 0; j < NumCols; j++)
		{
			if (!(i == 3 && j == 0) && !(i == 3 && j == 15))
			{
				mTiles[i][j]->SetTileState(Tile::EDefault);
			}
		}
	}
	Tile* t = start->mParent;
	while (t != GetEndTile())
	{
		t->SetTileState(Tile::EPath);
		t = t->mParent;
	}
}

void Grid::BuildTower()
{
	if (mSelectedTile && !mSelectedTile->mBlocked)
	{
		mSelectedTile->mBlocked = true;
		if (FindPath(GetEndTile(), GetStartTile()))
		{
			Tower* t = new Tower(GetGame());
			t->SetPosition(mSelectedTile->GetPosition());
		}
		else
		{
			// This tower would block the path
			mSelectedTile->mBlocked = false;
			FindPath(GetEndTile(), GetStartTile());
		}
		UpdatePathTiles(GetStartTile());
	}
}

Tile* Grid::GetStartTile()
{
	return mTiles[3][0];
}

Tile* Grid::GetEndTile()
{
	return mTiles[3][15];
}

void Grid::UpdateActor(float deltaTime)
{
	Actor::UpdateActor(deltaTime);

	// Is it time to spawn a new enemy?
	mNextEnemy -= deltaTime;
	if (mNextEnemy <= 0.0f)
	{
		new Enemy(GetGame());
		mNextEnemy += EnemyTime;
	}
}