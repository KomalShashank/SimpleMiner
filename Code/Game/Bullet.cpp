#include "Game/Bullet.hpp"



Bullet::Bullet(const Vector3& firingPosition, const Vector3& directionOfFire, uint8_t bulletType) :
Entity(firingPosition, 0.1f, 0.1f, 0.1f),
m_BulletType(bulletType),
m_Age(0.0f)
{
	m_Velocity = directionOfFire * BULLET_SPEED;

	BuildEntityVertices(RGBA::GREEN);
	m_EntityMesh = new Mesh(m_EntityVertices, m_EntityIndices, NUMBER_OF_ENTITY_VERTICES, NUMBER_OF_ENTITY_INDICES);
}



void Bullet::FireBullet(float deltaTimeInSeconds)
{
	m_Position += m_Velocity * deltaTimeInSeconds;
	m_Age += deltaTimeInSeconds;
}



bool Bullet::IsOfType(uint8_t bulletType)
{
	return m_BulletType == bulletType;
}



float Bullet::GetAge()
{
	return m_Age;
}



void Bullet::Update()
{
	DefineBoundingBox();
}