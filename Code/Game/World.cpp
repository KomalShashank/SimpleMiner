#include "Game/World.hpp"
#include "Game/TheGame.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <thread>



CRITICAL_SECTION g_ChunkLoadingCriticalSection;
CRITICAL_SECTION g_ChunkPopulatingCriticalSection;
CRITICAL_SECTION g_ChunkSavingCriticalSection;

ChunkProxy* g_SharedLoadedChunkProxies = nullptr;
ChunkProxy* g_LocalLoadedChunkProxies = nullptr;
size_t g_NumberOfActivatedChunks = 0U;

ChunkProxy* g_SharedPopulatedChunkProxies = nullptr;
ChunkProxy* g_LocalPopulatedChunkProxies = nullptr;
size_t g_NumberOfPopulatedChunks = 0U;

ChunkProxy* g_SharedSavedChunkProxies = nullptr;
ChunkProxy* g_LocalSavedChunkProxies = nullptr;
size_t g_NumberOfSavedChunks = 0U;

uint8_t g_RLEBuffer[4096U];
bool g_WorldIsActive = false;

const size_t NUMBER_OF_SKYBOX_VERTICES = 24U;
const size_t NUMBER_OF_SKYBOX_INDICES = 36U;



World::World() :
m_PlayerOneRaycastResult(RaycastResult3D()),
m_PlayerTwoRaycastResult(RaycastResult3D()),
m_PlayerOne(nullptr),
m_PlayerTwo(nullptr),
m_NumberOfActiveChunks(0U),
m_NumberOfAliveEnemies(0U),
m_PlayerAngleToSun(90.0f),
m_SkyRotation(0.0f),
m_WaterRipple(0.0f)
{
	CreateAndLoadPlayer(m_PlayerOne, g_TheGame->m_PlayerOneCamera, "PlayerOne.dat");
	if (g_TwoPlayerMode)
	{
		CreateAndLoadPlayer(m_PlayerTwo, g_TheGame->m_PlayerTwoCamera, "PlayerTwo.dat");
	}

	g_SharedLoadedChunkProxies = new ChunkProxy[MAXIMUM_NUMBER_OF_CHUNKS];
	g_LocalLoadedChunkProxies = new ChunkProxy[MAXIMUM_NUMBER_OF_CHUNKS];

	g_SharedPopulatedChunkProxies = new ChunkProxy[MAXIMUM_NUMBER_OF_CHUNKS];
	g_LocalPopulatedChunkProxies = new ChunkProxy[MAXIMUM_NUMBER_OF_CHUNKS];
	
	g_SharedSavedChunkProxies = new ChunkProxy[MAXIMUM_NUMBER_OF_CHUNKS];
	g_LocalSavedChunkProxies = new ChunkProxy[MAXIMUM_NUMBER_OF_CHUNKS];

	size_t proxyArraySize = MAXIMUM_NUMBER_OF_CHUNKS * sizeof(ChunkProxy);
	memset(g_SharedLoadedChunkProxies, 0, proxyArraySize);
	memset(g_LocalLoadedChunkProxies, 0, proxyArraySize);

	memset(g_SharedPopulatedChunkProxies, 0, proxyArraySize);
	memset(g_LocalPopulatedChunkProxies, 0, proxyArraySize);
	
	memset(g_SharedSavedChunkProxies, 0, proxyArraySize);
	memset(g_LocalSavedChunkProxies, 0, proxyArraySize);

	g_WorldIsActive = true;

	InitializeCriticalSection(&g_ChunkLoadingCriticalSection);
	InitializeCriticalSection(&g_ChunkSavingCriticalSection);
	InitializeCriticalSection(&g_ChunkPopulatingCriticalSection);

	m_ChunkManagementThread = Thread::CreateNewThread(ProcessChunkManagement, nullptr);
	m_ChunkManagementThread->DetachThread();

	m_ChunkPool.InitializeObjectPool(MAXIMUM_NUMBER_OF_CHUNKS);
	m_EnemyPool.InitializeObjectPool(MAXIMUM_NUMBER_OF_ENEMIES);

	SamplerData textureSamplerData = SamplerData(REPEAT_WRAP, REPEAT_WRAP, NEAREST_FILTER, NEAREST_FILTER);
	m_DiffuseSpriteSheet = new SpriteSheet("Data/Images/DiffuseTextures.png", 4, 4, textureSamplerData);
	m_NormalSpriteSheet = new SpriteSheet("Data/Images/NormalTextures.png", 4, 4, textureSamplerData);
	m_SpecularSpriteSheet = new SpriteSheet("Data/Images/SpecularTextures.png", 4, 4, textureSamplerData);

	m_ChunkMaterial = new Material("Data/Shaders/ChunkShader.vert", "Data/Shaders/ChunkShader.frag");
	m_ChunkMaterial->SetDiffuseTexture(m_DiffuseSpriteSheet->GetSpriteSheet());
	m_ChunkMaterial->SetNormalTexture(m_NormalSpriteSheet->GetSpriteSheet());
	m_ChunkMaterial->SetSpecularTexture(m_SpecularSpriteSheet->GetSpriteSheet());

	BlockDefinition::InitializeBlockDefinitions(*m_DiffuseSpriteSheet);

	m_SkyboxSpriteSheet = new SpriteSheet("Data/Images/SkyboxTexture.png", 4, 3, textureSamplerData);
	m_SkyboxMaterial = new Material("Data/Shaders/SkyboxShader.vert", "Data/Shaders/SkyboxShader.frag");
	m_SkyboxMaterial->SetDiffuseTexture(m_SkyboxSpriteSheet->GetSpriteSheet());

	CreateSkyboxMesh();

	InitializeAllLights();
	CreatePlayerLights();
	CreateSunLight();

	m_RaycastLineMesh = new Mesh();
	m_HighlightedFaceMesh = new Mesh();

	Texture* waterDistortionTexture = Texture::CreateOrGetTexture("Data/Images/WaterDistortionTexture.png", textureSamplerData);
	m_WaterMaterial = new Material("Data/Shaders/WaterShader.vert", "Data/Shaders/WaterShader.frag");
	m_WaterMaterial->SetDiffuseTexture(waterDistortionTexture);
	m_WaterMaterial->SetFloatToShaderProgram1D("g_WaterLevel", (float*)&WATER_LEVEL);
}



World::~World()
{
	g_WorldIsActive = false;
	
	if (m_NumberOfActiveChunks > 0U)
	{
		for (size_t chunkIndex = 0; chunkIndex < MAXIMUM_NUMBER_OF_CHUNKS; ++chunkIndex)
		{
			Chunk* currentChunk = m_AllChunks[chunkIndex];
			if (currentChunk == nullptr)
			{
				continue;
			}

// 			IntVector2 currentChunkCoordinates = currentChunk->GetChunkCoordinates();
// 
// 			char chunkFilePath[128];
// 			sprintf_s(chunkFilePath, "Data/SaveSlot/Region_0/Chunk_(%i, %i).chunk", currentChunkCoordinates.X, currentChunkCoordinates.Y);
// 
// 			size_t bufferSize = 0U;
// 			currentChunk->CompressToRLEBuffer(m_RLEBuffer, bufferSize);
// 			
// 			BinaryFileWriter fileWriter;
// 			fileWriter.OpenBinaryFile(chunkFilePath);
// 			fileWriter.WriteBytes(m_RLEBuffer, bufferSize);
// 			fileWriter.CloseBinaryFile();

			m_ChunkPool.DeallocateObjectToPool(currentChunk);
			m_AllChunks[chunkIndex] = nullptr;

			--m_NumberOfActiveChunks;
		}
	}

	DeleteCriticalSection(&g_ChunkLoadingCriticalSection);
	DeleteCriticalSection(&g_ChunkSavingCriticalSection);
	DeleteCriticalSection(&g_ChunkPopulatingCriticalSection);

	m_ChunkPool.UninitializeObjectPool();

	DestroyAllExistingEnemies();
	m_EnemyPool.UninitializeObjectPool();

	delete m_WaterMaterial;

	delete g_SharedLoadedChunkProxies;
	delete g_LocalLoadedChunkProxies;

	delete g_SharedPopulatedChunkProxies;
	delete g_LocalPopulatedChunkProxies;
	
	delete g_SharedSavedChunkProxies;
	delete g_LocalSavedChunkProxies;

	delete m_SkyboxMaterial;
	delete m_SkyboxMesh;
	delete m_SkyboxSpriteSheet;

	delete m_ChunkMaterial;
	delete m_RaycastLineMesh;
	delete m_HighlightedFaceMesh;

	delete m_DiffuseSpriteSheet;
	delete m_NormalSpriteSheet;
	delete m_SpecularSpriteSheet;

	DestroyAllLights();

	SaveAndDestroyPlayer(m_PlayerOne, g_TheGame->m_PlayerOneCamera, "PlayerOne.dat");
	if (g_TwoPlayerMode)
	{
		SaveAndDestroyPlayer(m_PlayerTwo, g_TheGame->m_PlayerTwoCamera, "PlayerTwo.dat");
	}
}



IntVector2 World::GetChunkCoordinatesForPlayerPosition(uint8_t currentPlayerID) const
{
	switch (currentPlayerID)
	{
	case PLAYER_ONE:
		return GetChunkCoordinatesForWorldCoordinates(m_PlayerOne->m_Position);

	case PLAYER_TWO:
		return GetChunkCoordinatesForWorldCoordinates(m_PlayerTwo->m_Position);

	default:
		return IntVector2();
	}
}



RaycastResult3D World::GetAWRaycast(const Vector3& startPosition, const Vector3& endPosition)
{
	RaycastResult3D raycastResult;

	BlockInfo previousBlockInfo = GetBlockInfoForWorldCoordinates(startPosition);
	BlockInfo currentBlockInfo = previousBlockInfo;

	BlockInfo endBlockInfo = GetBlockInfoForWorldCoordinates(endPosition);

	if (currentBlockInfo.GetChunk() == nullptr || endBlockInfo.GetChunk() == nullptr)
	{
		raycastResult.m_impactedSolidBlock = false;
		raycastResult.m_impactFraction = 0.0f;

		return raycastResult;
	}

	if (currentBlockInfo.GetChunk() != nullptr)
	{
		if (currentBlockInfo.GetBlock()->IsSolid())
		{
			raycastResult.m_impactedSolidBlock = true;
			raycastResult.m_impactedPosition = startPosition;
			raycastResult.m_impactFraction = 0.0f;
			raycastResult.m_currentBlockInfo = currentBlockInfo;
			raycastResult.m_previousBlockInfo = previousBlockInfo;
			raycastResult.m_surfaceNormal = Vector3::ZERO;

			return raycastResult;
		}
	}

	if (endPosition == startPosition)
	{
		raycastResult.m_impactedSolidBlock = false;
		raycastResult.m_impactFraction = 0.0f;

		return raycastResult;
	}

	float blockX = RoundDownToFloorValue(startPosition.X);
	float blockY = RoundDownToFloorValue(startPosition.Y);
	float blockZ = RoundDownToFloorValue(startPosition.Z);

	Vector3 displacement = endPosition - startPosition;

	float tDeltaX = (displacement.X == 0.0f) ? FLT_MAX : AbsoluteValue(1.0f / displacement.X);
	float tDeltaY = (displacement.Y == 0.0f) ? FLT_MAX : AbsoluteValue(1.0f / displacement.Y);
	float tDeltaZ = (displacement.Z == 0.0f) ? FLT_MAX : AbsoluteValue(1.0f / displacement.Z);

	int blockStepX = (displacement.X > 0.0f) ? 1 : -1;
	int blockStepY = (displacement.Y > 0.0f) ? 1 : -1;
	int blockStepZ = (displacement.Z > 0.0f) ? 1 : -1;

	int offsetToLeadingEdgeX = (blockStepX + 1) / 2;
	int offsetToLeadingEdgeY = (blockStepY + 1) / 2;
	int offsetToLeadingEdgeZ = (blockStepZ + 1) / 2;

	float firstVerticalIntersectionX = blockX + static_cast<float>(offsetToLeadingEdgeX);
	float firstVerticalIntersectionY = blockY + static_cast<float>(offsetToLeadingEdgeY);
	float firstVerticalIntersectionZ = blockZ + static_cast<float>(offsetToLeadingEdgeZ);

	float tOfNextCrossingX = (displacement.X == 0.0f) ? FLT_MAX : AbsoluteValue(firstVerticalIntersectionX - startPosition.X) * tDeltaX;
	float tOfNextCrossingY = (displacement.Y == 0.0f) ? FLT_MAX : AbsoluteValue(firstVerticalIntersectionY - startPosition.Y) * tDeltaY;
	float tOfNextCrossingZ = (displacement.Z == 0.0f) ? FLT_MAX : AbsoluteValue(firstVerticalIntersectionZ - startPosition.Z) * tDeltaZ;

	for (;;)
	{
		float tOfNextCrossing = GetMinimumOfThreeFloats(tOfNextCrossingX, tOfNextCrossingY, tOfNextCrossingZ);

		if (tOfNextCrossing > 1)
		{
			raycastResult.m_impactedSolidBlock = false;
			break;
		}

		if (previousBlockInfo.GetChunk() == nullptr)
		{
			raycastResult.m_impactedSolidBlock = false;
			break;
		}

		if (tOfNextCrossing == tOfNextCrossingX)
		{
			currentBlockInfo = (blockStepX > 0) ? previousBlockInfo.GetEasternNeighbour() : previousBlockInfo.GetWesternNeighbour();

			if (currentBlockInfo.GetChunk() != nullptr)
			{
				if (currentBlockInfo.GetBlock()->IsSolid())
				{
					raycastResult.m_impactedSolidBlock = true;
					raycastResult.m_surfaceNormal = (blockStepX > 0) ? Vector3(-1.0f, 0.0f, 0.0f) : Vector3(1.0f, 0.0f, 0.0f);
					raycastResult.m_impactedPosition = startPosition + (displacement * tOfNextCrossingX) + (raycastResult.m_surfaceNormal * 0.005f);
					raycastResult.m_impactFraction = tOfNextCrossingX;
					raycastResult.m_currentBlockInfo = currentBlockInfo;
					raycastResult.m_previousBlockInfo = previousBlockInfo;
					break;
				}
			}

			previousBlockInfo = currentBlockInfo;
			tOfNextCrossingX += tDeltaX;
		}
		else if (tOfNextCrossing == tOfNextCrossingY)
		{
			currentBlockInfo = (blockStepY > 0) ? previousBlockInfo.GetNorthernNeighbour() : previousBlockInfo.GetSouthernNeighbour();

			if (currentBlockInfo.GetChunk() != nullptr)
			{
				if (currentBlockInfo.GetBlock()->IsSolid())
				{
					raycastResult.m_impactedSolidBlock = true;
					raycastResult.m_surfaceNormal = (blockStepY > 0) ? Vector3(0.0f, -1.0f, 0.0f) : Vector3(0.0f, 1.0f, 0.0f);
					raycastResult.m_impactedPosition = startPosition + (displacement * tOfNextCrossingY) + (raycastResult.m_surfaceNormal * 0.005f);
					raycastResult.m_impactFraction = tOfNextCrossingY;
					raycastResult.m_currentBlockInfo = currentBlockInfo;
					raycastResult.m_previousBlockInfo = previousBlockInfo;
					break;
				}
			}

			previousBlockInfo = currentBlockInfo;
			tOfNextCrossingY += tDeltaY;
		}
		else if (tOfNextCrossing == tOfNextCrossingZ)
		{
			currentBlockInfo = (blockStepZ > 0) ? previousBlockInfo.GetAboveNeighbour() : previousBlockInfo.GetBelowNeighbour();

			if (currentBlockInfo.GetChunk() != nullptr)
			{
				if (currentBlockInfo.GetBlock()->IsSolid())
				{
					raycastResult.m_impactedSolidBlock = true;
					raycastResult.m_surfaceNormal = (blockStepZ > 0) ? Vector3(0.0f, 0.0f, -1.0f) : Vector3(0.0f, 0.0f, 1.0f);
					raycastResult.m_impactedPosition = startPosition + (displacement * tOfNextCrossingZ) + (raycastResult.m_surfaceNormal * 0.005f);
					raycastResult.m_impactFraction = tOfNextCrossingZ;
					raycastResult.m_currentBlockInfo = currentBlockInfo;
					raycastResult.m_previousBlockInfo = previousBlockInfo;
					break;
				}
			}

			previousBlockInfo = currentBlockInfo;
			tOfNextCrossingZ += tDeltaZ;
		}
	}

	return raycastResult;
}



