#pragma once

#include "Game/GameCommons.hpp"
#include "Game/Chunk.hpp"
#include "Game/BlockInfo.hpp"
#include "Game/Player.hpp"
#include "Game/Enemy.hpp"
#include "Game/Bullet.hpp"



enum PlayerID : uint8_t
{
	PLAYER_ONE,
	PLAYER_TWO,
	NUMBER_OF_PLAYERS,
	INVALID_PLAYER = 255U
};



struct RaycastResult3D
{
	BlockInfo m_currentBlockInfo;
	BlockInfo m_previousBlockInfo;
	bool m_impactedSolidBlock;
	Vector3 m_impactedPosition;
	float m_impactFraction;
	Vector3 m_surfaceNormal;

	RaycastResult3D() :
	m_currentBlockInfo(BlockInfo()),
	m_previousBlockInfo(BlockInfo()),
	m_impactedSolidBlock(false),
	m_impactedPosition(Vector3::ZERO),
	m_impactFraction(1.0f),
	m_surfaceNormal(Vector3::ZERO)
	{

	}
};



class World
{
public:
	World();
	~World();

	IntVector2 GetChunkCoordinatesForPlayerPosition(uint8_t currentPlayerID) const;
	RaycastResult3D GetAWRaycast(const Vector3& startPosition, const Vector3& endPosition);

	void DrawRaycastLines(const Player* currentPlayer, const RaycastResult3D& raycastResult) const;

	void Update(float deltaTimeInSeconds);
	void RenderWorldFromCamera(const Camera3D* playerCamera, uint8_t currentPlayerID, const Vector4& clippingPlane = Vector4::ZERO) const;
	void RenderWaterFromCamera(const Camera3D* playerCamera, Texture** colorTargetTextures, Texture* depthStencilTexture, float nearDistance, float farDistance) const;
	void RenderSkyboxFromCamera(const Camera3D* playerCamera) const;
	void RenderAllEntities() const;

private:
	void OnePlayerUpdateCall(float deltaTimeInSeconds);
	void TwoPlayerUpdateCall(float deltaTimeInSeconds);

	void OnePlayerRenderCall() const;
	void TwoPlayerRenderCall() const;

	static void ProcessChunkManagement(void*);
	void UpdateChunkManagement();
	void UpdateBlockManagement(Player* currentPlayer, uint8_t currentPlayerID, RaycastResult3D& raycastResult);

	void UpdateAllEnemies(float deltaTimeInSeconds);
	void RenderAllEnemies() const;
	void DestroyDeadEnemies();
	void DestroyAllExistingEnemies();

	void UpdateAllBullets(float deltaTimeInSeconds);
	void RenderAllBullets() const;
	void DestroyAllOutdatedAndImpactedBullets();
	bool BulletImpactedPlayer(Bullet* currentBullet);
	bool BulletImpactedEnemy(Bullet* currentBullet);

	void IdentifyHighlightedFace(Player* currentPlayer, const RaycastResult3D& raycastResult);
	void DrawHighlightedFace(const Player* currentPlayer, const RaycastResult3D& raycastResult) const;
	bool ChunkIsInViewOfCamera(const Camera3D* playerCamera, const Vector3& chunkWorldMinimums) const;

	void CreateSkyboxMesh();

	Chunk* CreateChunk(const IntVector2& chunkCoordinates);
	void DestroyChunk(Chunk* currentChunk);

	void ActivateNearestMissingChunk();
	bool FindNearestMissingChunk(IntVector2& chunkCoordinates, float& lowestPossibleSquaredDistance, const Player* currentPlayer);
	void ConnectChunk(Chunk* currentChunk);

	void DeactivateFarthestChunk();
	bool FindFarthestChunk(IntVector2& chunkCoordinates);
	void DisconnectChunk(Chunk* currentChunk);

	void PopulateLoadedChunks();

	bool AddToSharedLoadedChunkProxies(const ChunkProxy& currentChunkProxy);
	bool AddToSharedSavedChunkProxies(const ChunkProxy& currentChunkProxy);

	Chunk* FindActiveChunkWithCoordinates(const IntVector2& chunkCoordinates) const;

	void CalculateLightForChunk(Chunk* currentChunk);
	void MarkLightingAsDirty(BlockInfo currentBlockInfo);

	int CalculateIdealLightForBlock(BlockInfo currentBlockInfo);
	int GetPropagatedLightFromNeighbour(BlockInfo currentBlockInfo);

	void UpdateLighting();
	void UpdateLightForBlock(BlockInfo currentBlockInfo);

	void DirtyNeighboursOnLightChange(BlockInfo currentBlockInfo);

	void PlaceBlock(const RaycastResult3D& raycastResult, uint8_t controllerNumber, uint8_t selectedBlock);
	void DigBlock(const RaycastResult3D& raycastResult, uint8_t controllerNumber);

