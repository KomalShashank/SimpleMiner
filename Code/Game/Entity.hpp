#pragma once

#include "Game/GameCommons.hpp"



const size_t NUMBER_OF_ENTITY_VERTICES = 8U;
const size_t NUMBER_OF_ENTITY_INDICES = 24U;



class Entity
{
public:
	Entity(const Vector3& entityPosition, float entityWidth, float entityLength, float entityHeight);
	virtual ~Entity();

	void BuildEntityIndices();
	void BuildEntityVertices(const RGBA& entityColor);
	void DefineBoundingBox();

	Vector3 BottomSouthWestPoint();
	Vector3 BottomSouthEastPoint();
	Vector3 BottomNorthEastPoint();
	Vector3 BottomNorthWestPoint();

	Vector3 MiddleSouthWestPoint();
	Vector3 MiddleSouthEastPoint();
	Vector3 MiddleNorthEastPoint();
	Vector3 MiddleNorthWestPoint();

	Vector3 TopSouthWestPoint();
	Vector3 TopSouthEastPoint();
	Vector3 TopNorthEastPoint();
	Vector3 TopNorthWestPoint();

	void TakeDamage();
	int GetRemainingHealth();

	AABB3 GetBoundingBox();

	virtual void Update() = 0;
	void Render() const;

public:
	Vector3 m_Position;
	Vector3 m_Velocity;

	Vector3 m_ForwardXY;
	Vector3 m_LeftXY;
	Vector3 m_ForwardXYZ;

	Vector3 m_raycastStartPosition;
	Vector3 m_raycastEndPosition;

	float m_RateOfFire;
	int m_Health;

private:
	Vector3 m_InitialPosition;

	float m_Width;
	float m_Length;
	float m_Height;

	AABB3 m_BoundingBox;

protected:
	Mesh* m_EntityMesh;

	Vertex3D m_EntityVertices[NUMBER_OF_ENTITY_VERTICES];
	uint32_t m_EntityIndices[NUMBER_OF_ENTITY_INDICES];
};



inline Vector3 Entity::BottomSouthWestPoint()
{
	return Vector3(m_Position.X - m_Width / 2, m_Position.Y - m_Length / 2, m_Position.Z - m_Height / 2);
}



inline Vector3 Entity::BottomSouthEastPoint()
{
	return Vector3(m_Position.X + m_Width / 2, m_Position.Y - m_Length / 2, m_Position.Z - m_Height / 2);
}



inline Vector3 Entity::BottomNorthEastPoint()
{
	return Vector3(m_Position.X + m_Width / 2, m_Position.Y + m_Length / 2, m_Position.Z - m_Height / 2);
}



inline Vector3 Entity::BottomNorthWestPoint()
{
	return Vector3(m_Position.X - m_Width / 2, m_Position.Y + m_Length / 2, m_Position.Z - m_Height / 2);
}



inline Vector3 Entity::MiddleSouthWestPoint()
{
	return Vector3(m_Position.X - m_Width / 2, m_Position.Y - m_Length / 2, m_Position.Z);
}



inline Vector3 Entity::MiddleSouthEastPoint()
{
	return Vector3(m_Position.X + m_Width / 2, m_Position.Y - m_Length / 2, m_Position.Z);
}



inline Vector3 Entity::MiddleNorthEastPoint()
{
	return Vector3(m_Position.X + m_Width / 2, m_Position.Y + m_Length / 2, m_Position.Z);
}



inline Vector3 Entity::MiddleNorthWestPoint()
{
	return Vector3(m_Position.X - m_Width / 2, m_Position.Y + m_Length / 2, m_Position.Z);
}



inline Vector3 Entity::TopSouthWestPoint()
{
	return Vector3(m_Position.X - m_Width / 2, m_Position.Y - m_Length / 2, m_Position.Z + m_Height / 2);
}



inline Vector3 Entity::TopSouthEastPoint()
{
	return Vector3(m_Position.X + m_Width / 2, m_Position.Y - m_Length / 2, m_Position.Z + m_Height / 2);
}



inline Vector3 Entity::TopNorthEastPoint()
{
	return Vector3(m_Position.X + m_Width / 2, m_Position.Y + m_Length / 2, m_Position.Z + m_Height / 2);
}



inline Vector3 Entity::TopNorthWestPoint()
{
	return Vector3(m_Position.X - m_Width / 2, m_Position.Y + m_Length / 2, m_Position.Z + m_Height / 2);
}