void World::DrawRaycastLines(const Player* currentPlayer, const RaycastResult3D& raycastResult) const
{
	Vector3 impactPosition = (raycastResult.m_impactedSolidBlock) ? raycastResult.m_impactedPosition : currentPlayer->m_raycastEndPosition;

	const size_t NUMBER_OF_LINE_VERTICES = 2;
	const size_t NUMBER_OF_LINE_INDICES = 2;
	
	Vertex3D lineVertices[NUMBER_OF_LINE_VERTICES];
	uint32_t lineIndices[NUMBER_OF_LINE_INDICES] = { 0, 1 };
	
	Vertex3D lineVertex;
	lineVertex.m_Color = RGBA::MAGENTA;

	lineVertex.m_Position = currentPlayer->m_raycastStartPosition;
	lineVertices[0] = lineVertex;

	lineVertex.m_Position = impactPosition;
	lineVertices[1] = lineVertex;

	m_RaycastLineMesh->WriteToMesh(&lineVertices[0], &lineIndices[0], NUMBER_OF_LINE_VERTICES, NUMBER_OF_LINE_INDICES);
	AdvancedRenderer::SingletonInstance()->DrawLinesMesh(m_RaycastLineMesh, NUMBER_OF_LINE_VERTICES, NUMBER_OF_LINE_INDICES, 2.0f);
}



void World::Update(float deltaTimeInSeconds)
{
	(!g_TwoPlayerMode) ? OnePlayerUpdateCall(deltaTimeInSeconds) : TwoPlayerUpdateCall(deltaTimeInSeconds);

	UpdateAllEnemies(deltaTimeInSeconds);
	DestroyDeadEnemies();

	UpdateAllBullets(deltaTimeInSeconds);
	DestroyAllOutdatedAndImpactedBullets();

	UpdateLighting();

	for (Chunk* currentChunk : m_AllChunks)
	{
		if (currentChunk != nullptr)
		{
			if (currentChunk->IsModified())
			{
				currentChunk->RebuildChunkMesh();
				currentChunk->RebuildWaterMesh();
				currentChunk->SetModified(false);
			}
		}
	}

	UpdatePlayerLights();
	UpdatePlayerAngleToSun(deltaTimeInSeconds);

	m_WaterRipple += WATER_RIPPLE_SPEED * deltaTimeInSeconds;
	m_WaterRipple = WrapAroundCircularRange(m_WaterRipple, 0.0f, 1.0f);
}



void World::RenderWorldFromCamera(const Camera3D* playerCamera, uint8_t currentPlayerID, const Vector4& clippingPlane /*= Vector4::ZERO*/) const
{
	UpdateSunLight(currentPlayerID);
	SetLightDataToShaderProgram(playerCamera);

	m_ChunkMaterial->SetFloatToShaderProgram4D("g_ClippingPlane", (float*)&clippingPlane);
	
	for (Chunk* currentChunk : m_AllChunks)
	{
		if (currentChunk != nullptr)
		{
			Vector3 currentChunkWorldMinimums = currentChunk->GetChunkWorldMinimums();

			if (ChunkIsInViewOfCamera(playerCamera, currentChunkWorldMinimums))
			{
				currentChunk->RenderChunk(m_ChunkMaterial);
			}
		}
	}
}



void World::RenderWaterFromCamera(const Camera3D* playerCamera, Texture** colorTargetTextures, Texture* depthStencilTexture, float nearDistance, float farDistance) const
{
	m_WaterMaterial->SetColorTargetTextures(colorTargetTextures, 2U);
	m_WaterMaterial->SetDepthStencilTexture(depthStencilTexture);
	m_WaterMaterial->SetFloatToShaderProgram3D("g_CameraPosition", (float*)&playerCamera->m_Position);
	m_WaterMaterial->SetFloatToShaderProgram1D("g_WaterRipple", (float*)&m_WaterRipple);
	m_WaterMaterial->SetFloatToShaderProgram1D("g_NearDistance", (float*)&nearDistance);
	m_WaterMaterial->SetFloatToShaderProgram1D("g_FarDistance", (float*)&farDistance);
	
	AdvancedRenderer::SingletonInstance()->EnableBackFaceCulling(false);
	
	for (Chunk* currentChunk : m_AllChunks)
	{
		if (currentChunk != nullptr)
		{
			Vector3 currentChunkWorldMinimums = currentChunk->GetChunkWorldMinimums();

			if (ChunkIsInViewOfCamera(playerCamera, currentChunkWorldMinimums))
			{
				currentChunk->RenderWater(m_WaterMaterial);
			}
		}
	}

	AdvancedRenderer::SingletonInstance()->EnableBackFaceCulling(true);
}



void World::RenderSkyboxFromCamera(const Camera3D* playerCamera) const
{
	const Vector3 DAY_SKY_COLOR = Vector3(1.0f, 1.0f, 1.0f);
	const Vector3 NIGHT_SKY_COLOR = Vector3(0.15f, 0.15f, 0.15f);

	Vector3 skyboxTint = Vector3::ZERO;
	if (m_PlayerAngleToSun >= 90.0f && m_PlayerAngleToSun < 270.0f)
	{
		float interpolationFactor = RangeMap(m_PlayerAngleToSun, 90.0f, 270.0f, 0.0f, 1.0f);
		skyboxTint = LinearlyInterpolateIn3D(DAY_SKY_COLOR, NIGHT_SKY_COLOR, interpolationFactor);
	}
	else
	{
		float interpolationFactor = (m_PlayerAngleToSun < 90.0f) ? RangeMap(m_PlayerAngleToSun, -90.0f, 90.0f, 0.0f, 1.0f) : RangeMap(m_PlayerAngleToSun, 270.0f, 450.0f, 0.0f, 1.0f);
		skyboxTint = LinearlyInterpolateIn3D(NIGHT_SKY_COLOR, DAY_SKY_COLOR, interpolationFactor);
	}

	m_SkyboxMaterial->SetFloatToShaderProgram3D("g_SkyboxTint", (float*)&skyboxTint);
	
	Matrix4 modelMatrix = AdvancedRenderer::SingletonInstance()->GetModelMatrix(Vector3::ONE, EulerAngles(0.0f, 0.0f, m_SkyRotation), playerCamera->m_Position);
	AdvancedRenderer::SingletonInstance()->UpdateModelMatrix(modelMatrix);

	AdvancedRenderer::SingletonInstance()->EnableDepthWriting(false);
	AdvancedRenderer::SingletonInstance()->DrawPolygonMesh(m_SkyboxMesh, NUMBER_OF_SKYBOX_VERTICES, NUMBER_OF_SKYBOX_INDICES, m_SkyboxMaterial);
	AdvancedRenderer::SingletonInstance()->EnableDepthWriting(true);

	modelMatrix = AdvancedRenderer::SingletonInstance()->GetModelMatrix(Vector3::ONE, EulerAngles::ZERO, Vector3::ZERO);
	AdvancedRenderer::SingletonInstance()->UpdateModelMatrix(modelMatrix);
}



void World::RenderAllEntities() const
{
	if (g_ScreenshotMode)
	{
		return;
	}

	AdvancedRenderer::SingletonInstance()->EnableDepthWriting(false);

	(!g_TwoPlayerMode) ? OnePlayerRenderCall() : TwoPlayerRenderCall();

	RenderAllEnemies();
	RenderAllBullets();

	if (m_PlayerOne->IsAlive())
	{
		//DrawRaycastLines(m_PlayerOne, m_PlayerOneRaycastResult);
	}

	if (g_TwoPlayerMode)
	{
		if (m_PlayerTwo->IsAlive())
		{
			//DrawRaycastLines(m_PlayerTwo, m_PlayerTwoRaycastResult);
		}
	}

	AdvancedRenderer::SingletonInstance()->EnableDepthWriting(true);
}



void World::OnePlayerUpdateCall(float deltaTimeInSeconds)
{
	if (m_PlayerOne->IsAlive())
	{
		TogglePhysicsMode(PLAYER_ONE, m_PlayerOne);
		UpdatePlayerMovementAndPhysics(m_PlayerOne, PLAYER_ONE, deltaTimeInSeconds);
		FirePlayerBullets(m_PlayerOne, PLAYER_ONE, deltaTimeInSeconds);
	}

	UpdateChunkManagement();

	if (m_PlayerOne->IsAlive())
	{
		UpdateBlockManagement(m_PlayerOne, PLAYER_ONE, m_PlayerOneRaycastResult);
		IdentifyHighlightedFace(m_PlayerOne, m_PlayerOneRaycastResult);
	}
}



void World::TwoPlayerUpdateCall(float deltaTimeInSeconds)
{
	if (m_PlayerOne->IsAlive())
	{
		TogglePhysicsMode(PLAYER_ONE, m_PlayerOne);
		UpdatePlayerMovementAndPhysics(m_PlayerOne, PLAYER_ONE, deltaTimeInSeconds);
		FirePlayerBullets(m_PlayerOne, PLAYER_ONE, deltaTimeInSeconds);
	}

	if (m_PlayerTwo->IsAlive())
	{
		TogglePhysicsMode(PLAYER_TWO, m_PlayerTwo);
		UpdatePlayerMovementAndPhysics(m_PlayerTwo, PLAYER_TWO, deltaTimeInSeconds);
		FirePlayerBullets(m_PlayerTwo, PLAYER_TWO, deltaTimeInSeconds);
	}

	UpdateChunkManagement();

	if (m_PlayerOne->IsAlive())
	{
		UpdateBlockManagement(m_PlayerOne, PLAYER_ONE, m_PlayerOneRaycastResult);
		IdentifyHighlightedFace(m_PlayerOne, m_PlayerOneRaycastResult);
	}

	if (m_PlayerTwo->IsAlive())
	{
		UpdateBlockManagement(m_PlayerTwo, PLAYER_TWO, m_PlayerTwoRaycastResult);
		IdentifyHighlightedFace(m_PlayerTwo, m_PlayerTwoRaycastResult);
	}
}



void World::OnePlayerRenderCall() const
{
	if (m_PlayerOne->IsAlive())
	{
		m_PlayerOne->Render();
		DrawHighlightedFace(m_PlayerOne, m_PlayerOneRaycastResult);
	}
}



void World::TwoPlayerRenderCall() const
{
	if (m_PlayerOne->IsAlive())
	{
		m_PlayerOne->Render();
		DrawHighlightedFace(m_PlayerOne, m_PlayerOneRaycastResult);
	}

	if (m_PlayerTwo->IsAlive())
	{
		m_PlayerTwo->Render();
		DrawHighlightedFace(m_PlayerTwo, m_PlayerTwoRaycastResult);
	}
}



void World::ProcessChunkManagement(void*)
{
	size_t proxyArraySize = MAXIMUM_NUMBER_OF_CHUNKS * sizeof(ChunkProxy);
	
	while (g_WorldIsActive)
	{
		size_t numberOfActivatedChunks = 0U;

		EnterCriticalSection(&g_ChunkLoadingCriticalSection);
		{
			ChunkProxy* temporaryPointer = g_SharedLoadedChunkProxies;
			g_SharedLoadedChunkProxies = g_LocalLoadedChunkProxies;
			g_LocalLoadedChunkProxies = temporaryPointer;

			numberOfActivatedChunks = g_NumberOfActivatedChunks;
			g_NumberOfActivatedChunks = 0U;
		}
		LeaveCriticalSection(&g_ChunkLoadingCriticalSection);
		
		size_t numberOfDeactivatedChunks = 0U;

		EnterCriticalSection(&g_ChunkSavingCriticalSection);
		{
			ChunkProxy* temporaryPointer = g_SharedSavedChunkProxies;
			g_SharedSavedChunkProxies = g_LocalSavedChunkProxies;
			g_LocalSavedChunkProxies = temporaryPointer;

			numberOfDeactivatedChunks = g_NumberOfSavedChunks;
			g_NumberOfSavedChunks = 0U;
		}
		LeaveCriticalSection(&g_ChunkSavingCriticalSection);

		if (numberOfActivatedChunks > 0U)
		{
			size_t numberOfPopulatedChunks = 0U;
			for (size_t chunkProxyIndex = 0; chunkProxyIndex < MAXIMUM_NUMBER_OF_CHUNKS; ++chunkProxyIndex)
			{
				ChunkProxy& currentChunkProxy = g_LocalLoadedChunkProxies[chunkProxyIndex];
				if (!currentChunkProxy.m_IsValid)
				{
					continue;
				}

				IntVector2 currentChunkCoordinates = currentChunkProxy.m_ChunkCoordinates;

				char chunkFilePath[128];
				sprintf_s(chunkFilePath, "Data/SaveSlot/Region_0/Chunk_(%i, %i).chunk", currentChunkCoordinates.X, currentChunkCoordinates.Y);

				BinaryFileReader fileReader;
				bool fileExists = fileReader.OpenBinaryFile(chunkFilePath);

				if (fileExists)
				{
					size_t fileSize = fileReader.GetBinaryFileSize();
					fileSize = fileReader.ReadBytes(g_RLEBuffer, fileSize);

					currentChunkProxy.DecompressFromRLEBuffer(g_RLEBuffer, fileSize);
					fileReader.CloseBinaryFile();
				}
				else
				{
					currentChunkProxy.PopulateFromPerlinNoise();
				}

				++numberOfPopulatedChunks;
			}

			EnterCriticalSection(&g_ChunkPopulatingCriticalSection);
			{
				ChunkProxy* temporaryPointer = g_LocalLoadedChunkProxies;
				g_LocalLoadedChunkProxies = g_SharedPopulatedChunkProxies;
				g_SharedPopulatedChunkProxies = temporaryPointer;

				g_NumberOfPopulatedChunks = numberOfPopulatedChunks;
			}
			LeaveCriticalSection(&g_ChunkPopulatingCriticalSection);
		}

		if (numberOfDeactivatedChunks > 0U)
		{
			for (size_t chunkProxyIndex = 0; chunkProxyIndex < MAXIMUM_NUMBER_OF_CHUNKS; ++chunkProxyIndex)
			{
				ChunkProxy currentChunkProxy = g_LocalSavedChunkProxies[chunkProxyIndex];
				if (!currentChunkProxy.m_IsValid)
				{
					continue;
				}

				IntVector2 currentChunkCoordinates = currentChunkProxy.m_ChunkCoordinates;

				char chunkFilePath[128];
				sprintf_s(chunkFilePath, "Data/SaveSlot/Region_0/Chunk_(%i, %i).chunk", currentChunkCoordinates.X, currentChunkCoordinates.Y);

				size_t bufferSize = 0U;
				currentChunkProxy.CompressToRLEBuffer(g_RLEBuffer, bufferSize);

				BinaryFileWriter fileWriter;
				fileWriter.OpenBinaryFile(chunkFilePath);
				fileWriter.WriteBytes(g_RLEBuffer, bufferSize);
				fileWriter.CloseBinaryFile();
			}

			memset(g_LocalSavedChunkProxies, 0, proxyArraySize);
		}

		Thread::YieldThread();
	}
}



void World::UpdateChunkManagement()
{
	ActivateNearestMissingChunk();
	PopulateLoadedChunks();
	DeactivateFarthestChunk();
}



void World::UpdateBlockManagement(Player* currentPlayer, uint8_t currentPlayerID, RaycastResult3D& raycastResult)
{
	currentPlayer->m_raycastStartPosition = currentPlayer->m_Position + EYE_LEVEL_VIEW;
	currentPlayer->m_raycastEndPosition = currentPlayer->m_raycastStartPosition + (currentPlayer->m_ForwardXYZ * MAXIMUM_PLAYER_RANGE);

	raycastResult = GetAWRaycast(currentPlayer->m_raycastStartPosition, currentPlayer->m_raycastEndPosition);

	if (raycastResult.m_impactedSolidBlock)
	{
		uint8_t selectedBlock = INVALID_BLOCK;
		if (currentPlayerID == PLAYER_ONE)
		{
			selectedBlock = g_TheGame->m_PlayerOneSelectedBlock;
		}
		else if (currentPlayerID == PLAYER_TWO)
		{
			selectedBlock = g_TheGame->m_PlayerTwoSelectedBlock;
		}

		PlaceBlock(raycastResult, currentPlayerID, selectedBlock);
		DigBlock(raycastResult, currentPlayerID);
	}
}



void World::UpdateAllEnemies(float deltaTimeInSeconds)
{
	static float enemySpawnTime = 0.0f;
	enemySpawnTime += deltaTimeInSeconds;

	SpawnNewEnemy(enemySpawnTime);
	DespawnOutdatedEnemies();

	for (Enemy* currentEnemy : m_AllEnemies)
	{
		if (currentEnemy != nullptr)
		{
			bool playerIsInFocus = FocusOnNearestVisiblePlayer(currentEnemy);
			if (playerIsInFocus)
			{
				FireEnemyBullets(currentEnemy, deltaTimeInSeconds);
			}

			currentEnemy->Update();
		}
	}
}



void World::RenderAllEnemies() const
{
	for (Enemy* currentEnemy : m_AllEnemies)
	{
		if (currentEnemy != nullptr)
		{
			currentEnemy->Render();
		}
	}
}



