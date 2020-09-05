#include "Game/Enemy.hpp"



Enemy::Enemy() :
Entity(Vector3::ZERO, 0.75f, 0.75f, 0.75f)
{
	m_ForwardXYZ = Vector3::ZERO;
	m_RateOfFire = 0.0f;
	m_Health = 5;

	BuildEntityVertices(RGBA::YELLOW);
	m_EntityMesh = new Mesh(m_EntityVertices, m_EntityIndices, NUMBER_OF_ENTITY_VERTICES, NUMBER_OF_ENTITY_INDICES);
}



void Enemy::FocusOnPlayer(const Vector3& playerPosition)
{
	Vector3 displacementToPlayer = playerPosition - m_Position;
	Vector3 directionToPlayer = displacementToPlayer.GetNormalizedVector3();
	m_ForwardXYZ = directionToPlayer;
}



void Enemy::Update()
{
	DefineBoundingBox();
}