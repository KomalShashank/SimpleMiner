#pragma once

#include "Game/GameCommons.hpp"
#include "Game/Entity.hpp"



enum PhysicsMode : uint8_t
{
	WALKING_MODE,
	FLYING_MODE,
	NO_CLIP_MODE,
	NUMBER_OF_PHYSICS_MODES,
	INVALID_PHYSICS_MODE = 255U
};



class Player : public Entity
{
public:
	Player(const Vector3& playerPosition, uint8_t playerPhysicsMode);

	void UpdateBoundingPoints();
	const Vector3* GetBoundingPoints() const;

	void SetAlive(bool alive);
	bool IsAlive();

	void ResetHealth();
	float GetHealthPercentage();

	void Move(uint8_t controllerNumber, float deltaTimeInSeconds, bool playerIsOnTheGround);
	void Update();

public:
	Vector3 m_HighlightedFaceBottomLeft;
	Vector3 m_HighlightedFaceBottomRight;
	Vector3 m_HighlightedFaceTopLeft;
	Vector3 m_HighlightedFaceTopRight;

	Vector3 m_BoundingPoints[NUMBER_OF_BOUNDING_POINTS];
	uint8_t m_CurrentPhysicsMode;

private:
	bool m_IsAlive;
};