void World::DestroyDeadEnemies()
{
	for (size_t enemyIndex = 0; enemyIndex < MAXIMUM_NUMBER_OF_ENEMIES; ++enemyIndex)
	{
		Enemy* currentEnemy = m_AllEnemies[enemyIndex];
		if (currentEnemy != nullptr)
		{
			if (currentEnemy->GetRemainingHealth() <= 0)
			{
				m_EnemyPool.DeallocateObjectToPool(currentEnemy);
				m_AllEnemies[enemyIndex] = nullptr;

				--m_NumberOfAliveEnemies;
			}
		}
	}
}



void World::DestroyAllExistingEnemies()
{
	for (size_t enemyIndex = 0; enemyIndex < MAXIMUM_NUMBER_OF_ENEMIES; ++enemyIndex)
	{
		Enemy* currentEnemy = m_AllEnemies[enemyIndex];
		if (currentEnemy != nullptr)
		{
			m_EnemyPool.DeallocateObjectToPool(currentEnemy);
			m_AllEnemies[enemyIndex] = nullptr;

			--m_NumberOfAliveEnemies;
		}
	}
}



void World::UpdateAllBullets(float deltaTimeInSeconds)
{
	for (Bullet* currentBullet : m_AllBullets)
	{
		if (currentBullet != nullptr)
		{
			currentBullet->FireBullet(deltaTimeInSeconds);
			currentBullet->Update();
		}
	}
}



void World::RenderAllBullets() const
{
	for (Bullet* currentBullet : m_AllBullets)
	{
		if (currentBullet != nullptr)
		{
			currentBullet->Render();
		}
	}
}



void World::DestroyAllOutdatedAndImpactedBullets()
{
	for (auto currentBulletIterator = m_AllBullets.begin(); currentBulletIterator != m_AllBullets.end();)
	{
		bool bulletHasToBeDestroyed = false;

		Bullet* currentBullet = *currentBulletIterator;
		float currentBulletAge = currentBullet->GetAge();
		BlockInfo currentBulletBlockInfo = GetBlockInfoForWorldCoordinates(currentBullet->m_Position);

		if (currentBulletAge >= MAXIMUM_BULLET_LIFETIME)
		{
			bulletHasToBeDestroyed = true;
		}
		else if (BulletImpactedPlayer(currentBullet))
		{
			bulletHasToBeDestroyed = true;
		}
		else if (BulletImpactedEnemy(currentBullet))
		{
			bulletHasToBeDestroyed = true;
		}
		else if (currentBulletBlockInfo.GetBlock() != nullptr)
		{
			if (currentBulletBlockInfo.GetBlock()->IsSolid())
			{
				bulletHasToBeDestroyed = true;
			}
		}

		if (bulletHasToBeDestroyed)
		{
			currentBulletIterator = m_AllBullets.erase(currentBulletIterator);
			delete currentBullet;
			continue;
		}
		
		++currentBulletIterator;
	}
}



bool World::BulletImpactedPlayer(Bullet* currentBullet)
{
	AABB3 bulletBounds = currentBullet->GetBoundingBox();
	AABB3 playerOneBounds = m_PlayerOne->GetBoundingBox();

	if (currentBullet->IsOfType(ENEMY_BULLET_TYPE))
	{
		if (DoAABB3sIntersect(bulletBounds, playerOneBounds))
		{
			m_PlayerOne->TakeDamage();

			if (m_PlayerOne->GetRemainingHealth() <= 0)
			{
				float panLevel = (g_TwoPlayerMode) ? -1.0f : 0.0f;
				PlayPlayerDeathScream(panLevel);
			}

			return true;
		}

		if (g_TwoPlayerMode)
		{
			AABB3 playerTwoBounds = m_PlayerTwo->GetBoundingBox();
			if (DoAABB3sIntersect(bulletBounds, playerTwoBounds))
			{
				m_PlayerTwo->TakeDamage();

				if (m_PlayerTwo->GetRemainingHealth() <= 0)
				{
					float panLevel = 1.0f;
					PlayPlayerDeathScream(panLevel);
				}

				return true;
			}
		}
	}

	return false;
}



bool World::BulletImpactedEnemy(Bullet* currentBullet)
{
	AABB3 bulletBounds = currentBullet->GetBoundingBox();

	if (currentBullet->IsOfType(PLAYER_BULLET_TYPE))
	{
		for (Enemy* currentEnemy : m_AllEnemies)
		{
			if (currentEnemy != nullptr)
			{
				AABB3 currentEnemyBounds = currentEnemy->GetBoundingBox();
				if (DoAABB3sIntersect(bulletBounds, currentEnemyBounds))
				{
					currentEnemy->TakeDamage();
					return true;
				}
			}
		}
	}

	return false;
}



void World::IdentifyHighlightedFace(Player* currentPlayer, const RaycastResult3D& raycastResult)
{
	if (raycastResult.m_impactedSolidBlock)
	{
		Vector3 blockWorldCentre = GetBlockWorldCentreForBlockInfo(raycastResult.m_currentBlockInfo);
		Vector3 impactedSurfaceNormal = raycastResult.m_surfaceNormal;

		if (impactedSurfaceNormal.X != 0.0f)
		{
			if (impactedSurfaceNormal.X > 0.0f)
			{
				Vector3 bottomLeft = Vector3(blockWorldCentre.X + 0.51f, blockWorldCentre.Y - 0.49f, blockWorldCentre.Z - 0.49f);
				Vector3 bottomRight = Vector3(blockWorldCentre.X + 0.51f, blockWorldCentre.Y + 0.49f, blockWorldCentre.Z - 0.49f);
				Vector3 topLeft = Vector3(blockWorldCentre.X + 0.51f, blockWorldCentre.Y - 0.49f, blockWorldCentre.Z + 0.49f);
				Vector3 topRight = Vector3(blockWorldCentre.X + 0.51f, blockWorldCentre.Y + 0.49f, blockWorldCentre.Z + 0.49f);

				currentPlayer->m_HighlightedFaceBottomLeft = bottomLeft;
				currentPlayer->m_HighlightedFaceBottomRight = bottomRight;
				currentPlayer->m_HighlightedFaceTopLeft = topLeft;
				currentPlayer->m_HighlightedFaceTopRight = topRight;
			}
			else
			{
				Vector3 bottomLeft = Vector3(blockWorldCentre.X - 0.51f, blockWorldCentre.Y + 0.49f, blockWorldCentre.Z - 0.49f);
				Vector3 bottomRight = Vector3(blockWorldCentre.X - 0.51f, blockWorldCentre.Y - 0.49f, blockWorldCentre.Z - 0.49f);
				Vector3 topLeft = Vector3(blockWorldCentre.X - 0.51f, blockWorldCentre.Y + 0.49f, blockWorldCentre.Z + 0.49f);
				Vector3 topRight = Vector3(blockWorldCentre.X - 0.51f, blockWorldCentre.Y - 0.49f, blockWorldCentre.Z + 0.49f);

				currentPlayer->m_HighlightedFaceBottomLeft = bottomLeft;
				currentPlayer->m_HighlightedFaceBottomRight = bottomRight;
				currentPlayer->m_HighlightedFaceTopLeft = topLeft;
				currentPlayer->m_HighlightedFaceTopRight = topRight;
			}
		}
		else if (impactedSurfaceNormal.Y != 0.0f)
		{
			if (impactedSurfaceNormal.Y > 0.0f)
			{
				Vector3 bottomLeft = Vector3(blockWorldCentre.X + 0.49f, blockWorldCentre.Y + 0.51f, blockWorldCentre.Z - 0.49f);
				Vector3 bottomRight = Vector3(blockWorldCentre.X - 0.49f, blockWorldCentre.Y + 0.51f, blockWorldCentre.Z - 0.49f);
				Vector3 topLeft = Vector3(blockWorldCentre.X + 0.49f, blockWorldCentre.Y + 0.51f, blockWorldCentre.Z + 0.49f);
				Vector3 topRight = Vector3(blockWorldCentre.X - 0.49f, blockWorldCentre.Y + 0.51f, blockWorldCentre.Z + 0.49f);

				currentPlayer->m_HighlightedFaceBottomLeft = bottomLeft;
				currentPlayer->m_HighlightedFaceBottomRight = bottomRight;
				currentPlayer->m_HighlightedFaceTopLeft = topLeft;
				currentPlayer->m_HighlightedFaceTopRight = topRight;
			}
			else
			{
				Vector3 bottomLeft = Vector3(blockWorldCentre.X - 0.49f, blockWorldCentre.Y - 0.51f, blockWorldCentre.Z - 0.49f);
				Vector3 bottomRight = Vector3(blockWorldCentre.X + 0.49f, blockWorldCentre.Y - 0.51f, blockWorldCentre.Z - 0.49f);
				Vector3 topLeft = Vector3(blockWorldCentre.X - 0.49f, blockWorldCentre.Y - 0.51f, blockWorldCentre.Z + 0.49f);
				Vector3 topRight = Vector3(blockWorldCentre.X + 0.49f, blockWorldCentre.Y - 0.51f, blockWorldCentre.Z + 0.49f);

				currentPlayer->m_HighlightedFaceBottomLeft = bottomLeft;
				currentPlayer->m_HighlightedFaceBottomRight = bottomRight;
				currentPlayer->m_HighlightedFaceTopLeft = topLeft;
				currentPlayer->m_HighlightedFaceTopRight = topRight;
			}
		}
		else if (impactedSurfaceNormal.Z != 0.0f)
		{
			if (impactedSurfaceNormal.Z > 0.0f)
			{
				Vector3 bottomLeft = Vector3(blockWorldCentre.X - 0.49f, blockWorldCentre.Y - 0.49f, blockWorldCentre.Z + 0.51f);
				Vector3 bottomRight = Vector3(blockWorldCentre.X + 0.49f, blockWorldCentre.Y - 0.49f, blockWorldCentre.Z + 0.51f);
				Vector3 topLeft = Vector3(blockWorldCentre.X - 0.49f, blockWorldCentre.Y + 0.49f, blockWorldCentre.Z + 0.51f);
				Vector3 topRight = Vector3(blockWorldCentre.X + 0.49f, blockWorldCentre.Y + 0.49f, blockWorldCentre.Z + 0.51f);

				currentPlayer->m_HighlightedFaceBottomLeft = bottomLeft;
				currentPlayer->m_HighlightedFaceBottomRight = bottomRight;
				currentPlayer->m_HighlightedFaceTopLeft = topLeft;
				currentPlayer->m_HighlightedFaceTopRight = topRight;
			}
			else
			{
				Vector3 bottomLeft = Vector3(blockWorldCentre.X + 0.49f, blockWorldCentre.Y - 0.49f, blockWorldCentre.Z - 0.51f);
				Vector3 bottomRight = Vector3(blockWorldCentre.X - 0.49f, blockWorldCentre.Y - 0.49f, blockWorldCentre.Z - 0.51f);
				Vector3 topLeft = Vector3(blockWorldCentre.X + 0.49f, blockWorldCentre.Y + 0.49f, blockWorldCentre.Z - 0.51f);
				Vector3 topRight = Vector3(blockWorldCentre.X - 0.49f, blockWorldCentre.Y + 0.49f, blockWorldCentre.Z - 0.51f);

				currentPlayer->m_HighlightedFaceBottomLeft = bottomLeft;
				currentPlayer->m_HighlightedFaceBottomRight = bottomRight;
				currentPlayer->m_HighlightedFaceTopLeft = topLeft;
				currentPlayer->m_HighlightedFaceTopRight = topRight;
			}
		}
	}
}



void World::DrawHighlightedFace(const Player* currentPlayer, const RaycastResult3D& raycastResult) const
{
	if (raycastResult.m_impactedSolidBlock)
	{
		const size_t NUMBER_OF_FACE_VERTICES = 4;
		const size_t NUMBER_OF_FACE_INDICES = 4;
		
		Vertex3D faceVertices[NUMBER_OF_FACE_VERTICES];
		uint32_t faceIndices[NUMBER_OF_FACE_INDICES] = { 0, 1, 2, 3 };

		Vertex3D faceVertex;
		faceVertex.m_Color = RGBA::WHITE;

		faceVertex = currentPlayer->m_HighlightedFaceBottomLeft;
		faceVertices[0] = faceVertex;

		faceVertex = currentPlayer->m_HighlightedFaceBottomRight;
		faceVertices[1] = faceVertex;

		faceVertex = currentPlayer->m_HighlightedFaceTopRight;
		faceVertices[2] = faceVertex;

		faceVertex = currentPlayer->m_HighlightedFaceTopLeft;
		faceVertices[3] = faceVertex;

		m_HighlightedFaceMesh->WriteToMesh(&faceVertices[0], &faceIndices[0], NUMBER_OF_FACE_VERTICES, NUMBER_OF_FACE_INDICES);
		AdvancedRenderer::SingletonInstance()->DrawLineLoopMesh(m_HighlightedFaceMesh, NUMBER_OF_FACE_VERTICES, NUMBER_OF_FACE_INDICES, 2.0f);
	}
}



bool World::ChunkIsInViewOfCamera(const Camera3D* playerCamera, const Vector3& chunkWorldMinimums) const
{
	Vector3 cameraPosition = playerCamera->m_Position;
	Vector3 cameraForwardDirection = playerCamera->GetForwardXYZ();

	Vector3 BottomSouthWestCorner = chunkWorldMinimums;
	Vector3 BottomSouthEastCorner = chunkWorldMinimums + Vector3(16.0f, 0.0f, 0.0f);
	Vector3 BottomNorthEastCorner = chunkWorldMinimums + Vector3(16.0f, 16.0f, 0.0f);
	Vector3 BottomNorthWestCorner = chunkWorldMinimums + Vector3(0.0f, 16.0f, 0.0f);
	
	Vector3 TopSouthWestCorner = chunkWorldMinimums + Vector3(0.0f, 0.0f, 128.0f);
	Vector3 TopSouthEastCorner = chunkWorldMinimums + Vector3(16.0f, 0.0f, 128.0f);
	Vector3 TopNorthEastCorner = chunkWorldMinimums + Vector3(16.0f, 16.0f, 128.0f);
	Vector3 TopNorthWestCorner = chunkWorldMinimums + Vector3(0.0f, 16.0f, 128.0f);

	Vector3 displacement;
	
	displacement = BottomSouthWestCorner - cameraPosition;
	if (Vector3::DotProduct(displacement, cameraForwardDirection) > 0.0f)
	{
		return true;
	}

	displacement = BottomSouthEastCorner - cameraPosition;
	if (Vector3::DotProduct(displacement, cameraForwardDirection) > 0.0f)
	{
		return true;
	}

	displacement = BottomNorthEastCorner - cameraPosition;
	if (Vector3::DotProduct(displacement, cameraForwardDirection) > 0.0f)
	{
		return true;
	}

	displacement = BottomNorthWestCorner - cameraPosition;
	if (Vector3::DotProduct(displacement, cameraForwardDirection) > 0.0f)
	{
		return true;
	}

	displacement = TopSouthWestCorner - cameraPosition;
	if (Vector3::DotProduct(displacement, cameraForwardDirection) > 0.0f)
	{
		return true;
	}

	displacement = TopSouthEastCorner - cameraPosition;
	if (Vector3::DotProduct(displacement, cameraForwardDirection) > 0.0f)
	{
		return true;
	}

	displacement = TopNorthEastCorner - cameraPosition;
	if (Vector3::DotProduct(displacement, cameraForwardDirection) > 0.0f)
	{
		return true;
	}

	displacement = TopNorthWestCorner - cameraPosition;
	if (Vector3::DotProduct(displacement, cameraForwardDirection) > 0.0f)
	{
		return true;
	}

	return false;
}



