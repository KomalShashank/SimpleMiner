#pragma once

#include "Game/GameCommons.hpp"
#include "Game/Entity.hpp"



class Enemy : public Entity
{
public:
	Enemy();

	void FocusOnPlayer(const Vector3& playerPosition);
	void Update();
};