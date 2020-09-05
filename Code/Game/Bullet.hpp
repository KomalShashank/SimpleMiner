#pragma once

#include "Game/GameCommons.hpp"
#include "Game/Entity.hpp"



enum BulletType : uint8_t
{
	PLAYER_BULLET_TYPE,
	ENEMY_BULLET_TYPE,
	INVALID_BULLET_TYPE = 255U
};



class Bullet : public Entity
{
public:
	Bullet(const Vector3& firingPosition, const Vector3& directionOfFire, uint8_t bulletType);

	void FireBullet(float deltaTimeInSeconds);
	bool IsOfType(uint8_t bulletType);
	float GetAge();

	void Update();

private:
	uint8_t m_BulletType;
	float m_Age;
};