void World::CreateSkyboxMesh()
{
	const float SKYBOX_RADIUS = 0.5f;

	Vertex3D skyboxVertices[NUMBER_OF_SKYBOX_VERTICES];
	uint32_t skyboxIndices[NUMBER_OF_SKYBOX_INDICES];

	AABB2 textureCoordinates;
	Vertex3D skyboxVertex;
	skyboxVertex.m_Color = RGBA::WHITE;



	textureCoordinates = m_SkyboxSpriteSheet->GetTextureCoordsForSpriteCoords(IntVector2(1, 0));

	skyboxVertex.m_Position = Vector3(-SKYBOX_RADIUS, SKYBOX_RADIUS, SKYBOX_RADIUS);
	skyboxVertex.m_TextureCoordinates = Vector2(textureCoordinates.minimums.X, textureCoordinates.maximums.Y);
	skyboxVertices[0] = skyboxVertex;

	skyboxVertex.m_Position = Vector3(SKYBOX_RADIUS, SKYBOX_RADIUS, SKYBOX_RADIUS);
	skyboxVertex.m_TextureCoordinates = Vector2(textureCoordinates.maximums.X, textureCoordinates.maximums.Y);
	skyboxVertices[1] = skyboxVertex;

	skyboxVertex.m_Position = Vector3(SKYBOX_RADIUS, -SKYBOX_RADIUS, SKYBOX_RADIUS);
	skyboxVertex.m_TextureCoordinates = Vector2(textureCoordinates.maximums.X, textureCoordinates.minimums.Y);
	skyboxVertices[2] = skyboxVertex;

	skyboxVertex.m_Position = Vector3(-SKYBOX_RADIUS, -SKYBOX_RADIUS, SKYBOX_RADIUS);
	skyboxVertex.m_TextureCoordinates = Vector2(textureCoordinates.minimums.X, textureCoordinates.minimums.Y);
	skyboxVertices[3] = skyboxVertex;

	skyboxIndices[0] = 0;
	skyboxIndices[1] = 1;
	skyboxIndices[2] = 2;
	skyboxIndices[3] = 0;
	skyboxIndices[4] = 2;
	skyboxIndices[5] = 3;



	textureCoordinates = m_SkyboxSpriteSheet->GetTextureCoordsForSpriteCoords(IntVector2(0, 1));

	skyboxVertex.m_Position = Vector3(-SKYBOX_RADIUS, -SKYBOX_RADIUS, -SKYBOX_RADIUS);
	skyboxVertex.m_TextureCoordinates = Vector2(textureCoordinates.minimums.X, textureCoordinates.maximums.Y);
	skyboxVertices[4] = skyboxVertex;

	skyboxVertex.m_Position = Vector3(-SKYBOX_RADIUS, SKYBOX_RADIUS, -SKYBOX_RADIUS);
	skyboxVertex.m_TextureCoordinates = Vector2(textureCoordinates.maximums.X, textureCoordinates.maximums.Y);
	skyboxVertices[5] = skyboxVertex;

	skyboxVertex.m_Position = Vector3(-SKYBOX_RADIUS, SKYBOX_RADIUS, SKYBOX_RADIUS);
	skyboxVertex.m_TextureCoordinates = Vector2(textureCoordinates.maximums.X, textureCoordinates.minimums.Y);
	skyboxVertices[6] = skyboxVertex;

	skyboxVertex.m_Position = Vector3(-SKYBOX_RADIUS, -SKYBOX_RADIUS, SKYBOX_RADIUS);
	skyboxVertex.m_TextureCoordinates = Vector2(textureCoordinates.minimums.X, textureCoordinates.minimums.Y);
	skyboxVertices[7] = skyboxVertex;

	skyboxIndices[6] = 4;
	skyboxIndices[7] = 5;
	skyboxIndices[8] = 6;
	skyboxIndices[9] = 4;
	skyboxIndices[10] = 6;
	skyboxIndices[11] = 7;



	textureCoordinates = m_SkyboxSpriteSheet->GetTextureCoordsForSpriteCoords(IntVector2(1, 1));

	skyboxVertex.m_Position = Vector3(-SKYBOX_RADIUS, SKYBOX_RADIUS, -SKYBOX_RADIUS);
	skyboxVertex.m_TextureCoordinates = Vector2(textureCoordinates.minimums.X, textureCoordinates.maximums.Y);
	skyboxVertices[8] = skyboxVertex;

	skyboxVertex.m_Position = Vector3(SKYBOX_RADIUS, SKYBOX_RADIUS, -SKYBOX_RADIUS);
	skyboxVertex.m_TextureCoordinates = Vector2(textureCoordinates.maximums.X, textureCoordinates.maximums.Y);
	skyboxVertices[9] = skyboxVertex;

	skyboxVertex.m_Position = Vector3(SKYBOX_RADIUS, SKYBOX_RADIUS, SKYBOX_RADIUS);
	skyboxVertex.m_TextureCoordinates = Vector2(textureCoordinates.maximums.X, textureCoordinates.minimums.Y);
	skyboxVertices[10] = skyboxVertex;

	skyboxVertex.m_Position = Vector3(-SKYBOX_RADIUS, SKYBOX_RADIUS, SKYBOX_RADIUS);
	skyboxVertex.m_TextureCoordinates = Vector2(textureCoordinates.minimums.X, textureCoordinates.minimums.Y);
	skyboxVertices[11] = skyboxVertex;

	skyboxIndices[12] = 8;
	skyboxIndices[13] = 9;
	skyboxIndices[14] = 10;
	skyboxIndices[15] = 8;
	skyboxIndices[16] = 10;
	skyboxIndices[17] = 11;



	textureCoordinates = m_SkyboxSpriteSheet->GetTextureCoordsForSpriteCoords(IntVector2(2, 1));

	skyboxVertex.m_Position = Vector3(SKYBOX_RADIUS, SKYBOX_RADIUS, -SKYBOX_RADIUS);
	skyboxVertex.m_TextureCoordinates = Vector2(textureCoordinates.minimums.X, textureCoordinates.maximums.Y);
	skyboxVertices[12] = skyboxVertex;

	skyboxVertex.m_Position = Vector3(SKYBOX_RADIUS, -SKYBOX_RADIUS, -SKYBOX_RADIUS);
	skyboxVertex.m_TextureCoordinates = Vector2(textureCoordinates.maximums.X, textureCoordinates.maximums.Y);
	skyboxVertices[13] = skyboxVertex;

	skyboxVertex.m_Position = Vector3(SKYBOX_RADIUS, -SKYBOX_RADIUS, SKYBOX_RADIUS);
	skyboxVertex.m_TextureCoordinates = Vector2(textureCoordinates.maximums.X, textureCoordinates.minimums.Y);
	skyboxVertices[14] = skyboxVertex;

	skyboxVertex.m_Position = Vector3(SKYBOX_RADIUS, SKYBOX_RADIUS, SKYBOX_RADIUS);
	skyboxVertex.m_TextureCoordinates = Vector2(textureCoordinates.minimums.X, textureCoordinates.minimums.Y);
	skyboxVertices[15] = skyboxVertex;

	skyboxIndices[18] = 12;
	skyboxIndices[19] = 13;
	skyboxIndices[20] = 14;
	skyboxIndices[21] = 12;
	skyboxIndices[22] = 14;
	skyboxIndices[23] = 15;



	textureCoordinates = m_SkyboxSpriteSheet->GetTextureCoordsForSpriteCoords(IntVector2(3, 1));

	skyboxVertex.m_Position = Vector3(SKYBOX_RADIUS, -SKYBOX_RADIUS, -SKYBOX_RADIUS);
	skyboxVertex.m_TextureCoordinates = Vector2(textureCoordinates.minimums.X, textureCoordinates.maximums.Y);
	skyboxVertices[16] = skyboxVertex;

	skyboxVertex.m_Position = Vector3(-SKYBOX_RADIUS, -SKYBOX_RADIUS, -SKYBOX_RADIUS);
	skyboxVertex.m_TextureCoordinates = Vector2(textureCoordinates.maximums.X, textureCoordinates.maximums.Y);
	skyboxVertices[17] = skyboxVertex;

	skyboxVertex.m_Position = Vector3(-SKYBOX_RADIUS, -SKYBOX_RADIUS, SKYBOX_RADIUS);
	skyboxVertex.m_TextureCoordinates = Vector2(textureCoordinates.maximums.X, textureCoordinates.minimums.Y);
	skyboxVertices[18] = skyboxVertex;

	skyboxVertex.m_Position = Vector3(SKYBOX_RADIUS, -SKYBOX_RADIUS, SKYBOX_RADIUS);
	skyboxVertex.m_TextureCoordinates = Vector2(textureCoordinates.minimums.X, textureCoordinates.minimums.Y);
	skyboxVertices[19] = skyboxVertex;

	skyboxIndices[24] = 16;
	skyboxIndices[25] = 17;
	skyboxIndices[26] = 18;
	skyboxIndices[27] = 16;
	skyboxIndices[28] = 18;
	skyboxIndices[29] = 19;



	textureCoordinates = m_SkyboxSpriteSheet->GetTextureCoordsForSpriteCoords(IntVector2(1, 2));

	skyboxVertex.m_Position = Vector3(-SKYBOX_RADIUS, -SKYBOX_RADIUS, -SKYBOX_RADIUS);
	skyboxVertex.m_TextureCoordinates = Vector2(textureCoordinates.minimums.X, textureCoordinates.maximums.Y);
	skyboxVertices[20] = skyboxVertex;

	skyboxVertex.m_Position = Vector3(SKYBOX_RADIUS, -SKYBOX_RADIUS, -SKYBOX_RADIUS);
	skyboxVertex.m_TextureCoordinates = Vector2(textureCoordinates.maximums.X, textureCoordinates.maximums.Y);
	skyboxVertices[21] = skyboxVertex;

	skyboxVertex.m_Position = Vector3(SKYBOX_RADIUS, SKYBOX_RADIUS, -SKYBOX_RADIUS);
	skyboxVertex.m_TextureCoordinates = Vector2(textureCoordinates.maximums.X, textureCoordinates.minimums.Y);
	skyboxVertices[22] = skyboxVertex;

	skyboxVertex.m_Position = Vector3(-SKYBOX_RADIUS, SKYBOX_RADIUS, -SKYBOX_RADIUS);
	skyboxVertex.m_TextureCoordinates = Vector2(textureCoordinates.minimums.X, textureCoordinates.minimums.Y);
	skyboxVertices[23] = skyboxVertex;

	skyboxIndices[30] = 20;
	skyboxIndices[31] = 21;
	skyboxIndices[32] = 22;
	skyboxIndices[33] = 20;
	skyboxIndices[34] = 22;
	skyboxIndices[35] = 23;

	m_SkyboxMesh = new Mesh(skyboxVertices, skyboxIndices, NUMBER_OF_SKYBOX_VERTICES, NUMBER_OF_SKYBOX_INDICES);
}



Chunk* World::CreateChunk(const IntVector2& chunkCoordinates)
{
	if (m_NumberOfActiveChunks > MAXIMUM_NUMBER_OF_CHUNKS)
	{
		return nullptr;
	}
	
	Chunk* createdChunk = m_ChunkPool.AllocateObjectFromPool();
	createdChunk->InitializeChunk(chunkCoordinates);

	size_t availableIndex = 0U;
	bool availableIndexFound = false;
	for (size_t chunkIndex = 0; chunkIndex < MAXIMUM_NUMBER_OF_CHUNKS; ++chunkIndex)
	{
		if (m_AllChunks[chunkIndex] == nullptr)
		{
			availableIndex = chunkIndex;
			availableIndexFound = true;
			break;
		}
	}

	ASSERT_OR_DIE(availableIndexFound, "Chunk Error.");

	if (availableIndexFound)
	{
		m_AllChunks[availableIndex] = createdChunk;
		++m_NumberOfActiveChunks;
	}

	return createdChunk;
}



void World::DestroyChunk(Chunk* currentChunk)
{
	for (size_t chunkIndex = 0; chunkIndex < MAXIMUM_NUMBER_OF_CHUNKS; ++chunkIndex)
	{
		if (m_AllChunks[chunkIndex] == currentChunk)
		{
			m_ChunkPool.DeallocateObjectToPool(currentChunk);
			m_AllChunks[chunkIndex] = nullptr;

			--m_NumberOfActiveChunks;
		}
	}
}



void World::ActivateNearestMissingChunk()
{
	bool playerOneChunkIsNeeded = false;
	IntVector2 playerOneChunkCoordinates = IntVector2::ZERO;
	float playerOneShortestSquaredDistance = FLT_MAX;

	bool playerTwoChunkIsNeeded = false;
	IntVector2 playerTwoChunkCoordinates = IntVector2::ZERO;
	float playerTwoShortestSquaredDistance = FLT_MAX;

	playerOneChunkIsNeeded = FindNearestMissingChunk(playerOneChunkCoordinates, playerOneShortestSquaredDistance, m_PlayerOne);

	if (g_TwoPlayerMode)
	{
		playerTwoChunkIsNeeded = FindNearestMissingChunk(playerTwoChunkCoordinates, playerTwoShortestSquaredDistance, m_PlayerTwo);
	}

	if (playerOneChunkIsNeeded || playerTwoChunkIsNeeded)
	{
		IntVector2 mostUrgentMissingChunkCoordinates;

		float overallShortestDistance = GetMinimumOfTwoFloats(playerOneShortestSquaredDistance, playerTwoShortestSquaredDistance);

		if (overallShortestDistance == playerOneShortestSquaredDistance)
		{
			mostUrgentMissingChunkCoordinates = playerOneChunkCoordinates;
		}
		else if (overallShortestDistance == playerTwoShortestSquaredDistance)
		{
			mostUrgentMissingChunkCoordinates = playerTwoChunkCoordinates;
		}

		Chunk* activatedChunk = CreateChunk(mostUrgentMissingChunkCoordinates);
		if (activatedChunk == nullptr)
		{
			return;
		}

		ChunkProxy activatedChunkProxy = activatedChunk->GetChunkProxyFromChunk();
		bool addedSuccessfully = false;

		EnterCriticalSection(&g_ChunkLoadingCriticalSection);
		{
			addedSuccessfully = AddToSharedLoadedChunkProxies(activatedChunkProxy);
		}
		LeaveCriticalSection(&g_ChunkLoadingCriticalSection);

		if (!addedSuccessfully)
		{
			DestroyChunk(activatedChunk);
		}
	}
}



bool World::FindNearestMissingChunk(IntVector2& chunkCoordinates, float& lowestPossibleSquaredDistance, const Player* currentPlayer)
{
	Vector3 playerPosition = currentPlayer->m_Position;
	IntVector2 currentChunkCoordinates = GetChunkCoordinatesForWorldCoordinates(playerPosition);
	Vector2 playerPositionXY = Vector2(playerPosition.X, playerPosition.Y);

	float shortestSquaredDistance = SQUARED_ACTIVATION_DISTANCE;

	int minimumChunkRangeX = currentChunkCoordinates.X - ACTIVE_CHUNK_RADIUS;
	int maximumChunkRangeX = currentChunkCoordinates.X + ACTIVE_CHUNK_RADIUS;

	int minimumChunkRangeY = currentChunkCoordinates.Y - ACTIVE_CHUNK_RADIUS;
	int maximumChunkRangeY = currentChunkCoordinates.Y + ACTIVE_CHUNK_RADIUS;

	for (int chunkIndexX = minimumChunkRangeX; chunkIndexX <= maximumChunkRangeX; ++chunkIndexX)
	{
		for (int chunkIndexY = minimumChunkRangeY; chunkIndexY <= maximumChunkRangeY; ++chunkIndexY)
		{
			IntVector2 missingChunkCoordinates = IntVector2(chunkIndexX, chunkIndexY);
			Vector2 missingChunkCentre = GetChunkWorldCentreForChunkCoordinates(missingChunkCoordinates);

			float squaredDistanceToChunk = CalculateEuclidianSquaredDistanceIn2D(playerPositionXY, missingChunkCentre);
			if (squaredDistanceToChunk < shortestSquaredDistance)
			{
				if (FindActiveChunkWithCoordinates(missingChunkCoordinates) == nullptr)
				{
					shortestSquaredDistance = squaredDistanceToChunk;
					chunkCoordinates = missingChunkCoordinates;
					lowestPossibleSquaredDistance = shortestSquaredDistance;
				}
			}
		}
	}

	if (shortestSquaredDistance < SQUARED_ACTIVATION_DISTANCE)
	{
		return true;
	}

	return false;
}



void World::ConnectChunk(Chunk* currentChunk)
{
	int chunkX = currentChunk->GetChunkCoordinates().X;
	int chunkY = currentChunk->GetChunkCoordinates().Y;

	Chunk* foundChunk = nullptr;

	foundChunk = FindActiveChunkWithCoordinates(IntVector2(chunkX + 1, chunkY));
	if (foundChunk != nullptr)
	{
		Chunk* eastNeighbour = foundChunk;
		currentChunk->m_EasternNeighbour = eastNeighbour;
		eastNeighbour->m_WesternNeighbour = currentChunk;
		eastNeighbour->SetModified(true);
	}

	foundChunk = FindActiveChunkWithCoordinates(IntVector2(chunkX - 1, chunkY));
	if (foundChunk != nullptr)
	{
		Chunk* westNeighbour = foundChunk;
		currentChunk->m_WesternNeighbour = westNeighbour;
		westNeighbour->m_EasternNeighbour = currentChunk;
		westNeighbour->SetModified(true);
	}

	foundChunk = FindActiveChunkWithCoordinates(IntVector2(chunkX, chunkY + 1));
	if (foundChunk != nullptr)
	{
		Chunk* northNeighbour = foundChunk;
		currentChunk->m_NorthernNeighbour = northNeighbour;
		northNeighbour->m_SouthernNeighbour = currentChunk;
		northNeighbour->SetModified(true);
	}

	foundChunk = FindActiveChunkWithCoordinates(IntVector2(chunkX, chunkY - 1));
	if (foundChunk != nullptr)
	{
		Chunk* southNeighbour = foundChunk;
		currentChunk->m_SouthernNeighbour = southNeighbour;
		southNeighbour->m_NorthernNeighbour = currentChunk;
		southNeighbour->SetModified(true);
	}
}



