#include "Game/Player.hpp"



const int MAXIMUM_HEALTH = 10;



Player::Player(const Vector3& playerPosition, uint8_t playerPhysicsMode) :
Entity(playerPosition, 0.6f, 0.6f, 1.86f),
m_HighlightedFaceBottomLeft(Vector3::ZERO),
m_HighlightedFaceBottomRight(Vector3::ZERO),
m_HighlightedFaceTopLeft(Vector3::ZERO),
m_HighlightedFaceTopRight(Vector3::ZERO),
m_CurrentPhysicsMode(playerPhysicsMode),
m_IsAlive(true)
{
	m_ForwardXY = Vector3::ZERO;
	m_LeftXY = Vector3::ZERO;
	m_ForwardXYZ = Vector3::ZERO;
	m_raycastStartPosition = Vector3::ZERO;
	m_raycastEndPosition = Vector3::ZERO;
	m_RateOfFire = 0.0f;
	m_Health = MAXIMUM_HEALTH;

	BuildEntityVertices(RGBA::WHITE);
	m_EntityMesh = new Mesh(m_EntityVertices, m_EntityIndices, NUMBER_OF_ENTITY_VERTICES, NUMBER_OF_ENTITY_INDICES);
}



void Player::UpdateBoundingPoints()
{
	m_BoundingPoints[0] = BottomSouthWestPoint();
	m_BoundingPoints[1] = BottomSouthEastPoint();
	m_BoundingPoints[2] = BottomNorthEastPoint();
	m_BoundingPoints[3] = BottomNorthWestPoint();

	m_BoundingPoints[4] = MiddleSouthWestPoint();
	m_BoundingPoints[5] = MiddleSouthEastPoint();
	m_BoundingPoints[6] = MiddleNorthEastPoint();
	m_BoundingPoints[7] = MiddleNorthWestPoint();

	m_BoundingPoints[8] = TopSouthWestPoint();
	m_BoundingPoints[9] = TopSouthEastPoint();
	m_BoundingPoints[10] = TopNorthEastPoint();
	m_BoundingPoints[11] = TopNorthWestPoint();
}



const Vector3* Player::GetBoundingPoints() const
{
	return m_BoundingPoints;
}



void Player::SetAlive(bool alive)
{
	m_IsAlive = alive;
}



bool Player::IsAlive()
{
	return m_IsAlive;
}



void Player::ResetHealth()
{
	m_Health = MAXIMUM_HEALTH;
}



float Player::GetHealthPercentage()
{
	return (static_cast<float>(m_Health) / static_cast<float>(MAXIMUM_HEALTH));
}



