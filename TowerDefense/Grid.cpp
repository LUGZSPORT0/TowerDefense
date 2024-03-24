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
				//std::cout << "mTiles[" << i << "][" << j << "]-->adjacent to: " << "mTiles[" << i - 1 << "][" << j << "]" << "\n";
			}
			if (i < NumRows - 1)
			{
				mTiles[i][j]->mAdjacent.push_back(mTiles[i + 1][j]);
				//std::cout << "mTiles[" << i << "][" << j << "]-->adjacent to: " << "mTiles[" << i + 1 << "][" << j << "]" << "\n";
			}
			if (j > 0)
			{
				mTiles[i][j]->mAdjacent.push_back(mTiles[i][j - 1]);
				//std::cout << "mTiles[" << i << "][" << j << "]-->adjacent to: " << "mTiles[" << i << "][" << j - 1 << "]" << "\n";
			}
			if (j < NumCols - 1)
			{
				mTiles[i][j]->mAdjacent.push_back(mTiles[i][j + 1]);
				//std::cout << "mTiles[" << i << "][" << j << "]-->adjacent to: " << "mTiles[" << i << "][" << j + 1 << "]" << "\n";
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
				// Not in the open set, so set parent
				neighbor->mParent = current;
				//std::cout << "my parent is: (" << static_cast<int>(current->GetPosition().x / 64) << ", " << static_cast<int>(current->GetPosition().y / 64) - 3 << ")\n";
				//std::cout << "my position is: (" << static_cast<int>(neighbor->GetPosition().x / 64) << ", " << static_cast<int>(neighbor->GetPosition().y / 64) - 3 << ")\n";
				//std::cout << "goal position is: (" << static_cast<int>(goal->GetPosition().x / 64) << ", " << static_cast<int>(goal->GetPosition().y / 64) - 3 << ")\n";
				// estimated cost from this node to the goal node
				neighbor->h = static_cast<int>((neighbor->GetPosition() - goal->GetPosition()).Length());
				//std::cout <<" estimated cost h(x) " << neighbor->h << "\n";
				// g(x) is the parent's g plus cost of traversing edge
				neighbor->g = current->g + TileSize;
				//std::cout << " cost g(x) " << (neighbor->g) << "\n";
				// sum of the g path-cost and heuristic
				neighbor->f = neighbor->g + neighbor->h;
				//std::cout << " cost f(x) " << (neighbor->f) << "\n";
				openSet.emplace_back(neighbor);
				neighbor->mInOpenSet = true;
				//std::cout << "\n";
			}
			else
			{
				// Compute g(x) cost if current becomes the parent
				// this is in the closed set but we check to see if the g is less 
				float newG = current->g + TileSize;
				//std::cout << "my parent is: (" << static_cast<int>(current->GetPosition().x / 64) << ", " << static_cast<int>(current->GetPosition().y / 64) - 3 << ")\n";
				//std::cout << "my position is: (" << static_cast<int>(neighbor->GetPosition().x / 64) << ", " << static_cast<int>(neighbor->GetPosition().y / 64) - 3 << ")\n";
				//std::cout << "goal position is: (" << static_cast<int>(goal->GetPosition().x / 64) << ", " << static_cast<int>(goal->GetPosition().y / 64) - 3 << ")\n";
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

		// If open set is empty all possible paths are exhausted
		if (openSet.empty())
		{
			break;
		}

		// Find lowest cost node in open set
		auto iter = std::min_element(openSet.begin(), openSet.end(),
									// lambda expression [] is the capture clause (optional), since its empty it doesn't capture any variables by value or reference from the surrounding scope
									// (Tile* a, Tile* b) is the parameter list
									// return true or false based on return results	
									// 
									// calling the std::min_element iterates through the openSet, for each pair of element it calls the lambda function to compare their f values
									// the iterator pointing to the element with min f is stored in the iter variable	
									// 
									// this may not find the right node since there could be a lot ties							
									[](Tile* a, Tile* b) {
										
										return a->f < b->f;
									});
						
		// Set to current and move from open to closed
		// we then set current to the lowest cost node
		// we remove that node from openset vector and change it from open to closed

		current = *iter;
		openSet.erase(iter);
		current->mInOpenSet = false;
		current->mInClosedSet = true;
		//std::cout << "current nodes position is now : (" << static_cast<int>(current->GetPosition().x / 64) << ", " << static_cast<int>(current->GetPosition().y / 64) - 3 << ")\n";
	}
	while (current != goal);
	
	// Did we find a path?
	// after reaching the end we will go through the lowest cost node in open set that we haven't evealuated and pick one of the very first ones
	// as we go through them we will evaluate each neighbor and set open to the ones we previously set to closed (assuming they are no longer on the path)
	// we essentially restart the path finding
	return (current == goal) ? true : false;
}

void Grid::UpdatePathTiles(class Tile* start)
{
	// Reset all tiles to normal (except for start/end)
	for (size_t i = 0; i < NumRows; i++)
	{
		for (size_t j = 0; j < NumCols; j++)
		{	
			int yStartPos = ((Grid::GetStartTile()->GetPosition().y) / 64) - 3;
			int xStartPos = ((Grid::GetStartTile()->GetPosition().x) / 64);			
			int yEndPos = ((Grid::GetEndTile()->GetPosition().y) / 64) - 3;
			int xEndPos = ((Grid::GetEndTile()->GetPosition().x) / 64);
			if (!(i == yStartPos && j == xStartPos) && !(i == yEndPos && j == xEndPos))
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
	return mTiles[2][0];
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