void World::DeactivateFarthestChunk()
{
	IntVector2 chunkCoordinates;
	bool chunkFound = FindFarthestChunk(chunkCoordinates);

	if (chunkFound)
	{
		Chunk* foundChunk = FindActiveChunkWithCoordinates(chunkCoordinates);
		ChunkProxy foundChunkProxy = foundChunk->GetChunkProxyFromChunk();

		bool addedSuccessfully = false;
		EnterCriticalSection(&g_ChunkSavingCriticalSection);
		{
			addedSuccessfully = AddToSharedSavedChunkProxies(foundChunkProxy);
		}
		LeaveCriticalSection(&g_ChunkSavingCriticalSection);

		if (addedSuccessfully)
		{
			DisconnectChunk(foundChunk);
			DestroyChunk(foundChunk);
		}
	}
}



bool World::FindFarthestChunk(IntVector2& chunkCoordinates)
{
	Vector3 playerOnePosition = m_PlayerOne->m_Position;
	Vector2 playerOnePositionXY = Vector2(playerOnePosition.X, playerOnePosition.Y);

	float largestSquaredDistance = SQUARED_FLUSHING_DISTANCE;

	for (Chunk* currentChunk : m_AllChunks)
	{
		if (currentChunk == nullptr)
		{
			continue;
		}
		
		IntVector2 flushChunkCoordinates = currentChunk->GetChunkCoordinates();
		Vector2 flushChunkCentre = GetChunkWorldCentreForChunkCoordinates(flushChunkCoordinates);

		float playerOneSquaredDistanceToChunk = CalculateEuclidianSquaredDistanceIn2D(playerOnePositionXY, flushChunkCentre);
		float squaredDistanceToNearestPlayer = playerOneSquaredDistanceToChunk;

		if (g_TwoPlayerMode)
		{
			Vector3 playerTwoPosition = m_PlayerTwo->m_Position;
			Vector2 playerTwoPositionXY = Vector2(playerTwoPosition.X, playerTwoPosition.Y);

			float playerTwoSquaredDistanceToChunk = CalculateEuclidianSquaredDistanceIn2D(playerTwoPositionXY, flushChunkCentre);
			if (playerTwoSquaredDistanceToChunk < squaredDistanceToNearestPlayer)
			{
				squaredDistanceToNearestPlayer = playerTwoSquaredDistanceToChunk;
			}
		}

		if (squaredDistanceToNearestPlayer > largestSquaredDistance)
		{
			largestSquaredDistance = squaredDistanceToNearestPlayer;
			chunkCoordinates = flushChunkCoordinates;
		}
	}

	if (largestSquaredDistance > SQUARED_FLUSHING_DISTANCE)
	{
		return true;
	}

	return false;
}



void World::DisconnectChunk(Chunk* currentChunk)
{
	int chunkX = currentChunk->GetChunkCoordinates().X;
	int chunkY = currentChunk->GetChunkCoordinates().Y;

	Chunk* foundChunk = nullptr;

	foundChunk = FindActiveChunkWithCoordinates(IntVector2(chunkX + 1, chunkY));
	if (foundChunk != nullptr)
	{
		Chunk* eastNeighbour = foundChunk;
		eastNeighbour->m_WesternNeighbour = nullptr;
		eastNeighbour->SetModified(true);
	}

	foundChunk = FindActiveChunkWithCoordinates(IntVector2(chunkX - 1, chunkY));
	if (foundChunk != nullptr)
	{
		Chunk* westNeighbour = foundChunk;
		westNeighbour->m_EasternNeighbour = nullptr;
		westNeighbour->SetModified(true);
	}

	foundChunk = FindActiveChunkWithCoordinates(IntVector2(chunkX, chunkY + 1));
	if (foundChunk != nullptr)
	{
		Chunk* northNeighbour = foundChunk;
		northNeighbour->m_SouthernNeighbour = nullptr;
		northNeighbour->SetModified(true);
	}

	foundChunk = FindActiveChunkWithCoordinates(IntVector2(chunkX, chunkY - 1));
	if (foundChunk != nullptr)
	{
		Chunk* southNeighbour = foundChunk;
		southNeighbour->m_NorthernNeighbour = nullptr;
		southNeighbour->SetModified(true);
	}
}



void World::PopulateLoadedChunks()
{
	size_t proxyArraySize = MAXIMUM_NUMBER_OF_CHUNKS * sizeof(ChunkProxy);
	size_t numberOfPopulatedChunks = 0U;
	
	EnterCriticalSection(&g_ChunkPopulatingCriticalSection);
	{
		ChunkProxy* temporaryPointer = g_SharedPopulatedChunkProxies;
		g_SharedPopulatedChunkProxies = g_LocalPopulatedChunkProxies;
		g_LocalPopulatedChunkProxies = temporaryPointer;

		numberOfPopulatedChunks = g_NumberOfPopulatedChunks;
		g_NumberOfPopulatedChunks = 0U;
	}
	LeaveCriticalSection(&g_ChunkPopulatingCriticalSection);

	if (numberOfPopulatedChunks > 0U)
	{
		for (size_t chunkProxyIndex = 0; chunkProxyIndex < MAXIMUM_NUMBER_OF_CHUNKS; ++chunkProxyIndex)
		{
			ChunkProxy currentProxy = g_LocalPopulatedChunkProxies[chunkProxyIndex];
			if (!currentProxy.m_IsValid)
			{
				continue;
			}

			Chunk* desiredChunk = nullptr;
			for (Chunk* currentChunk : m_AllChunks)
			{
				if (currentChunk == nullptr)
				{
					continue;
				}
				
				if (currentChunk->GetChunkCoordinates() == currentProxy.m_ChunkCoordinates)
				{
					desiredChunk = currentChunk;
					break;
				}
			}

			if (desiredChunk != nullptr)
			{
				desiredChunk->PopulateChunkFromChunkProxy(currentProxy);
				ConnectChunk(desiredChunk);
				CalculateLightForChunk(desiredChunk);
			}
		}

		memset(g_LocalPopulatedChunkProxies, 0, proxyArraySize);
	}
}



bool World::AddToSharedLoadedChunkProxies(const ChunkProxy& currentChunkProxy)
{
	for (size_t chunkProxyIndex = 0; chunkProxyIndex < MAXIMUM_NUMBER_OF_CHUNKS; ++chunkProxyIndex)
	{
		ChunkProxy& currentProxy = g_SharedLoadedChunkProxies[chunkProxyIndex];
		if (currentProxy.m_IsValid)
		{
			continue;
		}

		currentProxy = currentChunkProxy;
		++g_NumberOfActivatedChunks;

		return true;
	}

	return false;
}



bool World::AddToSharedSavedChunkProxies(const ChunkProxy& currentChunkProxy)
{
	for (size_t chunkProxyIndex = 0; chunkProxyIndex < MAXIMUM_NUMBER_OF_CHUNKS; ++chunkProxyIndex)
	{
		ChunkProxy& currentProxy = g_SharedSavedChunkProxies[chunkProxyIndex];
		if (currentProxy.m_IsValid)
		{
			continue;
		}

		currentProxy = currentChunkProxy;
		++g_NumberOfSavedChunks;

		return true;
	}

	return false;
}



Chunk* World::FindActiveChunkWithCoordinates(const IntVector2& chunkCoordinates) const
{
	for (Chunk* currentChunk : m_AllChunks)
	{
		if (currentChunk != nullptr)
		{
			IntVector2 currentChunkCoordinates = currentChunk->GetChunkCoordinates();
			if (currentChunkCoordinates == chunkCoordinates)
			{
				return currentChunk;
			}
		}
	}

	return nullptr;
}



void World::CalculateLightForChunk(Chunk* currentChunk)
{
	for (int blockIndexY = 0; blockIndexY < NUMBER_OF_BLOCKS_Y; ++blockIndexY)
	{
		for (int blockIndexX = 0; blockIndexX < NUMBER_OF_BLOCKS_X; ++blockIndexX)
		{
			for (int blockIndexZ = NUMBER_OF_BLOCKS_Z - 1; blockIndexZ >= 0; --blockIndexZ)
			{
				IntVector3 localCoordinates = IntVector3(blockIndexX, blockIndexY, blockIndexZ);

				int localIndex = currentChunk->GetLocalIndexForLocalCoordinates(localCoordinates);

				if (currentChunk->m_Blocks[localIndex].IsOpaque())
				{
					break;
				}

				currentChunk->m_Blocks[localIndex].SetSkyBit();
				currentChunk->m_Blocks[localIndex].SetLightValue(SKY_LIGHT_VALUE);
			}
		}
	}

	for (int blockIndexY = 0; blockIndexY < NUMBER_OF_BLOCKS_Y; ++blockIndexY)
	{
		for (int blockIndexX = 0; blockIndexX < NUMBER_OF_BLOCKS_X; ++blockIndexX)
		{
			for (int blockIndexZ = NUMBER_OF_BLOCKS_Z - 1; blockIndexZ >= 0; --blockIndexZ)
			{
				IntVector3 localCoordinates = IntVector3(blockIndexX, blockIndexY, blockIndexZ);

				int localIndex = currentChunk->GetLocalIndexForLocalCoordinates(localCoordinates);

				if (currentChunk->m_Blocks[localIndex].IsOpaque())
				{
					break;
				}

				BlockInfo currentBlockInfo = BlockInfo(currentChunk, localIndex);

				BlockInfo easternNeighbour = currentBlockInfo.GetEasternNeighbour();
				if (easternNeighbour.GetChunk() != nullptr)
				{
					Block* easternBlock = easternNeighbour.GetBlock();
					if (!easternBlock->IsOpaque())
					{
						if (!easternBlock->IsSky())
						{
							MarkLightingAsDirty(easternNeighbour);
						}
					}
				}

				BlockInfo westernNeighbour = currentBlockInfo.GetWesternNeighbour();
				if (westernNeighbour.GetChunk() != nullptr)
				{
					Block* westernBlock = westernNeighbour.GetBlock();
					if (!westernBlock->IsOpaque())
					{
						if (!westernBlock->IsSky())
						{
							MarkLightingAsDirty(westernNeighbour);
						}
					}
				}

				BlockInfo northernNeighbour = currentBlockInfo.GetNorthernNeighbour();
				if (northernNeighbour.GetChunk() != nullptr)
				{
					Block* northernBlock = northernNeighbour.GetBlock();
					if (!northernBlock->IsOpaque())
					{
						if (!northernBlock->IsSky())
						{
							MarkLightingAsDirty(northernNeighbour);
						}
					}
				}

				BlockInfo southernNeighbour = currentBlockInfo.GetSouthernNeighbour();
				if (southernNeighbour.GetChunk() != nullptr)
				{
					Block* southernBlock = southernNeighbour.GetBlock();
					if (!southernBlock->IsOpaque())
					{
						if (!southernBlock->IsSky())
						{
							MarkLightingAsDirty(southernNeighbour);
						}
					}
				}
			}
		}
	}

	for (int blockIndexZ = 0; blockIndexZ < NUMBER_OF_BLOCKS_Z; ++blockIndexZ)
	{
		for (int blockIndexY = 0; blockIndexY < NUMBER_OF_BLOCKS_Y; ++blockIndexY)
		{
			for (int blockIndexX = 0; blockIndexX < NUMBER_OF_BLOCKS_X; ++blockIndexX)
			{
				IntVector3 localCoordinates = IntVector3(blockIndexX, blockIndexY, blockIndexZ);

				int localIndex = currentChunk->GetLocalIndexForLocalCoordinates(localCoordinates);

				int illuminationValue = currentChunk->m_Blocks[localIndex].GetInternalIlluminationValue();

				BlockInfo currentBlockInfo = BlockInfo(currentChunk, localIndex);

				if (illuminationValue > 0)
				{
					MarkLightingAsDirty(currentBlockInfo);
				}

				if (!currentBlockInfo.GetBlock()->IsOpaque())
				{
					if (currentBlockInfo.IsEdgeBlock())
					{
						MarkLightingAsDirty(currentBlockInfo);
					}
				}
			}
		}
	}
}



void World::MarkLightingAsDirty(BlockInfo currentBlockInfo)
{
	Block* currentBlock = currentBlockInfo.GetBlock();

	if (currentBlock != nullptr)
	{
		if (!currentBlock->LightingIsDirty())
		{
			m_DirtyBlocks.push_back(currentBlockInfo);
			currentBlock->SetLightingDirty();
		}
	}
}



int World::CalculateIdealLightForBlock(BlockInfo currentBlockInfo)
{
	Block* currentBlock = currentBlockInfo.GetBlock();

	int internalLightValue = currentBlock->GetInternalIlluminationValue();
	if (currentBlock->IsOpaque())
	{
		return internalLightValue;
	}

	int neighbourPropagatedLightValue = GetPropagatedLightFromNeighbour(currentBlockInfo);
	int skyLightValue = currentBlock->IsSky() ? SKY_LIGHT_VALUE : 0;

	return GetMaximumOfThreeInts(internalLightValue, neighbourPropagatedLightValue, skyLightValue);
}



int World::GetPropagatedLightFromNeighbour(BlockInfo currentBlockInfo)
{
	int neededPropagatedLight = 0;
	int idealPropogatedLight;

	int currentPropogatedLight = neededPropagatedLight;

	BlockInfo easternNeighbour = currentBlockInfo.GetEasternNeighbour();
	if (easternNeighbour.GetChunk() != nullptr)
	{
		currentPropogatedLight = easternNeighbour.GetBlock()->GetLightValue();
		idealPropogatedLight = currentPropogatedLight - 1;
		neededPropagatedLight = GetMaximumOfTwoInts(idealPropogatedLight, neededPropagatedLight);
	}

	BlockInfo westernNeighbour = currentBlockInfo.GetWesternNeighbour();
	if (westernNeighbour.GetChunk() != nullptr)
	{
		currentPropogatedLight = westernNeighbour.GetBlock()->GetLightValue();
		idealPropogatedLight = currentPropogatedLight - 1;
		neededPropagatedLight = GetMaximumOfTwoInts(idealPropogatedLight, neededPropagatedLight);
	}

	BlockInfo northernNeighbour = currentBlockInfo.GetNorthernNeighbour();
	if (northernNeighbour.GetChunk() != nullptr)
	{
		currentPropogatedLight = northernNeighbour.GetBlock()->GetLightValue();
		idealPropogatedLight = currentPropogatedLight - 1;
		neededPropagatedLight = GetMaximumOfTwoInts(idealPropogatedLight, neededPropagatedLight);
	}

	BlockInfo southernNeighbour = currentBlockInfo.GetSouthernNeighbour();
	if (southernNeighbour.GetChunk() != nullptr)
	{
		currentPropogatedLight = southernNeighbour.GetBlock()->GetLightValue();
		idealPropogatedLight = currentPropogatedLight - 1;
		neededPropagatedLight = GetMaximumOfTwoInts(idealPropogatedLight, neededPropagatedLight);
	}

	BlockInfo aboveNeighbour = currentBlockInfo.GetAboveNeighbour();
	if (aboveNeighbour.GetChunk() != nullptr)
	{
		currentPropogatedLight = aboveNeighbour.GetBlock()->GetLightValue();
		idealPropogatedLight = currentPropogatedLight - 1;
		neededPropagatedLight = GetMaximumOfTwoInts(idealPropogatedLight, neededPropagatedLight);
	}

	BlockInfo belowNeighbour = currentBlockInfo.GetBelowNeighbour();
	if (belowNeighbour.GetChunk() != nullptr)
	{
		currentPropogatedLight = belowNeighbour.GetBlock()->GetLightValue();
		idealPropogatedLight = currentPropogatedLight - 1;
		neededPropagatedLight = GetMaximumOfTwoInts(idealPropogatedLight, neededPropagatedLight);
	}

	if (neededPropagatedLight < 0)
	{
		neededPropagatedLight = 0;
	}

	return neededPropagatedLight;
}



void World::UpdateLighting()
{
	while (!m_DirtyBlocks.empty())
	{
		BlockInfo dirtyBlockInfo = m_DirtyBlocks.front();
		m_DirtyBlocks.pop_front();
		dirtyBlockInfo.GetBlock()->ClearLightingDirty();
		UpdateLightForBlock(dirtyBlockInfo);
	}
}