void Player::Move(uint8_t controllerNumber, float deltaTimeInSeconds, bool playerIsOnTheGround)
{
	Vector3 playerAcceleration = Vector3::ZERO;

	float moveDisplacement = MOVEMENT_SPEED;
	float fastMoveDisplacement = moveDisplacement * SPEED_MULTIPLIER;

	Vector2 leftStickPosition = InputSystem::SingletonInstance()->AnalogStickCartesianPosition(controllerNumber, LEFT_STICK);

	if (m_CurrentPhysicsMode == WALKING_MODE)
	{
		if (InputSystem::SingletonInstance()->TriggerPosition(controllerNumber, LEFT_TRIGGER) >= 1.0f)
		{
			playerAcceleration += m_LeftXY * -leftStickPosition.X * fastMoveDisplacement;
			playerAcceleration += m_ForwardXY * leftStickPosition.Y * fastMoveDisplacement;
		}
		else
		{
			playerAcceleration += m_LeftXY * -leftStickPosition.X * moveDisplacement;
			playerAcceleration += m_ForwardXY * leftStickPosition.Y * moveDisplacement;
		}

		if (InputSystem::SingletonInstance()->ButtonWasJustPressed(controllerNumber, A_BUTTON))
		{
			if (playerIsOnTheGround)
			{
				m_Velocity.Z += JUMP_IMPULSE;
			}
		}

		playerAcceleration.Z = STANDARD_GRAVITY;

		m_Velocity += playerAcceleration * deltaTimeInSeconds;
		m_Velocity.X = m_Velocity.X * COEFFICIENT_OF_FRICTION;
		m_Velocity.Y = m_Velocity.Y * COEFFICIENT_OF_FRICTION;
	}
	else if (m_CurrentPhysicsMode == FLYING_MODE)
	{
		if (InputSystem::SingletonInstance()->TriggerPosition(controllerNumber, LEFT_TRIGGER) >= 1.0f)
		{
			playerAcceleration += m_LeftXY * -leftStickPosition.X * fastMoveDisplacement;
			playerAcceleration += m_ForwardXY * leftStickPosition.Y * fastMoveDisplacement;
		}
		else
		{
			playerAcceleration += m_LeftXY * -leftStickPosition.X * moveDisplacement;
			playerAcceleration += m_ForwardXY * leftStickPosition.Y * moveDisplacement;
		}

		if (InputSystem::SingletonInstance()->ButtonIsHeldDown(controllerNumber, A_BUTTON))
		{
			if (InputSystem::SingletonInstance()->TriggerPosition(controllerNumber, LEFT_TRIGGER) >= 1.0f)
			{
				playerAcceleration.Z += fastMoveDisplacement;
			}
			else
			{
				playerAcceleration.Z += moveDisplacement;
			}
		}
		if (InputSystem::SingletonInstance()->ButtonIsHeldDown(controllerNumber, B_BUTTON))
		{
			if (InputSystem::SingletonInstance()->TriggerPosition(controllerNumber, LEFT_TRIGGER) >= 1.0f)
			{
				playerAcceleration.Z -= fastMoveDisplacement;
			}
			else
			{
				playerAcceleration.Z -= moveDisplacement;
			}
		}

		m_Velocity += playerAcceleration * deltaTimeInSeconds;
		m_Velocity = m_Velocity * COEFFICIENT_OF_FRICTION;
	}
	else if (m_CurrentPhysicsMode == NO_CLIP_MODE)
	{
		if (InputSystem::SingletonInstance()->TriggerPosition(controllerNumber, LEFT_TRIGGER) >= 1.0f)
		{
			m_Position += m_LeftXY * -leftStickPosition.X * fastMoveDisplacement * deltaTimeInSeconds;
			m_Position += m_ForwardXY * leftStickPosition.Y * fastMoveDisplacement * deltaTimeInSeconds;
		}
		else
		{
			m_Position += m_LeftXY * -leftStickPosition.X * moveDisplacement * deltaTimeInSeconds;
			m_Position += m_ForwardXY * leftStickPosition.Y * moveDisplacement * deltaTimeInSeconds;
		}

		if (InputSystem::SingletonInstance()->ButtonIsHeldDown(controllerNumber, A_BUTTON))
		{
			if (InputSystem::SingletonInstance()->TriggerPosition(controllerNumber, LEFT_TRIGGER) >= 1.0f)
			{
				m_Position.Z += fastMoveDisplacement * deltaTimeInSeconds;
			}
			else
			{
				m_Position.Z += moveDisplacement * deltaTimeInSeconds;
			}
		}
		if (InputSystem::SingletonInstance()->ButtonIsHeldDown(controllerNumber, B_BUTTON))
		{
			if (InputSystem::SingletonInstance()->TriggerPosition(controllerNumber, LEFT_TRIGGER) >= 1.0f)
			{
				m_Position.Z -= fastMoveDisplacement * deltaTimeInSeconds;
			}
			else
			{
				m_Position.Z -= moveDisplacement * deltaTimeInSeconds;
			}
		}
	}
}



void Player::Update()
{
	DefineBoundingBox();
	UpdateBoundingPoints();
}