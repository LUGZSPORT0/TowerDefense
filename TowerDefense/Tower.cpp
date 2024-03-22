#include "Tower.h"
#include "SpriteComponent.h"
#include "MoveComponent.h"
#include "Game.h"
#include "Enemy.h"
#include "Bullet.h"

Tower::Tower(class Game* game)
	:Actor(game)
{
	SpriteComponent* sc = new SpriteComponent(this);
	sc->SetTexture(game->GetTexture("Assets/Tower.png"));

	mMove = new MoveComponent(this);
	// this makes the tower rotate
	//mMove->SetAngularSpeed(Math::Pi);

	mNextAttack = AttackTime;
}

void Tower::UpdateActor(float deltaTime)
{
	Actor::UpdateActor(deltaTime);

	mNextAttack -= deltaTime;
	if (mNextAttack <= 0.0f)
	{
		Enemy* e = GetGame()->GetNearestEnemy(GetPosition());
		if (e != nullptr)
		{
			// Vector from me to Enemy is Enemy minus me
			Vector2 dir = e->GetPosition() - GetPosition();
			// Determine distance: Length -- we want the magnitude as well! So don't normalize
			// take square root of the sum of the squares of each component:
			// sqroot of (dir.x * dir.x + dir.y + dir.y)
			float dist = dir.Length();
			if (dist < AttackRange)
			{
				// Rotate to face enemy	
				// take the arctangent of the forward vector: y is negative since -y is up in sdl
				SetRotation(Math::Atan2(-dir.y, dir.x));
				// Spawn bullet at tower postion facing enemy
				// the bullet inherits the Position of the tower and its rotation
				// it sets at the x, y position and then comes out from where the tower is newly positioned
				Bullet* b = new Bullet(GetGame());
				b->SetPosition(GetPosition());
				b->SetRotation(GetRotation());
			}
		}
		mNextAttack += AttackTime;
	}
}