void World::UpdateLightForBlock(BlockInfo currentBlockInfo)
{
	int idealLightValue = CalculateIdealLightForBlock(currentBlockInfo);
	int currentLightValue = currentBlockInfo.GetBlock()->GetLightValue();

	if (idealLightValue != currentLightValue)
	{
		currentBlockInfo.GetBlock()->SetLightValue(idealLightValue);
		DirtyNeighboursOnLightChange(currentBlockInfo);
		currentBlockInfo.GetChunk()->SetModified(true);
	}

	ModifyNeighbourChunksForEdgeBlocks(currentBlockInfo);
}



void World::DirtyNeighboursOnLightChange(BlockInfo currentBlockInfo)
{
	BlockInfo easternNeighbour = currentBlockInfo.GetEasternNeighbour();
	if (easternNeighbour.GetChunk() != nullptr)
	{
		Block* easternBlock = easternNeighbour.GetBlock();
		if (!easternBlock->IsOpaque())
		{
			MarkLightingAsDirty(easternNeighbour);
		}
	}

	BlockInfo westernNeighbour = currentBlockInfo.GetWesternNeighbour();
	if (westernNeighbour.GetChunk() != nullptr)
	{
		Block* westernBlock = westernNeighbour.GetBlock();
		if (!westernBlock->IsOpaque())
		{
			MarkLightingAsDirty(westernNeighbour);
		}
	}

	BlockInfo northernNeighbour = currentBlockInfo.GetNorthernNeighbour();
	if (northernNeighbour.GetChunk() != nullptr)
	{
		Block* northernBlock = northernNeighbour.GetBlock();
		if (!northernBlock->IsOpaque())
		{
			MarkLightingAsDirty(northernNeighbour);
		}
	}

	BlockInfo southernNeighbour = currentBlockInfo.GetSouthernNeighbour();
	if (southernNeighbour.GetChunk() != nullptr)
	{
		Block* southernBlock = southernNeighbour.GetBlock();
		if (!southernBlock->IsOpaque())
		{
			MarkLightingAsDirty(southernNeighbour);
		}
	}

	BlockInfo aboveNeighbour = currentBlockInfo.GetAboveNeighbour();
	if (aboveNeighbour.GetChunk() != nullptr)
	{
		Block* aboveBlock = aboveNeighbour.GetBlock();
		if (!aboveBlock->IsOpaque())
		{
			MarkLightingAsDirty(aboveNeighbour);
		}
	}

	BlockInfo belowNeighbour = currentBlockInfo.GetBelowNeighbour();
	if (belowNeighbour.GetChunk() != nullptr)
	{
		Block* belowBlock = belowNeighbour.GetBlock();
		if (!belowBlock->IsOpaque())
		{
			MarkLightingAsDirty(belowNeighbour);
		}
	}
}



void World::PlaceBlock(const RaycastResult3D& raycastResult, uint8_t controllerNumber, uint8_t selectedBlock)
{
	BlockInfo previousBlockInfo = raycastResult.m_previousBlockInfo;
	BlockInfo currentBlockInfo = raycastResult.m_currentBlockInfo;

	Block* previousBlock = previousBlockInfo.GetBlock();
	Block* currentBlock = currentBlockInfo.GetBlock();

	bool blockCanBePlaced = false;

	BlockInfo playerOneOverlappingBlocks[NUMBER_OF_BOUNDING_POINTS];
	size_t numberOfOverlappingPlayerOneBlocks = 0U;
	GetPlayerOverlappingBlocks(m_PlayerOne, playerOneOverlappingBlocks, numberOfOverlappingPlayerOneBlocks);

	if (g_TwoPlayerMode)
	{
		BlockInfo playerTwoOverlappingBlocks[NUMBER_OF_BOUNDING_POINTS];
		size_t numberOfOverlappingPlayerTwoBlocks = 0U;
		GetPlayerOverlappingBlocks(m_PlayerTwo, playerTwoOverlappingBlocks, numberOfOverlappingPlayerTwoBlocks);

		if (currentBlock->GetType() != WATER_BLOCK)
		{
			blockCanBePlaced =	!PlayerOverlapsSelectedBlock(previousBlockInfo, playerOneOverlappingBlocks, numberOfOverlappingPlayerOneBlocks) &&
								!PlayerOverlapsSelectedBlock(previousBlockInfo, playerTwoOverlappingBlocks, numberOfOverlappingPlayerTwoBlocks);
		}
		else
		{
			blockCanBePlaced =	!PlayerOverlapsSelectedBlock(currentBlockInfo, playerOneOverlappingBlocks, numberOfOverlappingPlayerOneBlocks) &&
								!PlayerOverlapsSelectedBlock(currentBlockInfo, playerTwoOverlappingBlocks, numberOfOverlappingPlayerTwoBlocks);
		}
	}
	else
	{
		if (currentBlock->GetType() != WATER_BLOCK)
		{
			blockCanBePlaced = !PlayerOverlapsSelectedBlock(previousBlockInfo, playerOneOverlappingBlocks, numberOfOverlappingPlayerOneBlocks);
		}
		else
		{
			blockCanBePlaced = !PlayerOverlapsSelectedBlock(currentBlockInfo, playerOneOverlappingBlocks, numberOfOverlappingPlayerOneBlocks);
		}
	}

	if (InputSystem::SingletonInstance()->ButtonWasJustPressed(controllerNumber, RIGHT_BUMPER))
	{
		if (currentBlock->GetType() != WATER_BLOCK)
		{
			if (blockCanBePlaced)
			{
				RecalculateLightUponPlacing(previousBlockInfo);
				previousBlock->SetType(selectedBlock);
				Chunk* previousBlockChunk = previousBlockInfo.GetChunk();
				previousBlockChunk->SetModified(true);
				ModifyNeighbourChunksForEdgeBlocks(previousBlockInfo);

				AudioSystem::SingletonInstance()->PlaySound(previousBlock->GetPlacingSound(), FORWARD_PLAYBACK_MODE);
			}
		}
		else
		{
			if (blockCanBePlaced)
			{
				RecalculateLightUponPlacing(currentBlockInfo);
				currentBlock->SetType(selectedBlock);
				Chunk* currentBlockChunk = currentBlockInfo.GetChunk();
				currentBlockChunk->SetModified(true);
				ModifyNeighbourChunksForEdgeBlocks(currentBlockInfo);

				AudioSystem::SingletonInstance()->PlaySound(currentBlock->GetPlacingSound(), FORWARD_PLAYBACK_MODE);
			}
		}
	}
}



void World::DigBlock(const RaycastResult3D& raycastResult, uint8_t controllerNumber)
{
	BlockInfo currentBlockInfo = raycastResult.m_currentBlockInfo;
	Block* currentBlock = currentBlockInfo.GetBlock();

	if (InputSystem::SingletonInstance()->ButtonWasJustPressed(controllerNumber, LEFT_BUMPER))
	{
		AudioSystem::SingletonInstance()->PlaySound(currentBlock->GetBreakingSound(), FORWARD_PLAYBACK_MODE);

		if (currentBlock->GetType() != WATER_BLOCK && currentBlock->GetType() != BEDROCK_BLOCK)
		{
			Block* easternNeighbourBlock = currentBlockInfo.GetEasternNeighbour().GetBlock();
			Block* westernNeighbourBlock = currentBlockInfo.GetWesternNeighbour().GetBlock();
			Block* northernNeighbourBlock = currentBlockInfo.GetNorthernNeighbour().GetBlock();
			Block* southernNeighbourBlock = currentBlockInfo.GetSouthernNeighbour().GetBlock();
			Block* aboveNeighbourBlock = currentBlockInfo.GetAboveNeighbour().GetBlock();

			if (easternNeighbourBlock->GetType() != WATER_BLOCK &&
				westernNeighbourBlock->GetType() != WATER_BLOCK &&
				northernNeighbourBlock->GetType() != WATER_BLOCK &&
				southernNeighbourBlock->GetType() != WATER_BLOCK &&
				aboveNeighbourBlock->GetType() != WATER_BLOCK)
			{
				currentBlock->SetType(AIR_BLOCK);
			}
			else
			{
				currentBlock->SetType(WATER_BLOCK);
			}

			RecalculateLightUponDigging(currentBlockInfo);
			Chunk* currentBlockChunk = currentBlockInfo.GetChunk();
			currentBlockChunk->SetModified(true);

			ModifyNeighbourChunksForEdgeBlocks(currentBlockInfo);
		}
	}
}



void World::ModifyNeighbourChunksForEdgeBlocks(BlockInfo currentBlockInfo)
{
	Chunk* currentChunk = currentBlockInfo.GetChunk();

	if (currentBlockInfo.IsEasternEdgeBlock())
	{
		if (currentChunk->m_EasternNeighbour != nullptr)
		{
			currentChunk->m_EasternNeighbour->SetModified(true);
		}
	}
	if (currentBlockInfo.IsWesternEdgeBlock())
	{
		if (currentChunk->m_WesternNeighbour != nullptr)
		{
			currentChunk->m_WesternNeighbour->SetModified(true);
		}
	}
	if (currentBlockInfo.IsNorthernEdgeBlock())
	{
		if (currentChunk->m_NorthernNeighbour != nullptr)
		{
			currentChunk->m_NorthernNeighbour->SetModified(true);
		}
	}
	if (currentBlockInfo.IsSouthernEdgeBlock())
	{
		if (currentChunk->m_SouthernNeighbour != nullptr)
		{
			currentChunk->m_SouthernNeighbour->SetModified(true);
		}
	}
}



void World::RecalculateLightUponPlacing(BlockInfo currentBlockInfo)
{
	Block* currentBlock = currentBlockInfo.GetBlock();

	if (currentBlock->IsSky())
	{
		Chunk* currentChunk = currentBlockInfo.GetChunk();
		int currentBlockIndex = currentBlockInfo.GetBlockIndex();
		int newBlockIndex = currentBlockIndex;

		while (!currentChunk->m_Blocks[newBlockIndex].IsOpaque())
		{
			currentChunk->m_Blocks[newBlockIndex].ClearSkyBit();
			BlockInfo newBlockInfo = BlockInfo(currentChunk, newBlockIndex);
			MarkLightingAsDirty(newBlockInfo);

			newBlockIndex -= NUMBER_OF_BLOCKS_PER_LAYER;
		}
	}
	else
	{
		MarkLightingAsDirty(currentBlockInfo);
	}
}



void World::RecalculateLightUponDigging(BlockInfo currentBlockInfo)
{
	BlockInfo aboveNeighbourInfo = currentBlockInfo.GetAboveNeighbour();
	Block* aboveNeighbourBlock = aboveNeighbourInfo.GetBlock();

	if (aboveNeighbourBlock->IsSky())
	{
		Chunk* currentChunk = currentBlockInfo.GetChunk();
		int currentBlockIndex = currentBlockInfo.GetBlockIndex();
		int newBlockIndex = currentBlockIndex;

		while (!currentChunk->m_Blocks[newBlockIndex].IsOpaque())
		{
			currentChunk->m_Blocks[newBlockIndex].SetSkyBit();
			BlockInfo newBlockInfo = BlockInfo(currentChunk, newBlockIndex);
			MarkLightingAsDirty(newBlockInfo);

			newBlockIndex -= NUMBER_OF_BLOCKS_PER_LAYER;
		}
	}
	else
	{
		MarkLightingAsDirty(currentBlockInfo);
	}
}



void World::CreateAndLoadPlayer(Player*& currentPlayer, Camera3D* playerCamera, const char* currentPlayerFileName)
{
	float playerHeight = static_cast<float>(NUMBER_OF_BLOCKS_Z);
	
	char playerFilePath[128];
	sprintf_s(playerFilePath, "Data/SaveSlot/PlayerData/%s", currentPlayerFileName);

	BinaryFileReader fileReader;
	bool fileExists = fileReader.OpenBinaryFile(playerFilePath);

	if (fileExists)
	{
		Vector2 playerPosition;
		fileReader.ReadVector2(playerPosition);

		currentPlayer = new Player(Vector3(playerPosition.X, playerPosition.Y, playerHeight), WALKING_MODE);

		fileReader.ReadVector3(playerCamera->m_Position);
		fileReader.Read<float>(playerCamera->m_Orientation.m_RollAngleInDegrees);
		fileReader.Read<float>(playerCamera->m_Orientation.m_PitchAngleInDegrees);
		fileReader.Read<float>(playerCamera->m_Orientation.m_YawAngleInDegrees);

		fileReader.CloseBinaryFile();
	}
	else
	{
		currentPlayer = new Player(Vector3(0.0f, 0.0f, playerHeight), WALKING_MODE);
	}
}



void World::SaveAndDestroyPlayer(Player*& currentPlayer, const Camera3D* playerCamera, const char* currentPlayerFileName)
{
	char playerFilePath[128];
	sprintf_s(playerFilePath, "Data/SaveSlot/PlayerData/%s", currentPlayerFileName);

	BinaryFileWriter fileWriter;
	fileWriter.OpenBinaryFile(playerFilePath);

	fileWriter.WriteVector2(Vector2(currentPlayer->m_Position.X, currentPlayer->m_Position.Y));
	fileWriter.WriteVector3(playerCamera->m_Position);
	fileWriter.Write<float>(playerCamera->m_Orientation.m_RollAngleInDegrees);
	fileWriter.Write<float>(playerCamera->m_Orientation.m_PitchAngleInDegrees);
	fileWriter.Write<float>(playerCamera->m_Orientation.m_YawAngleInDegrees);

	fileWriter.CloseBinaryFile();

	delete currentPlayer;
}



void World::TogglePhysicsMode(uint8_t currentPlayerID, Player* currentPlayer)
{
	if (InputSystem::SingletonInstance()->ButtonIsHeldDown(currentPlayerID, D_PAD_DOWN))
	{
		if (InputSystem::SingletonInstance()->ButtonWasJustPressed(currentPlayerID, RIGHT_STICK_BUTTON))
		{
			if (currentPlayer->m_CurrentPhysicsMode == WALKING_MODE)
			{
				currentPlayer->m_CurrentPhysicsMode = FLYING_MODE;
			}
			else if (currentPlayer->m_CurrentPhysicsMode == FLYING_MODE)
			{
				currentPlayer->m_CurrentPhysicsMode = NO_CLIP_MODE;
			}
			else if (currentPlayer->m_CurrentPhysicsMode == NO_CLIP_MODE)
			{
				currentPlayer->m_CurrentPhysicsMode = WALKING_MODE;
			}
		}
	}
}



bool World::PlayerIsOnTheGround(Player* currentPlayer)
{
	Vector3 southWestPoint = currentPlayer->BottomSouthWestPoint();
	Vector3 southEastPoint = currentPlayer->BottomSouthEastPoint();
	Vector3 northEastPoint = currentPlayer->BottomNorthEastPoint();
	Vector3 northWestPoint = currentPlayer->BottomNorthWestPoint();

	southWestPoint.Z -= 0.01f;
	southEastPoint.Z -= 0.01f;
	northEastPoint.Z -= 0.01f;
	northWestPoint.Z -= 0.01f;

	Block* southWestFloorBlock = GetBlockForWorldCoordinates(southWestPoint);
	Block* southEastFloorBlock = GetBlockForWorldCoordinates(southEastPoint);
	Block* northEastFloorBlock = GetBlockForWorldCoordinates(northEastPoint);
	Block* northWestFloorBlock = GetBlockForWorldCoordinates(northWestPoint);

	if (southWestFloorBlock != nullptr)
	{
		if (southWestFloorBlock->IsSolid())
		{
			return true;
		}
	}

	if (southEastFloorBlock != nullptr)
	{
		if (southEastFloorBlock->IsSolid())
		{
			return true;
		}
	}

	if (northEastFloorBlock != nullptr)
	{
		if (northEastFloorBlock->IsSolid())
		{
			return true;
		}
	}

	if (northWestFloorBlock != nullptr)
	{
		if (northWestFloorBlock->IsSolid())
		{
			return true;
		}
	}

	return false;
}