	void ModifyNeighbourChunksForEdgeBlocks(BlockInfo currentBlockInfo);

	void RecalculateLightUponPlacing(BlockInfo currentBlockInfo);
	void RecalculateLightUponDigging(BlockInfo currentBlockInfo);

	void CreateAndLoadPlayer(Player*& currentPlayer, Camera3D* playerCamera, const char* currentPlayerFileName);
	void SaveAndDestroyPlayer(Player*& currentPlayer, const Camera3D* playerCamera, const char* currentPlayerFileName);

	void TogglePhysicsMode(uint8_t currentPlayerID, Player* currentPlayer);
	bool PlayerIsOnTheGround(Player* currentPlayer);

	void ApplyPreventativePhysics(Player* currentPlayer);
	void ApplyCorrectivePhysics(Player* currentPlayer);
	Vector3 GetShortestCorrection(const AABB3& playerBounds, const AABB3& blockBoundingBox);
	void UpdatePlayerMovementAndPhysics(Player* currentPlayer, uint8_t currentPlayerID, float deltaTimeInSeconds);

	void PlayPlayerDeathScream(float panLevel);

	void SpawnNewEnemy(float& enemySpawnTime);
	void DespawnOutdatedEnemies();
	bool PlayerIsInSight(const Vector3& startPosition, const Vector3& endPosition);
	bool FocusOnNearestVisiblePlayer(Enemy* currentEnemy);

	void FirePlayerBullets(Player* currentPlayer, uint8_t currentPlayerID, float deltaTimeInSeconds);
	void FireEnemyBullets(Enemy* currentEnemy, float deltaTimeInSeconds);

	IntVector2 GetChunkCoordinatesForWorldCoordinates(const Vector3& worldCoordinates) const;
	Vector2 GetChunkWorldCentreForChunkCoordinates(const IntVector2& chunkCoordinates);

	Vector3 GetBlockWorldCentreForBlockInfo(BlockInfo currentBlockInfo);

	Chunk* GetChunkAtWorldCoordinates(const Vector3& worldCoordinates);
	BlockInfo GetBlockInfoForWorldCoordinates(const Vector3& blockWorldCoordinates);
	Block* GetBlockForWorldCoordinates(const Vector3& blockWorldCoordinates);
	float GetColumnHeightForWorldXYCoordinates(const Vector2& worldXYCoordinates);

	AABB3 GetBoundingBoxForBlockInfo(BlockInfo currentBlockInfo);
	void GetPlayerOverlappingBlocks(Player* currentPlayer, BlockInfo* overlappingBlocks, size_t& numberOfOverlappingBlocks);
	bool PlayerOverlapsSelectedBlock(BlockInfo selectedBlockInfo, const BlockInfo* overlappingBlocks, size_t numberOfOverlappingBlocks);

	void InitializeAllLights();
	void CreatePlayerLights();
	void CreateSunLight();

	void DestroyAllLights();

	void UpdatePlayerLights();
	void UpdatePlayerAngleToSun(float deltaTimeInSeconds);
	void UpdateSunLight(uint8_t currentPlayerID) const;
	void SetLightDataToShaderProgram(const Camera3D* playerCamera) const;

public:
	ObjectPool<Chunk> m_ChunkPool;
	Chunk* m_AllChunks[MAXIMUM_NUMBER_OF_CHUNKS] = { nullptr };
	size_t m_NumberOfActiveChunks;
	uint8_t m_RLEBuffer[4096U];

	Thread* m_ChunkManagementThread;

	std::deque<BlockInfo> m_DirtyBlocks;
	Light* m_AllWorldLights[MAXIMUM_NUMBER_OF_LIGHTS];

	const SpriteSheet* m_DiffuseSpriteSheet;
	const SpriteSheet* m_NormalSpriteSheet;
	const SpriteSheet* m_SpecularSpriteSheet;
	Material* m_ChunkMaterial;

	const SpriteSheet* m_SkyboxSpriteSheet;
	Mesh* m_SkyboxMesh;
	Material* m_SkyboxMaterial;

	Mesh* m_RaycastLineMesh;
	Mesh* m_HighlightedFaceMesh;

	Material* m_WaterMaterial;

	RaycastResult3D m_PlayerOneRaycastResult;
	RaycastResult3D m_PlayerTwoRaycastResult;

	Player* m_PlayerOne;
	Player* m_PlayerTwo;

	ObjectPool<Enemy> m_EnemyPool;
	Enemy* m_AllEnemies[MAXIMUM_NUMBER_OF_ENEMIES] = { nullptr };
	size_t m_NumberOfAliveEnemies;

	std::set<Bullet*> m_AllBullets;

	float m_PlayerAngleToSun;
	float m_SkyRotation;
	float m_WaterRipple;
};