void World::ApplyPreventativePhysics(Player* currentPlayer)
{
	RaycastResult3D raycastResults[NUMBER_OF_BOUNDING_POINTS];
	
	float remainingTime = 1.0f / 60.0f;
	while (remainingTime > 0.0f)
	{
		Vector3 displacement = currentPlayer->m_Velocity * remainingTime;
		currentPlayer->GetBoundingPoints();

		const Vector3* playerBoundingPoints = currentPlayer->GetBoundingPoints();
		for (size_t boundingPointIndex = 0; boundingPointIndex < NUMBER_OF_BOUNDING_POINTS; ++boundingPointIndex)
		{
			Vector3 currentBoundingPoint = playerBoundingPoints[boundingPointIndex];

			RaycastResult3D currentResult = GetAWRaycast(currentBoundingPoint, currentBoundingPoint + displacement);
			raycastResults[boundingPointIndex] = currentResult;
		}

		RaycastResult3D shortestRaycastResult;
		size_t bestRaycastResultIndex = 255U;

		for (size_t raycastResultIndex = 0; raycastResultIndex < NUMBER_OF_BOUNDING_POINTS; ++raycastResultIndex)
		{
			RaycastResult3D currentResult = raycastResults[raycastResultIndex];
			if (currentResult.m_impactedSolidBlock)
			{
				if (currentResult.m_impactFraction < shortestRaycastResult.m_impactFraction)
				{
					shortestRaycastResult = currentResult;
					bestRaycastResultIndex = raycastResultIndex;
				}
			}
		}

		if (bestRaycastResultIndex == 255U)
		{
			currentPlayer->m_Position += displacement;
			currentPlayer->Update();
			remainingTime = 0.0f;
			return;
		}
		else
		{
			currentPlayer->m_Position += shortestRaycastResult.m_impactedPosition - playerBoundingPoints[bestRaycastResultIndex];
			currentPlayer->Update();

			if (shortestRaycastResult.m_surfaceNormal.X != 0.0f)
			{
				currentPlayer->m_Velocity.X = 0.0f;
			}
			else if (shortestRaycastResult.m_surfaceNormal.Y != 0.0f)
			{
				currentPlayer->m_Velocity.Y = 0.0f;
			}
			else if (shortestRaycastResult.m_surfaceNormal.Z != 0.0f)
			{
				currentPlayer->m_Velocity.Z = 0.0f;
			}

			remainingTime -= remainingTime * shortestRaycastResult.m_impactFraction;

			if (currentPlayer->m_Velocity == Vector3::ZERO)
			{
				remainingTime = 0.0f;
				return;
			}
		}
	}
}



void World::ApplyCorrectivePhysics(Player* currentPlayer)
{
	AABB3 playerBounds = currentPlayer->GetBoundingBox();

	BlockInfo overlappingBlocks[NUMBER_OF_BOUNDING_POINTS];
	size_t numberOfOverlappingBlocks = 0U;
	GetPlayerOverlappingBlocks(currentPlayer, overlappingBlocks, numberOfOverlappingBlocks);

	float shortestCorrectionDistance = 1000.0f;
	Vector3 shortestCorrection = Vector3::ZERO;

	for (size_t blockIndex = 0; blockIndex < numberOfOverlappingBlocks; ++blockIndex)
	{
		BlockInfo currentOverlappingBlock = overlappingBlocks[blockIndex];
		if (currentOverlappingBlock.GetChunk() != nullptr)
		{
			if (currentOverlappingBlock.GetBlock()->IsSolid())
			{
				AABB3 blockBoundingBox = GetBoundingBoxForBlockInfo(currentOverlappingBlock);
				if (DoAABB3sIntersect(playerBounds, blockBoundingBox))
				{
					Vector3 newCorrection = GetShortestCorrection(playerBounds, blockBoundingBox);
					float newCorrectionDistance = 0.0f;

					if (newCorrection.X != 0.0f)
					{
						newCorrectionDistance = AbsoluteValue(newCorrection.X);
					}
					else if (newCorrection.Y != 0.0f)
					{
						newCorrectionDistance = AbsoluteValue(newCorrection.Y);
					}
					else if (newCorrection.Z != 0.0f)
					{
						newCorrectionDistance = AbsoluteValue(newCorrection.Z);
					}

					if (newCorrectionDistance < shortestCorrectionDistance)
					{
						shortestCorrectionDistance = newCorrectionDistance;
						shortestCorrection = newCorrection;
					}
				}
			}
		}
	}

	currentPlayer->m_Position += shortestCorrection;
}



Vector3 World::GetShortestCorrection(const AABB3& playerBounds, const AABB3& blockBoundingBox)
{
	float minimumCorrectionDistance = 1000.0f;

	float eastCorrectionDistance = blockBoundingBox.maximums.X - playerBounds.minimums.X;
	if (eastCorrectionDistance > 0.0f)
	{
		if (eastCorrectionDistance < minimumCorrectionDistance)
		{
			minimumCorrectionDistance = eastCorrectionDistance;
		}
	}

	float westCorrectionDistance = playerBounds.maximums.X - blockBoundingBox.minimums.X;
	if (westCorrectionDistance > 0.0f)
	{
		if (westCorrectionDistance < minimumCorrectionDistance)
		{
			minimumCorrectionDistance = westCorrectionDistance;
		}
	}

	float northCorrectionDistance = blockBoundingBox.maximums.Y - playerBounds.minimums.Y;
	if (northCorrectionDistance > 0.0f)
	{
		if (northCorrectionDistance < minimumCorrectionDistance)
		{
			minimumCorrectionDistance = northCorrectionDistance;
		}
	}

	float southCorrectionDistance = playerBounds.maximums.Y - blockBoundingBox.minimums.Y;
	if (southCorrectionDistance > 0.0f)
	{
		if (southCorrectionDistance < minimumCorrectionDistance)
		{
			minimumCorrectionDistance = southCorrectionDistance;
		}
	}

	float topCorrectionDistance = blockBoundingBox.maximums.Z - playerBounds.minimums.Z;
	if (topCorrectionDistance > 0.0f)
	{
		if (topCorrectionDistance < minimumCorrectionDistance)
		{
			minimumCorrectionDistance = topCorrectionDistance;
		}
	}

	float bottomCorrectionDistance = playerBounds.maximums.Z - blockBoundingBox.minimums.Z;
	if (bottomCorrectionDistance > 0.0f)
	{
		if (bottomCorrectionDistance < minimumCorrectionDistance)
		{
			minimumCorrectionDistance = bottomCorrectionDistance;
		}
	}

	if (minimumCorrectionDistance == eastCorrectionDistance)
	{
		return Vector3(eastCorrectionDistance + COLLISION_CORRECTION_VALUE, 0.0f, 0.0f);
	}
	else if (minimumCorrectionDistance == westCorrectionDistance)
	{
		return Vector3(-westCorrectionDistance - COLLISION_CORRECTION_VALUE, 0.0f, 0.0f);
	}
	else if (minimumCorrectionDistance == northCorrectionDistance)
	{
		return Vector3(0.0f, northCorrectionDistance + COLLISION_CORRECTION_VALUE, 0.0f);
	}
	else if (minimumCorrectionDistance == southCorrectionDistance)
	{
		return Vector3(0.0f, -southCorrectionDistance - COLLISION_CORRECTION_VALUE, 0.0f);
	}
	else if (minimumCorrectionDistance == topCorrectionDistance)
	{
		return Vector3(0.0f, 0.0f, topCorrectionDistance + COLLISION_CORRECTION_VALUE);
	}
	else if (minimumCorrectionDistance == bottomCorrectionDistance)
	{
		return Vector3(0.0f, 0.0f, -bottomCorrectionDistance - COLLISION_CORRECTION_VALUE);
	}
	else
	{
		return Vector3::ZERO;
	}
}



void World::UpdatePlayerMovementAndPhysics(Player* currentPlayer, uint8_t currentPlayerID, float deltaTimeInSeconds)
{
	bool playerIsOnTheGround = PlayerIsOnTheGround(currentPlayer);

	currentPlayer->Move(currentPlayerID, deltaTimeInSeconds, playerIsOnTheGround);
	currentPlayer->Update();

	uint8_t currentPlayerPhysicsMode = currentPlayer->m_CurrentPhysicsMode;
	if (currentPlayerPhysicsMode == WALKING_MODE || currentPlayerPhysicsMode == FLYING_MODE)
	{
		ApplyCorrectivePhysics(currentPlayer);
		ApplyPreventativePhysics(currentPlayer);
	}
	currentPlayer->Update();
}



void World::PlayPlayerDeathScream(float panLevel)
{
	SoundID deathScream = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/PlayerScream.ogg");
	AudioSystem::SingletonInstance()->PlaySound(deathScream, FORWARD_PLAYBACK_MODE, 1.0f, panLevel);
}



void World::SpawnNewEnemy(float& enemySpawnTime)
{
	if (enemySpawnTime >= 3.0f)
	{
		if (m_NumberOfAliveEnemies < MAXIMUM_NUMBER_OF_ENEMIES)
		{
			const int MAXIMUM_NUMBER_OF_TRIES = 10;

			Player* currentPlayer = m_PlayerOne;
			Vector3 idealEnemyPosition = Vector3::ZERO;
			bool foundIdealPosition = false;

			if (g_TwoPlayerMode)
			{
				int randomPlayerID = rand() % 2;
				currentPlayer = (randomPlayerID == 0) ? m_PlayerOne : m_PlayerTwo;
			}

			for (int tryIndex = 0; tryIndex < MAXIMUM_NUMBER_OF_TRIES; ++tryIndex)
			{
				int randomRadiusAroundPlayer = rand() % 33 + 64;
				int randomAngleAroundPlayer = rand() % 360;

				float radius = static_cast<float>(randomRadiusAroundPlayer);
				float angle = static_cast<float>(randomAngleAroundPlayer);

				Vector2 possiblePositionXY = ConvertToCartesian(Vector2(radius, angle));
				possiblePositionXY.X += currentPlayer->m_Position.X;
				possiblePositionXY.Y += currentPlayer->m_Position.Y;

				float groundHeight = GetColumnHeightForWorldXYCoordinates(possiblePositionXY);
				if (groundHeight == 0.0f)
				{
					continue;
				}

				float possibleHeight = groundHeight + (BLOCK_HEIGHT * 4.0f);
				Vector3 possibleEnemyPosition = Vector3(possiblePositionXY.X, possiblePositionXY.Y, possibleHeight);

				if (tryIndex > MAXIMUM_NUMBER_OF_TRIES / 2)
				{
					Vector3 directionToPlayerOne = possibleEnemyPosition - m_PlayerOne->m_Position;
					if (Vector3::DotProduct(directionToPlayerOne, m_PlayerOne->m_ForwardXYZ) < 0.0f)
					{
						if (g_TwoPlayerMode)
						{
							Vector3 directionToPlayerTwo = possibleEnemyPosition - m_PlayerTwo->m_Position;
							if (Vector3::DotProduct(directionToPlayerTwo, m_PlayerTwo->m_ForwardXYZ) < 0.0f)
							{
								foundIdealPosition = true;
							}
						}
						else
						{
							foundIdealPosition = true;
						}
					}
				}

				if (!foundIdealPosition)
				{
					RaycastResult3D raycastToPlayerOne = GetAWRaycast(possibleEnemyPosition, m_PlayerOne->m_Position);

					if (g_TwoPlayerMode)
					{
						RaycastResult3D raycastToPlayerTwo = GetAWRaycast(possibleEnemyPosition, m_PlayerTwo->m_Position);
						if (raycastToPlayerOne.m_impactedSolidBlock && raycastToPlayerTwo.m_impactedSolidBlock)
						{
							foundIdealPosition = true;
						}
					}
					else
					{
						if (raycastToPlayerOne.m_impactedSolidBlock)
						{
							foundIdealPosition = true;
						}
					}
				}

				idealEnemyPosition = possibleEnemyPosition;

				if (foundIdealPosition)
				{
					break;
				}
			}

			if (foundIdealPosition)
			{
				Enemy* spawnedEnemy = m_EnemyPool.AllocateObjectFromPool();
				spawnedEnemy->m_Position = idealEnemyPosition;

				size_t availableIndex = 0U;
				bool availableIndexFound = false;
				for (size_t enemyIndex = 0; enemyIndex < MAXIMUM_NUMBER_OF_ENEMIES; ++enemyIndex)
				{
					if (m_AllEnemies[enemyIndex] == nullptr)
					{
						availableIndex = enemyIndex;
						availableIndexFound = true;
						break;
					}
				}

				if (availableIndexFound)
				{
					m_AllEnemies[availableIndex] = spawnedEnemy;
					++m_NumberOfAliveEnemies;
					enemySpawnTime = 0.0f;
				}
				else
				{
					m_EnemyPool.DeallocateObjectToPool(spawnedEnemy);
				}
			}
		}
	}
}



void World::DespawnOutdatedEnemies()
{
	for (size_t enemyIndex = 0; enemyIndex < MAXIMUM_NUMBER_OF_ENEMIES; ++enemyIndex)
	{
		Enemy* currentEnemy = m_AllEnemies[enemyIndex];
		if (currentEnemy != nullptr)
		{
			IntVector2 currentEnemyChunkCoordinates = GetChunkCoordinatesForWorldCoordinates(currentEnemy->m_Position);
			if (FindActiveChunkWithCoordinates(currentEnemyChunkCoordinates) == nullptr)
			{
				m_EnemyPool.DeallocateObjectToPool(currentEnemy);
				m_AllEnemies[enemyIndex] = nullptr;

				--m_NumberOfAliveEnemies;
			}
		}
	}
}



bool World::PlayerIsInSight(const Vector3& startPosition, const Vector3& endPosition)
{
	if (CalculateEuclidianSquaredDistanceIn3D(startPosition, endPosition) >= ENEMY_SQUARED_RANGE_OF_VIEW)
	{
		return false;
	}
	else
	{
		RaycastResult3D raycastResult = GetAWRaycast(startPosition, endPosition);

		if (raycastResult.m_impactedSolidBlock)
		{
			return false;
		}
	}

	return true;
}



bool World::FocusOnNearestVisiblePlayer(Enemy* currentEnemy)
{
	bool playerOneCanBeSeen = false;
	bool playerTwoCanBeSeen = false;

	Vector3 enemyPosition = currentEnemy->m_Position;

	Vector3 playerOnePosition = m_PlayerOne->m_Position;
	if (m_PlayerOne->IsAlive())
	{
		playerOneCanBeSeen = PlayerIsInSight(enemyPosition, playerOnePosition);
	}

	if (g_TwoPlayerMode)
	{
		Vector3 playerTwoPosition = m_PlayerTwo->m_Position;
		if (m_PlayerTwo->IsAlive())
		{
			playerTwoCanBeSeen = PlayerIsInSight(enemyPosition, playerTwoPosition);
		}

		if (playerOneCanBeSeen && playerTwoCanBeSeen)
		{
			float squaredDistanceToPlayerOne = CalculateEuclidianSquaredDistanceIn3D(enemyPosition, playerOnePosition);
			float squaredDistanceToPlayerTwo = CalculateEuclidianSquaredDistanceIn3D(enemyPosition, playerTwoPosition);

			float nearestSquaredDistance = GetMinimumOfTwoFloats(squaredDistanceToPlayerOne, squaredDistanceToPlayerTwo);
			
			if (nearestSquaredDistance == squaredDistanceToPlayerOne)
			{
				currentEnemy->FocusOnPlayer(playerOnePosition);
				return true;
			}
			else if (nearestSquaredDistance == squaredDistanceToPlayerTwo)
			{
				currentEnemy->FocusOnPlayer(playerTwoPosition);
				return true;
			}
		}
		else if (playerTwoCanBeSeen)
		{
			currentEnemy->FocusOnPlayer(playerTwoPosition);
			return true;
		}
	}

	if (playerOneCanBeSeen && !playerTwoCanBeSeen)
	{
		currentEnemy->FocusOnPlayer(playerOnePosition);
		return true;
	}

	return false;
}



void World::FirePlayerBullets(Player* currentPlayer, uint8_t currentPlayerID, float deltaTimeInSeconds)
{
	currentPlayer->m_RateOfFire += deltaTimeInSeconds;

	if (InputSystem::SingletonInstance()->TriggerPosition(currentPlayerID, RIGHT_TRIGGER) > 0.6f)
	{
		if (currentPlayer->m_RateOfFire >= PLAYER_RATE_OF_FIRE)
		{
			Bullet* playerBullet = new Bullet(currentPlayer->m_Position + EYE_LEVEL_VIEW, currentPlayer->m_ForwardXYZ, PLAYER_BULLET_TYPE);
			m_AllBullets.insert(playerBullet);
			//Play Bullet Sound
			currentPlayer->m_RateOfFire = 0.0f;
		}
	}
}



void World::FireEnemyBullets(Enemy* currentEnemy, float deltaTimeInSeconds)
{
	currentEnemy->m_RateOfFire += deltaTimeInSeconds;

	if (currentEnemy->m_RateOfFire >= ENEMY_RATE_OF_FIRE)
	{
		Bullet* enemyBullet = new Bullet(currentEnemy->m_Position, currentEnemy->m_ForwardXYZ, ENEMY_BULLET_TYPE);
		m_AllBullets.insert(enemyBullet);
		//Play Bullet Sound
		currentEnemy->m_RateOfFire = 0.0f;
	}
}



IntVector2 World::GetChunkCoordinatesForWorldCoordinates(const Vector3& worldCoordinates) const
{
	float flooredworldCoordinatesX = RoundDownToFloorValue(worldCoordinates.X);
	float flooredworldCoordinatesY = RoundDownToFloorValue(worldCoordinates.Y);

	int chunkCoordinatesX = static_cast<int>(flooredworldCoordinatesX);
	int chunkCoordinatesY = static_cast<int>(flooredworldCoordinatesY);

	chunkCoordinatesX = ShiftBitsRight(chunkCoordinatesX, 4);
	chunkCoordinatesY = ShiftBitsRight(chunkCoordinatesY, 4);

	return IntVector2(chunkCoordinatesX, chunkCoordinatesY);
}



Vector2 World::GetChunkWorldCentreForChunkCoordinates(const IntVector2& chunkCoordinates)
{
	int X = ShiftBitsLeft(chunkCoordinates.X, 4) + ShiftBitsLeft(1, 3);
	int Y = ShiftBitsLeft(chunkCoordinates.Y, 4) + ShiftBitsLeft(1, 3);

	float chunkCentreX = static_cast<float>(X);
	float chunkCentreY = static_cast<float>(Y);

	return Vector2(chunkCentreX, chunkCentreY);
}



Vector3 World::GetBlockWorldCentreForBlockInfo(BlockInfo currentBlockInfo)
{
	Chunk* currentChunk = currentBlockInfo.GetChunk();
	if (currentChunk != nullptr)
	{
		IntVector3 blockWorldCoordinates = currentChunk->GetBlockWorldCoordinatesForLocalIndex(currentBlockInfo.GetBlockIndex());

		float blockCentreX = static_cast<float>(blockWorldCoordinates.X) + (BLOCK_WIDTH / 2.0f);
		float blockCentreY = static_cast<float>(blockWorldCoordinates.Y) + (BLOCK_LENGTH / 2.0f);
		float blockCentreZ = static_cast<float>(blockWorldCoordinates.Z) + (BLOCK_HEIGHT / 2.0f);

		return Vector3(blockCentreX, blockCentreY, blockCentreZ);
	}

	return Vector3::ZERO;
}



Chunk* World::GetChunkAtWorldCoordinates(const Vector3& worldCoordinates)
{
	IntVector2 chunkCoordinates = GetChunkCoordinatesForWorldCoordinates(worldCoordinates);

	return FindActiveChunkWithCoordinates(chunkCoordinates);
}



BlockInfo World::GetBlockInfoForWorldCoordinates(const Vector3& blockWorldCoordinates)
{
	Chunk* chunkOfBlock = GetChunkAtWorldCoordinates(blockWorldCoordinates);
	if (chunkOfBlock != nullptr)
	{
		int blockIndex = chunkOfBlock->GetLocalIndexForBlockWorldCoordinates(blockWorldCoordinates);
		if (blockIndex >= 0 && blockIndex < NUMBER_OF_BLOCKS_PER_CHUNK)
		{
			return BlockInfo(chunkOfBlock, blockIndex);
		}
	}

	return BlockInfo(nullptr, 0);
}



Block* World::GetBlockForWorldCoordinates(const Vector3& blockWorldCoordinates)
{
	BlockInfo currentBlockInfo = GetBlockInfoForWorldCoordinates(blockWorldCoordinates);

	return currentBlockInfo.GetBlock();
}



float World::GetColumnHeightForWorldXYCoordinates(const Vector2& worldXYCoordinates)
{
	for (int heightIndex = NUMBER_OF_BLOCKS_Z - 1; heightIndex >= 0; --heightIndex)
	{
		float worldHeight = static_cast<float>(heightIndex);

		BlockInfo currentBlockInfo = GetBlockInfoForWorldCoordinates(Vector3(worldXYCoordinates.X, worldXYCoordinates.Y, worldHeight));
		Block* currentBlock = currentBlockInfo.GetBlock();

		if (currentBlock != nullptr)
		{
			if (currentBlock->IsSolid())
			{
				return worldHeight;
			}
		}
	}

	return 0.0f;
}



AABB3 World::GetBoundingBoxForBlockInfo(BlockInfo currentBlockInfo)
{
	Vector3 blockCentre = GetBlockWorldCentreForBlockInfo(currentBlockInfo);

	Vector3 blockMinimums = blockCentre - Vector3(BLOCK_WIDTH / 2.0f, BLOCK_LENGTH / 2.0f, BLOCK_HEIGHT / 2.0f);
	Vector3 blockMaximums = blockMinimums + Vector3(BLOCK_WIDTH, BLOCK_LENGTH, BLOCK_HEIGHT);

	return AABB3(blockMinimums, blockMaximums);
}



void World::GetPlayerOverlappingBlocks(Player* currentPlayer, BlockInfo* overlappingBlocks, size_t& numberOfOverlappingBlocks)
{
	const Vector3* playerBoundingPoints = currentPlayer->GetBoundingPoints();
	for (size_t boundingPointIndex = 0; boundingPointIndex < NUMBER_OF_BOUNDING_POINTS; ++boundingPointIndex)
	{
		Vector3 currentBoundingPoint = playerBoundingPoints[boundingPointIndex];
		BlockInfo currentOverlappingBlock = GetBlockInfoForWorldCoordinates(currentBoundingPoint);

		if (currentOverlappingBlock.GetBlock() != nullptr)
		{
			overlappingBlocks[numberOfOverlappingBlocks] = currentOverlappingBlock;
			++numberOfOverlappingBlocks;
		}
	}
}



bool World::PlayerOverlapsSelectedBlock(BlockInfo selectedBlockInfo, const BlockInfo* overlappingBlocks, size_t numberOfOverlappingBlocks)
{
	for (size_t blockIndex = 0; blockIndex < numberOfOverlappingBlocks; ++blockIndex)
	{
		BlockInfo currentOverlappingBlock = overlappingBlocks[blockIndex];
		if (currentOverlappingBlock == selectedBlockInfo)
		{
			return true;
		}
	}

	return false;
}



void World::InitializeAllLights()
{
	for (size_t lightIndex = 0; lightIndex < MAXIMUM_NUMBER_OF_LIGHTS; ++lightIndex)
	{
		m_AllWorldLights[lightIndex] = nullptr;
	}
}



void World::CreatePlayerLights()
{
	float nearDistance = static_cast<float>((NUMBER_OF_BLOCKS_X + NUMBER_OF_BLOCKS_Y) >> 2);
	float farDistance = nearDistance * 2.0f;
	
	Light* playerOneLight = new Light(Light::LOCAL_POINT_LIGHT);
	playerOneLight->SetLightPosition(m_PlayerOne->m_Position);
	playerOneLight->SetLightColor(Vector3::ONE);
	
	playerOneLight->SetNearDistance(nearDistance);
	playerOneLight->SetFarDistance(farDistance);
	
	playerOneLight->SetNearFactor(1.0f);
	playerOneLight->SetFarFactor(0.0f);
	
	m_AllWorldLights[0] = playerOneLight;

	if (g_TwoPlayerMode)
	{
		Light* playerTwoLight = new Light(Light::LOCAL_POINT_LIGHT);
		playerTwoLight->SetLightPosition(m_PlayerTwo->m_Position);
		playerTwoLight->SetLightColor(Vector3::ONE);

		playerTwoLight->SetNearDistance(nearDistance);
		playerTwoLight->SetFarDistance(farDistance);

		playerTwoLight->SetNearFactor(1.0f);
		playerTwoLight->SetFarFactor(0.0f);

		m_AllWorldLights[1] = playerTwoLight;
	}
}



void World::CreateSunLight()
{
	float lightHeight = static_cast<float>(NUMBER_OF_BLOCKS_Z);

	Light* sunLight = new Light(Light::GLOBAL_POINT_LIGHT);
	sunLight->SetLightPosition(Vector3(0.0f, 0.0f, lightHeight));
	sunLight->SetLightColor(Vector3::ONE);

	m_AllWorldLights[2] = sunLight;
}



void World::DestroyAllLights()
{
	for (size_t lightIndex = 0; lightIndex < MAXIMUM_NUMBER_OF_LIGHTS; ++lightIndex)
	{
		delete m_AllWorldLights[lightIndex];
		m_AllWorldLights[lightIndex] = nullptr;
	}
}



void World::UpdatePlayerLights()
{
	m_AllWorldLights[0]->SetLightPosition(m_PlayerOne->m_Position);
	if (g_TwoPlayerMode)
	{
		m_AllWorldLights[1]->SetLightPosition(m_PlayerTwo->m_Position);
	}
}



void World::UpdatePlayerAngleToSun(float deltaTimeInSeconds)
{
	float dayDurationInSeconds = DAY_DURATION_IN_MINUTES * 60.0f;
	float rotationPerSecondInDegrees = 360.0f / dayDurationInSeconds;
	float rotationPerUpdate = rotationPerSecondInDegrees * deltaTimeInSeconds;

	m_PlayerAngleToSun += rotationPerUpdate;
	m_PlayerAngleToSun = WrapAroundCircularRange(m_PlayerAngleToSun, 0.0f, 360.0f);

	m_SkyRotation += (rotationPerUpdate * 2.0f);
	m_SkyRotation = WrapAroundCircularRange(m_SkyRotation, 0.0f, 360.0f);

	Vector3 initialColor = Vector3::ZERO;
	Vector3 finalColor = Vector3::ZERO;
	float interpolationFactor = 0.0f;

	const Vector3 DAY_COLOR = Vector3(1.0f, 1.0f, 1.0f);
	const Vector3 HORIZON_COLOR = Vector3(0.933f, 0.867f, 0.51f);
	const Vector3 NIGHT_COLOR = Vector3(0.0f, 0.0f, 0.0f);

	if (m_PlayerAngleToSun >= 0.0f && m_PlayerAngleToSun < 90.0f)
	{
		initialColor = HORIZON_COLOR;
		finalColor = DAY_COLOR;
		interpolationFactor = m_PlayerAngleToSun / 90.0f;
	}
	else if (m_PlayerAngleToSun >= 90.0f && m_PlayerAngleToSun < 180.0f)
	{
		initialColor = DAY_COLOR;
		finalColor = HORIZON_COLOR;
		interpolationFactor = (m_PlayerAngleToSun - 90.0f) / 90.0f;
	}
	else if (m_PlayerAngleToSun >= 180.0f && m_PlayerAngleToSun < 270.0f)
	{
		initialColor = HORIZON_COLOR;
		finalColor = NIGHT_COLOR;
		interpolationFactor = (m_PlayerAngleToSun - 180.0f) / 90.0f;
	}
	else if (m_PlayerAngleToSun >= 270.0f && m_PlayerAngleToSun < 360.0f)
	{
		initialColor = NIGHT_COLOR;
		finalColor = HORIZON_COLOR;
		interpolationFactor = (m_PlayerAngleToSun - 270.0f) / 90.0f;
	}

	m_AllWorldLights[2]->SetLightColor(LinearlyInterpolateIn3D(initialColor, finalColor, interpolationFactor));
}



void World::UpdateSunLight(uint8_t currentPlayerID) const
{
	const float DISTANCE_TO_SUN = (((ACTIVE_CHUNK_RADIUS + FLUSH_CHUNK_RADIUS) >> 1) * ((NUMBER_OF_BLOCKS_X + NUMBER_OF_BLOCKS_Y) >> 1));
	Vector3 sunCurrentPosition = Vector3::ZERO;

	float sunAngleInRadians = ConvertToRadians(m_PlayerAngleToSun);
	Vector2 sunRelativePosition = ConvertToCartesian(Vector2(DISTANCE_TO_SUN, sunAngleInRadians));
	
	switch (currentPlayerID)
	{
	case PLAYER_ONE:
		sunCurrentPosition = m_PlayerOne->m_Position + Vector3(sunRelativePosition.X, 0.0f, sunRelativePosition.Y);
		break;

	case PLAYER_TWO:
		sunCurrentPosition = m_PlayerTwo->m_Position + Vector3(sunRelativePosition.X, 0.0f, sunRelativePosition.Y);
		break;

	default:
		break;
	}

	m_AllWorldLights[2]->SetLightPosition(sunCurrentPosition);
}



void World::SetLightDataToShaderProgram(const Camera3D* playerCamera) const
{
	Vector3 lightPosition[MAXIMUM_NUMBER_OF_LIGHTS];
	Vector3 lightColor[MAXIMUM_NUMBER_OF_LIGHTS];

	float nearDistance[MAXIMUM_NUMBER_OF_LIGHTS] = {};
	float farDistance[MAXIMUM_NUMBER_OF_LIGHTS] = {};
	float nearFactor[MAXIMUM_NUMBER_OF_LIGHTS] = {};
	float farFactor[MAXIMUM_NUMBER_OF_LIGHTS] = {};

	Vector3 lightDirection[MAXIMUM_NUMBER_OF_LIGHTS];
	float directionalFactor[MAXIMUM_NUMBER_OF_LIGHTS] = {};

	float innerAngle[MAXIMUM_NUMBER_OF_LIGHTS] = {};
	float outerAngle[MAXIMUM_NUMBER_OF_LIGHTS] = {};
	float innerFactor[MAXIMUM_NUMBER_OF_LIGHTS] = {};
	float outerFactor[MAXIMUM_NUMBER_OF_LIGHTS] = {};

	memset(lightPosition, 0, MAXIMUM_NUMBER_OF_LIGHTS * sizeof(Vector3));
	memset(lightColor, 0, MAXIMUM_NUMBER_OF_LIGHTS * sizeof(Vector3));
	memset(lightPosition, 0, MAXIMUM_NUMBER_OF_LIGHTS * sizeof(Vector3));

	for (size_t lightIndex = 0; lightIndex < MAXIMUM_NUMBER_OF_LIGHTS; ++lightIndex)
	{
		Light* currentLight = m_AllWorldLights[lightIndex];
		if (currentLight != nullptr)
		{
			lightPosition[lightIndex] = currentLight->GetLightPosition();
			lightColor[lightIndex] = currentLight->GetLightColor();

			nearDistance[lightIndex] = currentLight->GetNearDistance();
			farDistance[lightIndex] = currentLight->GetFarDistance();
			nearFactor[lightIndex] = currentLight->GetNearFactor();
			farFactor[lightIndex] = currentLight->GetFarFactor();

			lightDirection[lightIndex] = currentLight->GetLightDirection();
			directionalFactor[lightIndex] = currentLight->GetDirectionalFactor();

			innerAngle[lightIndex] = currentLight->GetInnerAngle();
			outerAngle[lightIndex] = currentLight->GetOuterAngle();
			innerFactor[lightIndex] = currentLight->GetInnerFactor();
			outerFactor[lightIndex] = currentLight->GetOuterFactor();
		}
	}

	m_ChunkMaterial->SetFloatToShaderProgram3D("g_LightPosition[0]", (float*)&lightPosition[0], MAXIMUM_NUMBER_OF_LIGHTS);
	m_ChunkMaterial->SetFloatToShaderProgram3D("g_LightColor[0]", (float*)&lightColor[0], MAXIMUM_NUMBER_OF_LIGHTS);

	m_ChunkMaterial->SetFloatToShaderProgram1D("g_NearDistance[0]", (float*)&nearDistance[0], MAXIMUM_NUMBER_OF_LIGHTS);
	m_ChunkMaterial->SetFloatToShaderProgram1D("g_FarDistance[0]", (float*)&farDistance[0], MAXIMUM_NUMBER_OF_LIGHTS);
	m_ChunkMaterial->SetFloatToShaderProgram1D("g_NearFactor[0]", (float*)&nearFactor[0], MAXIMUM_NUMBER_OF_LIGHTS);
	m_ChunkMaterial->SetFloatToShaderProgram1D("g_FarFactor[0]", (float*)&farFactor[0], MAXIMUM_NUMBER_OF_LIGHTS);

	m_ChunkMaterial->SetFloatToShaderProgram3D("g_LightDirection[0]", (float*)&lightDirection[0], MAXIMUM_NUMBER_OF_LIGHTS);
	m_ChunkMaterial->SetFloatToShaderProgram1D("g_DirectionalFactor[0]", (float*)&directionalFactor[0], MAXIMUM_NUMBER_OF_LIGHTS);

	m_ChunkMaterial->SetFloatToShaderProgram1D("g_InnerAngle[0]", (float*)&innerAngle[0], MAXIMUM_NUMBER_OF_LIGHTS);
	m_ChunkMaterial->SetFloatToShaderProgram1D("g_OuterAngle[0]", (float*)&outerAngle[0], MAXIMUM_NUMBER_OF_LIGHTS);
	m_ChunkMaterial->SetFloatToShaderProgram1D("g_InnerFactor[0]", (float*)&innerFactor[0], MAXIMUM_NUMBER_OF_LIGHTS);
	m_ChunkMaterial->SetFloatToShaderProgram1D("g_OuterFactor[0]", (float*)&outerFactor[0], MAXIMUM_NUMBER_OF_LIGHTS);

	float specularPower = 32.0f;
	m_ChunkMaterial->SetFloatToShaderProgram1D("g_SpecularPower", (float*)&specularPower);
	m_ChunkMaterial->SetFloatToShaderProgram3D("g_CameraPosition", (float*)&playerCamera->m_Position);
}