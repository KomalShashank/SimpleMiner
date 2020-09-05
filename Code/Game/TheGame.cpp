#include "Game/TheGame.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>



const float FOV_ANGLE_IN_DEGREES = 60.0f;
const float ASPECT_RATIO = VIEW_WIDTH / VIEW_HEIGHT;
const float PERSPECTIVE_NEAR_DISTANCE = 0.1f;
const float PERSPECTIVE_FAR_DISTANCE = 1000.0f;

const float ORTHOGRAPHIC_NEAR_DISTANCE = -1.0f;
const float ORTHOGRAPHIC_FAR_DISTANCE = 1.0f;



TheGame* g_TheGame = nullptr;

CRITICAL_SECTION g_MusicLoadingCriticalSection;
bool g_MusicFinishedLoading = false;



HUDTileDefinition HUDTileDefinition::s_HUDTileDefinitions[NUMBER_OF_HUD_TILES];



HUDTileDefinition::HUDTileDefinition()
{

}



void HUDTileDefinition::InitializeHUDTileDefinitions(const SpriteSheet& tileSpriteSheet)
{
	const float tileOffsetFromLeftEdge = (g_TwoPlayerMode) ? 0.75f : 3.75f;
	const float tileOffsetFromBottomEdge = 0.5f;
	const Vector2 tileDimensions = Vector2(0.5f, 0.5f);
	const float spacing = (g_TwoPlayerMode) ? 0.75f : 1.0f;

	Vector2 tileMinimums;
	Vector2 tileMaximums;

	tileMinimums = Vector2(tileOffsetFromLeftEdge, tileOffsetFromBottomEdge);
	tileMaximums = tileMinimums + tileDimensions;
	s_HUDTileDefinitions[GRASS_BLOCK - 1].m_tileBounds = AABB2(tileMinimums, tileMaximums);
	s_HUDTileDefinitions[GRASS_BLOCK - 1].m_tileTextureBounds = tileSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(1, 0));

	tileMinimums = tileMinimums + Vector2(spacing, 0.0f);
	tileMaximums = tileMinimums + tileDimensions;
	s_HUDTileDefinitions[DIRT_BLOCK - 1].m_tileBounds = AABB2(tileMinimums, tileMaximums);
	s_HUDTileDefinitions[DIRT_BLOCK - 1].m_tileTextureBounds = tileSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(3, 0));

	tileMinimums = tileMinimums + Vector2(spacing, 0.0f);
	tileMaximums = tileMinimums + tileDimensions;
	s_HUDTileDefinitions[STONE_BLOCK - 1].m_tileBounds = AABB2(tileMinimums, tileMaximums);
	s_HUDTileDefinitions[STONE_BLOCK - 1].m_tileTextureBounds = tileSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(0, 1));

	tileMinimums = tileMinimums + Vector2(spacing, 0.0f);
	tileMaximums = tileMinimums + tileDimensions;
	s_HUDTileDefinitions[SAND_BLOCK - 1].m_tileBounds = AABB2(tileMinimums, tileMaximums);
	s_HUDTileDefinitions[SAND_BLOCK - 1].m_tileTextureBounds = tileSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(1, 1));

	tileMinimums = tileMinimums + Vector2(spacing, 0.0f);
	tileMaximums = tileMinimums + tileDimensions;
	s_HUDTileDefinitions[CLAY_BLOCK - 1].m_tileBounds = AABB2(tileMinimums, tileMaximums);
	s_HUDTileDefinitions[CLAY_BLOCK - 1].m_tileTextureBounds = tileSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(2, 1));

	tileMinimums = tileMinimums + Vector2(spacing, 0.0f);
	tileMaximums = tileMinimums + tileDimensions;
	s_HUDTileDefinitions[GLOWSTONE_BLOCK - 1].m_tileBounds = AABB2(tileMinimums, tileMaximums);
	s_HUDTileDefinitions[GLOWSTONE_BLOCK - 1].m_tileTextureBounds = tileSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(3, 1));

	tileMinimums = tileMinimums + Vector2(spacing, 0.0f);
	tileMaximums = tileMinimums + tileDimensions;
	s_HUDTileDefinitions[COBBLESTONE_BLOCK - 1].m_tileBounds = AABB2(tileMinimums, tileMaximums);
	s_HUDTileDefinitions[COBBLESTONE_BLOCK - 1].m_tileTextureBounds = tileSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(0, 2));

	tileMinimums = tileMinimums + Vector2(spacing, 0.0f);
	tileMaximums = tileMinimums + tileDimensions;
	s_HUDTileDefinitions[CLAY_BRICK_BLOCK - 1].m_tileBounds = AABB2(tileMinimums, tileMaximums);
	s_HUDTileDefinitions[CLAY_BRICK_BLOCK - 1].m_tileTextureBounds = tileSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(1, 2));

	tileMinimums = tileMinimums + Vector2(spacing, 0.0f);
	tileMaximums = tileMinimums + tileDimensions;
	s_HUDTileDefinitions[STONE_BRICK_BLOCK - 1].m_tileBounds = AABB2(tileMinimums, tileMaximums);
	s_HUDTileDefinitions[STONE_BRICK_BLOCK - 1].m_tileTextureBounds = tileSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(2, 2));
}



ScreenFragment::ScreenFragment() :
m_LinearOffset(Vector3::ZERO),
m_AngularOffset(0.0f)
{
	m_LinearVelocity = GetRandomLinearVelocity();
	m_AngularVelocity = GetRandomFloatWithinRange(0.0f, 1024.0f);
}



void ScreenFragment::GeneratePivotMatrices()
{
	float totalX = 0.0f;
	float totalY = 0.0f;

	for (Vertex3D currentVertex : m_Vertices)
	{
		totalX += currentVertex.m_Position.X;
		totalY += currentVertex.m_Position.Y;
	}

	float numberOfVertices = static_cast<float>(m_Vertices.size());

	Vector3 pivotPoint = Vector3(totalX / numberOfVertices, totalY / numberOfVertices, 0.0f);

	m_PivotMatrix = AdvancedRenderer::SingletonInstance()->GetTranslationMatrix(pivotPoint);
	m_InversePivotMatrix = AdvancedRenderer::SingletonInstance()->GetTranslationMatrix(pivotPoint.GetNegatedVector3());
}



void ScreenFragment::GetUpdatedVertices(float deltaTimeInSeconds, std::vector<Vertex3D>& fragmentVertices)
{
	Vector3 linearDisplacement = m_LinearVelocity * deltaTimeInSeconds;
	float angularDisplacement = m_AngularVelocity * deltaTimeInSeconds;

	m_LinearOffset += linearDisplacement;
	m_AngularOffset += angularDisplacement;

	Matrix4 translationMatrix = AdvancedRenderer::SingletonInstance()->GetTranslationMatrix(m_LinearOffset);
	Matrix4 rotationMatrix = AdvancedRenderer::SingletonInstance()->GetRotationMatrix(EulerAngles(0.0f, 0.0f, m_AngularOffset));

	Matrix4 modelMatrix = m_InversePivotMatrix * rotationMatrix * m_PivotMatrix * translationMatrix;

	for (Vertex3D currentVertex : m_Vertices)
	{
		Vertex3D transformedVertex = currentVertex;
		transformedVertex.m_Position = modelMatrix.TransformVector3(currentVertex.m_Position, 1.0f);
		fragmentVertices.push_back(transformedVertex);
	}
}



Vector3 ScreenFragment::GetRandomLinearVelocity() const
{
	float linearSpeedX = GetRandomFloatWithinRange(1.0f, 5.0f);
	float linearSpeedY = GetRandomFloatWithinRange(1.0f, 5.0f);

	int linearDirectionX = GetRandomIntWithinRange(0, 1);
	int linearDirectionY = GetRandomIntWithinRange(0, 1);

	float linearVelocityX = (linearDirectionX == 0) ? -linearSpeedX : linearSpeedX;
	float linearVelocityY = (linearDirectionY == 0) ? -linearSpeedY : linearSpeedY;

	return Vector3(linearVelocityX, linearVelocityY, 0.0f);
}



SoundID TheGame::s_MusicTracks[NUMBER_OF_MUSIC_TRACKS];



TheGame::TheGame() :
m_HUDFont(nullptr),
m_HUDSpriteSheet(nullptr),
m_MainMenu(nullptr),
m_World(nullptr),
m_SplitScreenPartitionMesh(nullptr),
m_PlayerOneFirstFBO(nullptr),
m_PlayerOneSecondFBO(nullptr),
m_PlayerTwoFirstFBO(nullptr),
m_PlayerTwoSecondFBO(nullptr),
m_FullScreenFBO(nullptr),
m_PlayerOneFBODepthMesh(nullptr),
m_PlayerOneFBOShatterMesh(nullptr),
m_PlayerTwoFBODepthMesh(nullptr),
m_PlayerTwoFBOShatterMesh(nullptr),
m_FullScreenFBOMesh(nullptr)
{
	SamplerData fontSamplerData = SamplerData(REPEAT_WRAP, REPEAT_WRAP, LINEAR_FILTER, NEAREST_FILTER);
	m_HUDFont = MonospaceFont::CreateOrGetMonospaceFont("Data/Fonts/SquirrelFixedFont.png", fontSamplerData);

	SamplerData textureSamplerData = SamplerData(REPEAT_WRAP, REPEAT_WRAP, LINEAR_FILTER, NEAREST_FILTER);
	m_HUDSpriteSheet = new SpriteSheet("Data/Images/HUDTextures.png", 4, 4, textureSamplerData);
	
	InitializeCriticalSection(&g_MusicLoadingCriticalSection);
	m_MusicLoadingThread = Thread::CreateNewThread(InitializeMusicTracks, nullptr);
	m_MusicLoadingThread->DetachThread();

	MainMenu::InitializeMainMenuTextures();
	m_MainMenu = new MainMenu();

	AdvancedRenderer::SingletonInstance()->EnableBackFaceCulling(true);
	InputSystem::SingletonInstance()->HideMouseCursor();
}



TheGame::~TheGame()
{
	delete m_MainMenu;
}



void TheGame::Update(float deltaTimeInSeconds)
{
	if (m_MainMenu != nullptr)
	{
		m_MainMenu->Update();

		if (g_LoadGame && m_World == nullptr)
		{
			LoadGame();
		}
	}

	if (m_World != nullptr)
	{
		bool musicHasLoaded;
		EnterCriticalSection(&g_MusicLoadingCriticalSection);
		{
			musicHasLoaded = g_MusicFinishedLoading;
		}
		LeaveCriticalSection(&g_MusicLoadingCriticalSection);

		if (musicHasLoaded)
		{
			if (m_MusicLoadingThread != nullptr)
			{
				Thread::DestroyThread(m_MusicLoadingThread);
			}
			
			PlayBackgroundMusic();
		}

		ToggleDebugMode();
		ToggleScreenshotMode();

		(!g_TwoPlayerMode) ? OnePlayerUpdateCall(deltaTimeInSeconds) : TwoPlayerUpdateCall(deltaTimeInSeconds);

		ExitToMainMenu(deltaTimeInSeconds);

		if (!g_LoadGame && m_MainMenu == nullptr)
		{
			UnloadGame();
		}
	}
}



void TheGame::Render() const
{
	AdvancedRenderer::SingletonInstance()->ClearScreen(RGBA::BLACK);

	if (m_MainMenu != nullptr)
	{
		m_MainMenu->Render();
	}

	if (m_World != nullptr)
	{
		(!g_TwoPlayerMode) ? OnePlayerRenderCall() : TwoPlayerRenderCall();
	}
}



void TheGame::OnePlayerUpdateCall(float deltaTimeInSeconds)
{
	if (m_World->m_PlayerOne->IsAlive())
	{
		CameraLook(m_PlayerOneCamera, PLAYER_ONE, deltaTimeInSeconds);
		ToggleCameraMode(m_PlayerOneCameraMode, PLAYER_ONE);
		SelectBlocks(m_PlayerOneSelectedBlock, PLAYER_ONE);
	}

	UpdatePlayerDirections(m_World->m_PlayerOne, m_PlayerOneCamera);

	m_World->Update(deltaTimeInSeconds);

	KillPlayerAccordingly(m_World->m_PlayerOne);
	ResurrectPlayerAccordingly(m_World->m_PlayerOne, PLAYER_ONE);

	UpdateCameraPosition(m_World->m_PlayerOne, m_PlayerOneCamera, m_PlayerOneCameraMode);

	if (!m_World->m_PlayerOne->IsAlive())
	{
		UpdatePlayerFBOMesh(m_PlayerOneFBOShatterMesh, m_PlayerOneFragments, deltaTimeInSeconds);
	}
}



void TheGame::TwoPlayerUpdateCall(float deltaTimeInSeconds)
{
	if (m_World->m_PlayerOne->IsAlive())
	{
		CameraLook(m_PlayerOneCamera, PLAYER_ONE, deltaTimeInSeconds);
		ToggleCameraMode(m_PlayerOneCameraMode, PLAYER_ONE);
		SelectBlocks(m_PlayerOneSelectedBlock, PLAYER_ONE);
	}

	if (m_World->m_PlayerTwo->IsAlive())
	{
		CameraLook(m_PlayerTwoCamera, PLAYER_TWO, deltaTimeInSeconds);
		ToggleCameraMode(m_PlayerTwoCameraMode, PLAYER_TWO);
		SelectBlocks(m_PlayerTwoSelectedBlock, PLAYER_TWO);
	}

	UpdatePlayerDirections(m_World->m_PlayerOne, m_PlayerOneCamera);
	UpdatePlayerDirections(m_World->m_PlayerTwo, m_PlayerTwoCamera);
	m_World->Update(deltaTimeInSeconds);

	KillPlayerAccordingly(m_World->m_PlayerOne);
	ResurrectPlayerAccordingly(m_World->m_PlayerOne, PLAYER_ONE);

	KillPlayerAccordingly(m_World->m_PlayerTwo);
	ResurrectPlayerAccordingly(m_World->m_PlayerTwo, PLAYER_TWO);

	UpdateCameraPosition(m_World->m_PlayerOne, m_PlayerOneCamera, m_PlayerOneCameraMode);
	UpdateCameraPosition(m_World->m_PlayerTwo, m_PlayerTwoCamera, m_PlayerTwoCameraMode);

	if (!m_World->m_PlayerOne->IsAlive())
	{
		UpdatePlayerFBOMesh(m_PlayerOneFBOShatterMesh, m_PlayerOneFragments, deltaTimeInSeconds);
	}

	if (!m_World->m_PlayerTwo->IsAlive())
	{
		UpdatePlayerFBOMesh(m_PlayerTwoFBOShatterMesh, m_PlayerTwoFragments, deltaTimeInSeconds);
	}
}



void TheGame::OnePlayerRenderCall() const
{
	AdvancedRenderer::SingletonInstance()->BindFrameBuffer(m_FullScreenFBO);
	AdvancedRenderer::SingletonInstance()->ClearScreen(RGBA::BLACK);
	AdvancedRenderer::SingletonInstance()->UnbindFrameBuffer();

	RenderPlayerOne();

	AdvancedRenderer::SingletonInstance()->CopyFrameBufferToBackBuffer(m_FullScreenFBO);
}



void TheGame::TwoPlayerRenderCall() const
{
	AdvancedRenderer::SingletonInstance()->BindFrameBuffer(m_FullScreenFBO);
	AdvancedRenderer::SingletonInstance()->ClearScreen(RGBA::BLACK);
	AdvancedRenderer::SingletonInstance()->UnbindFrameBuffer();

	RenderPlayerOne();
	RenderPlayerTwo();

	AdvancedRenderer::SingletonInstance()->CopyFrameBufferToBackBuffer(m_FullScreenFBO);

	DrawSplitScreenPartition2D();
}



void TheGame::RenderPlayerOne() const
{
	RenderPlayerWorld(m_PlayerOneCamera, m_PlayerOneFirstFBO, PLAYER_ONE);

	AdvancedRenderer::SingletonInstance()->UpdateModelMatrix(Matrix4::IdentityMatrix4());
	AdvancedRenderer::SingletonInstance()->UpdateViewMatrix(Matrix4::IdentityMatrix4());
	AdvancedRenderer::SingletonInstance()->UpdateProjectionMatrix(Matrix4::IdentityMatrix4());

	float renderWidth;
	float renderHeight;

	m_DepthOfFieldMaterial->SetColorTargetTextures(m_PlayerOneFirstFBO->GetColorTargets(), m_PlayerOneFirstFBO->GetNumberOfColorTargets());
	m_DepthOfFieldMaterial->SetDepthStencilTexture(m_PlayerOneFirstFBO->GetDepthStencil());

	renderWidth = static_cast<float>(m_PlayerOneFirstFBO->GetFrameDimensions().X);
	m_DepthOfFieldMaterial->SetFloatToShaderProgram1D("g_RenderWidth", (float*)&renderWidth);

	renderHeight = static_cast<float>(m_PlayerOneFirstFBO->GetFrameDimensions().Y);
	m_DepthOfFieldMaterial->SetFloatToShaderProgram1D("g_RenderHeight", (float*)&renderHeight);

	m_DepthOfFieldMaterial->SetFloatToShaderProgram1D("g_NearDistance", (float*)&PERSPECTIVE_NEAR_DISTANCE);
	m_DepthOfFieldMaterial->SetFloatToShaderProgram1D("g_FarDistance", (float*)&PERSPECTIVE_FAR_DISTANCE);

	size_t numberOfDepthVertices = m_PlayerOneFBODepthMesh->m_VertexBufferObject->GetElementCount();
	size_t numberOfDepthIndices = m_PlayerOneFBODepthMesh->m_IndexBufferObject->GetElementCount();
	
	AdvancedRenderer::SingletonInstance()->BindFrameBuffer(m_PlayerOneSecondFBO);
	AdvancedRenderer::SingletonInstance()->ClearScreen(RGBA::BLACK);
	AdvancedRenderer::SingletonInstance()->DrawPolygonMesh(m_PlayerOneFBODepthMesh, numberOfDepthVertices, numberOfDepthIndices, m_DepthOfFieldMaterial);
	AdvancedRenderer::SingletonInstance()->UnbindFrameBuffer();

	m_DeathEffectMaterial->SetColorTargetTextures(m_PlayerOneSecondFBO->GetColorTargets(), m_PlayerOneSecondFBO->GetNumberOfColorTargets());
	m_DeathEffectMaterial->SetDepthStencilTexture(m_PlayerOneSecondFBO->GetDepthStencil());
	
	float blendFactor = m_World->m_PlayerOne->GetHealthPercentage();
	m_DeathEffectMaterial->SetFloatToShaderProgram1D("g_ColorBlendFactor", (float*)&blendFactor);

	renderWidth = static_cast<float>(m_PlayerOneSecondFBO->GetFrameDimensions().X);
	m_DeathEffectMaterial->SetFloatToShaderProgram1D("g_RenderWidth", (float*)&renderWidth);

	renderHeight = static_cast<float>(m_PlayerOneSecondFBO->GetFrameDimensions().Y);
	m_DeathEffectMaterial->SetFloatToShaderProgram1D("g_RenderHeight", (float*)&renderHeight);

	size_t numberOfShatterVertices = m_PlayerOneFBOShatterMesh->m_VertexBufferObject->GetElementCount();
	size_t numberOfShatterIndices = m_PlayerOneFBOShatterMesh->m_IndexBufferObject->GetElementCount();
	
	IntVector2 bottomLeft = IntVector2::ZERO;
	IntVector2 viewSize = (g_TwoPlayerMode) ? IntVector2(WINDOW_PHYSICAL_WIDTH / 2, WINDOW_PHYSICAL_HEIGHT) : IntVector2(WINDOW_PHYSICAL_WIDTH, WINDOW_PHYSICAL_HEIGHT);
	
	AdvancedRenderer::SingletonInstance()->BindFrameBuffer(m_FullScreenFBO);
	AdvancedRenderer::SingletonInstance()->SetScissorViewport(bottomLeft, viewSize);
	
	AdvancedRenderer::SingletonInstance()->EnableScissorTesting(true);
	AdvancedRenderer::SingletonInstance()->DrawPolygonMesh(m_PlayerOneFBOShatterMesh, numberOfShatterVertices, numberOfShatterIndices, m_DeathEffectMaterial);
	RenderPlayerOneUI(bottomLeft, viewSize);
	AdvancedRenderer::SingletonInstance()->EnableScissorTesting(false);
	
	AdvancedRenderer::SingletonInstance()->UnbindFrameBuffer();
}



void TheGame::RenderPlayerTwo() const
{
	RenderPlayerWorld(m_PlayerTwoCamera, m_PlayerTwoFirstFBO, PLAYER_TWO);

	AdvancedRenderer::SingletonInstance()->UpdateModelMatrix(Matrix4::IdentityMatrix4());
	AdvancedRenderer::SingletonInstance()->UpdateViewMatrix(Matrix4::IdentityMatrix4());
	AdvancedRenderer::SingletonInstance()->UpdateProjectionMatrix(Matrix4::IdentityMatrix4());

	float renderWidth;
	float renderHeight;

	m_DepthOfFieldMaterial->SetColorTargetTextures(m_PlayerTwoFirstFBO->GetColorTargets(), m_PlayerTwoFirstFBO->GetNumberOfColorTargets());
	m_DepthOfFieldMaterial->SetDepthStencilTexture(m_PlayerTwoFirstFBO->GetDepthStencil());

	renderWidth = static_cast<float>(m_PlayerTwoFirstFBO->GetFrameDimensions().X);
	m_DepthOfFieldMaterial->SetFloatToShaderProgram1D("g_RenderWidth", (float*)&renderWidth);

	renderHeight = static_cast<float>(m_PlayerTwoFirstFBO->GetFrameDimensions().Y);
	m_DepthOfFieldMaterial->SetFloatToShaderProgram1D("g_RenderHeight", (float*)&renderHeight);

	m_DepthOfFieldMaterial->SetFloatToShaderProgram1D("g_NearDistance", (float*)&PERSPECTIVE_NEAR_DISTANCE);
	m_DepthOfFieldMaterial->SetFloatToShaderProgram1D("g_FarDistance", (float*)&PERSPECTIVE_FAR_DISTANCE);

	size_t numberOfDepthVertices = m_PlayerTwoFBODepthMesh->m_VertexBufferObject->GetElementCount();
	size_t numberOfDepthIndices = m_PlayerTwoFBODepthMesh->m_IndexBufferObject->GetElementCount();
	
	AdvancedRenderer::SingletonInstance()->BindFrameBuffer(m_PlayerTwoSecondFBO);
	AdvancedRenderer::SingletonInstance()->ClearScreen(RGBA::BLACK);
	AdvancedRenderer::SingletonInstance()->DrawPolygonMesh(m_PlayerTwoFBODepthMesh, numberOfDepthVertices, numberOfDepthIndices, m_DepthOfFieldMaterial);
	AdvancedRenderer::SingletonInstance()->UnbindFrameBuffer();

	m_DeathEffectMaterial->SetColorTargetTextures(m_PlayerTwoSecondFBO->GetColorTargets(), m_PlayerTwoSecondFBO->GetNumberOfColorTargets());
	m_DeathEffectMaterial->SetDepthStencilTexture(m_PlayerTwoSecondFBO->GetDepthStencil());

	float blendFactor = m_World->m_PlayerTwo->GetHealthPercentage();
	m_DeathEffectMaterial->SetFloatToShaderProgram1D("g_ColorBlendFactor", (float*)&blendFactor);

	renderWidth = static_cast<float>(m_PlayerTwoSecondFBO->GetFrameDimensions().X);
	m_DeathEffectMaterial->SetFloatToShaderProgram1D("g_RenderWidth", (float*)&renderWidth);

	renderHeight = static_cast<float>(m_PlayerTwoSecondFBO->GetFrameDimensions().Y);
	m_DeathEffectMaterial->SetFloatToShaderProgram1D("g_RenderHeight", (float*)&renderHeight);

	size_t numberOfShatterVertices = m_PlayerTwoFBOShatterMesh->m_VertexBufferObject->GetElementCount();
	size_t numberOfShatterIndices = m_PlayerTwoFBOShatterMesh->m_IndexBufferObject->GetElementCount();

	IntVector2 bottomLeft = IntVector2(WINDOW_PHYSICAL_WIDTH / 2, 0);
	IntVector2 viewSize = IntVector2(WINDOW_PHYSICAL_WIDTH / 2, WINDOW_PHYSICAL_HEIGHT);
	
	AdvancedRenderer::SingletonInstance()->BindFrameBuffer(m_FullScreenFBO);
	AdvancedRenderer::SingletonInstance()->SetScissorViewport(bottomLeft, viewSize);
	
	AdvancedRenderer::SingletonInstance()->EnableScissorTesting(true);
	AdvancedRenderer::SingletonInstance()->DrawPolygonMesh(m_PlayerTwoFBOShatterMesh, numberOfShatterVertices, numberOfShatterIndices, m_DeathEffectMaterial);
	RenderPlayerTwoUI(bottomLeft, viewSize);
	AdvancedRenderer::SingletonInstance()->EnableScissorTesting(false);
	
	AdvancedRenderer::SingletonInstance()->UnbindFrameBuffer();
}



void TheGame::RenderPlayerWorld(const Camera3D* playerCamera, FrameBuffer* currentPlayerFBO, uint8_t currentPlayerID) const
{
	SetUpDrawingIn3D(playerCamera);

	Camera3D reflectedCamera = *playerCamera;
	reflectedCamera.m_Position.Z -= (reflectedCamera.m_Position.Z - WATER_LEVEL) * 2.0f;
	reflectedCamera.m_Orientation.m_PitchAngleInDegrees *= -1.0f;
	Matrix4 reflectedViewMatrix = AdvancedRenderer::SingletonInstance()->GetViewMatrix(reflectedCamera.m_Position, reflectedCamera.m_Orientation);

	Camera3D refractedCamera = *playerCamera;
	Matrix4 refractedViewMatrix = AdvancedRenderer::SingletonInstance()->GetViewMatrix(refractedCamera.m_Position, refractedCamera.m_Orientation);

	Vector4 clipPlane;
	AdvancedRenderer::SingletonInstance()->EnableClipping(true, 0);

	AdvancedRenderer::SingletonInstance()->BindFrameBuffer(m_ReflectionFBO);
	AdvancedRenderer::SingletonInstance()->ClearScreen(RGBA::BLACK);
	AdvancedRenderer::SingletonInstance()->UpdateViewMatrix(reflectedViewMatrix);

	clipPlane = (playerCamera->m_Position.Z > WATER_LEVEL) ? Vector4(0.0f, 0.0f, 1.0f, -WATER_LEVEL) : Vector4(0.0f, 0.0f, -1.0f, WATER_LEVEL);
	m_World->RenderSkyboxFromCamera(&reflectedCamera);
	m_World->RenderWorldFromCamera(&reflectedCamera, currentPlayerID, clipPlane);
	m_World->RenderAllEntities();

	AdvancedRenderer::SingletonInstance()->BindFrameBuffer(m_RefractionFBO);
	AdvancedRenderer::SingletonInstance()->ClearScreen(RGBA::BLACK);
	AdvancedRenderer::SingletonInstance()->UpdateViewMatrix(refractedViewMatrix);
	
	clipPlane = (playerCamera->m_Position.Z > WATER_LEVEL) ? Vector4(0.0f, 0.0f, -1.0f, WATER_LEVEL) : Vector4(0.0f, 0.0f, 1.0f, -WATER_LEVEL);
	m_World->RenderSkyboxFromCamera(&refractedCamera);
	m_World->RenderWorldFromCamera(&refractedCamera, currentPlayerID, clipPlane);
	m_World->RenderAllEntities();

	AdvancedRenderer::SingletonInstance()->EnableClipping(false, 0);

	AdvancedRenderer::SingletonInstance()->BindFrameBuffer(currentPlayerFBO);
	AdvancedRenderer::SingletonInstance()->ClearScreen(RGBA::BLACK);
	m_World->RenderSkyboxFromCamera(playerCamera);
	m_World->RenderWorldFromCamera(playerCamera, currentPlayerID);

	Texture* colorTargets[2U];
	colorTargets[0] = m_ReflectionFBO->GetColorTargets()[0];
	colorTargets[1] = m_RefractionFBO->GetColorTargets()[0];

	Texture* depthStencil = m_RefractionFBO->GetDepthStencil();

	m_World->RenderWaterFromCamera(playerCamera, colorTargets, depthStencil, PERSPECTIVE_NEAR_DISTANCE, PERSPECTIVE_FAR_DISTANCE);
	m_World->RenderAllEntities();

	if (g_DebugMode)
	{
		DrawOriginLinesIn3D();
	}

	AdvancedRenderer::SingletonInstance()->UnbindFrameBuffer();
}



void TheGame::RenderPlayerOneUI(const IntVector2& bottomLeft, const IntVector2& viewSize) const
{
	if (g_ScreenshotMode)
	{
		return;
	}

	AdvancedRenderer::SingletonInstance()->SetProjectionViewport(bottomLeft, viewSize);
	
	SetUpDrawingIn2D();
	DrawBlockSelection2D(m_PlayerOneSelectedBlock);
	if (m_World->m_PlayerOne->IsAlive())
	{
		DrawCrosshairs2D();
	}

	if (g_DebugMode)
	{
		DisplayCameraStatsIn2D(*m_PlayerOneCamera, m_PlayerOneCameraMode, m_World->m_PlayerOne->m_CurrentPhysicsMode, PLAYER_ONE);
	}
}



void TheGame::RenderPlayerTwoUI(const IntVector2& bottomLeft, const IntVector2& viewSize) const
{
	if (g_ScreenshotMode)
	{
		return;
	}
	
	AdvancedRenderer::SingletonInstance()->SetProjectionViewport(bottomLeft, viewSize);
	
	SetUpDrawingIn2D();
	DrawBlockSelection2D(m_PlayerTwoSelectedBlock);
	if (m_World->m_PlayerTwo->IsAlive())
	{
		DrawCrosshairs2D();
	}

	if (g_DebugMode)
	{
		DisplayCameraStatsIn2D(*m_PlayerTwoCamera, m_PlayerTwoCameraMode, m_World->m_PlayerTwo->m_CurrentPhysicsMode, PLAYER_TWO);
	}
}



void TheGame::InitializeMusicTracks(void*)
{
	SoundID currentMusicTrack;

	currentMusicTrack = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Music/GameSoundtrack/Track 1 - Victory or Death.mp3");
	s_MusicTracks[0] = currentMusicTrack;

	currentMusicTrack = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Music/GameSoundtrack/Track 2 - For The King.mp3");
	s_MusicTracks[1] = currentMusicTrack;

	currentMusicTrack = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Music/GameSoundtrack/Track 3 - Home of Heroes.mp3");
	s_MusicTracks[2] = currentMusicTrack;

	currentMusicTrack = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Music/GameSoundtrack/Track 4 - Legend.mp3");
	s_MusicTracks[3] = currentMusicTrack;

	currentMusicTrack = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Music/GameSoundtrack/Track 5 - Queen of the Gaels.mp3");
	s_MusicTracks[4] = currentMusicTrack;

	currentMusicTrack = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Music/GameSoundtrack/Track 6 - Victorious.mp3");
	s_MusicTracks[5] = currentMusicTrack;

	currentMusicTrack = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Music/GameSoundtrack/Track 7 - Wolf Blood.mp3");
	s_MusicTracks[6] = currentMusicTrack;

	currentMusicTrack = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Music/GameSoundtrack/Track 8 - Woodland Tales.mp3");
	s_MusicTracks[7] = currentMusicTrack;

	EnterCriticalSection(&g_MusicLoadingCriticalSection);
	{
		g_MusicFinishedLoading = true;
	}
	LeaveCriticalSection(&g_MusicLoadingCriticalSection);
}



void TheGame::PlayBackgroundMusic()
{
	if (AudioSystem::SingletonInstance()->IsSoundPlaying(m_MusicChannel))
	{
		return;
	}

	int trackNumber = rand() % (NUMBER_OF_MUSIC_TRACKS / 2);
	SoundID playableTrack = s_MusicTracks[trackNumber];

	for (int trackID = trackNumber; trackID < NUMBER_OF_MUSIC_TRACKS - 1; ++trackID)
	{
		s_MusicTracks[trackID] = s_MusicTracks[trackID + 1];
	}
	s_MusicTracks[NUMBER_OF_MUSIC_TRACKS - 1] = playableTrack;

	m_MusicChannel = AudioSystem::SingletonInstance()->PlaySound(playableTrack, FORWARD_PLAYBACK_MODE, 0.25f);
}



void TheGame::InitializePlayerDefaults()
{
	m_PlayerOneCamera = new Camera3D(Vector3::ZERO, EulerAngles::ZERO);
	m_PlayerOneCameraMode = FIRST_PERSON_MODE;
	m_PlayerOneSelectedBlock = GRASS_BLOCK;

	m_PlayerTwoCamera = (g_TwoPlayerMode) ? new Camera3D(Vector3::ZERO, EulerAngles::ZERO) : nullptr;
	m_PlayerTwoCameraMode = (g_TwoPlayerMode) ? FIRST_PERSON_MODE : INVALID_CAMERA_MODE;
	m_PlayerTwoSelectedBlock = (g_TwoPlayerMode) ? GRASS_BLOCK : INVALID_BLOCK;
}



void TheGame::UninitializePlayerDefaults()
{
	delete m_PlayerOneCamera;

	delete m_PlayerTwoCamera;
}



void TheGame::LoadGame()
{
	delete m_MainMenu;
	m_MainMenu = nullptr;

	InitializePlayerDefaults();
	m_World = new World();

	HUDTileDefinition::InitializeHUDTileDefinitions(*m_HUDSpriteSheet);
	CreateCrosshairsMesh();
	CreateBlockSelectionMeshAndMaterial();
	CreateDepthOfFieldAndDeathEffectMaterials();
	m_BlockSelectionOutlineMesh = new Mesh();

	if (g_TwoPlayerMode)
	{
		CreateSplitScreenPartitionMesh();
	}

	IntVector2 playerFBODimensions = (g_TwoPlayerMode) ? IntVector2(WINDOW_PHYSICAL_WIDTH / 2, WINDOW_PHYSICAL_HEIGHT) : IntVector2(WINDOW_PHYSICAL_WIDTH, WINDOW_PHYSICAL_HEIGHT);
	m_PlayerOneFirstFBO = FrameBuffer::CreateFrameBuffer(playerFBODimensions);
	m_PlayerOneSecondFBO = FrameBuffer::CreateFrameBuffer(playerFBODimensions);
	m_PlayerTwoFirstFBO = (g_TwoPlayerMode) ? FrameBuffer::CreateFrameBuffer(playerFBODimensions) : nullptr;
	m_PlayerTwoSecondFBO = (g_TwoPlayerMode) ? FrameBuffer::CreateFrameBuffer(playerFBODimensions) : nullptr;
	m_ReflectionFBO = FrameBuffer::CreateFrameBuffer(playerFBODimensions);
	m_RefractionFBO = FrameBuffer::CreateFrameBuffer(playerFBODimensions);
	m_FullScreenFBO = FrameBuffer::CreateFrameBuffer(WINDOW_DIMENSIONS);

	CreatePlayerOneFBODepthMesh();
	CreatePlayerOneFBOShatterMesh();
	CreatePlayerTwoFBODepthMesh();
	CreatePlayerTwoFBOShatterMesh();
	CreateFullScreenFBOMesh();
}



void TheGame::UnloadGame()
{
	delete m_World;
	m_World = nullptr;
	AudioSystem::SingletonInstance()->StopSound(m_MusicChannel);

	UninitializePlayerDefaults();

	delete m_CrosshairsMesh;
	delete m_BlockSelectionMesh;
	delete m_BlockSelectionOutlineMesh;
	delete m_BlockSelectionMaterial;
	delete m_DepthOfFieldMaterial;
	delete m_DeathEffectMaterial;
	delete m_SplitScreenPartitionMesh;

	FrameBuffer::DestroyFrameBuffer(m_PlayerOneFirstFBO);
	FrameBuffer::DestroyFrameBuffer(m_PlayerOneSecondFBO);
	FrameBuffer::DestroyFrameBuffer(m_PlayerTwoFirstFBO);
	FrameBuffer::DestroyFrameBuffer(m_PlayerTwoSecondFBO);
	FrameBuffer::DestroyFrameBuffer(m_ReflectionFBO);
	FrameBuffer::DestroyFrameBuffer(m_RefractionFBO);
	FrameBuffer::DestroyFrameBuffer(m_FullScreenFBO);

	delete m_PlayerOneFBODepthMesh;
	delete m_PlayerOneFBOShatterMesh;
	delete m_PlayerTwoFBODepthMesh;
	delete m_PlayerTwoFBOShatterMesh;
	delete m_FullScreenFBOMesh;

	m_MainMenu = new MainMenu();
}



void TheGame::ExitToMainMenu(float deltaTimeInSeconds)
{
	static float exitTimer = 0.0f;
	const float TIMER_THRESHOLD = 2.5f;

	if (!g_TwoPlayerMode)
	{
		if (InputSystem::SingletonInstance()->ButtonIsHeldDown(PLAYER_ONE, BACK_BUTTON) && InputSystem::SingletonInstance()->ButtonIsHeldDown(PLAYER_ONE, START_BUTTON))
		{
			exitTimer += deltaTimeInSeconds;
		}
		else
		{
			if (exitTimer < TIMER_THRESHOLD)
			{
				exitTimer = 0.0f;
			}
		}
	}
	else
	{
		if ((InputSystem::SingletonInstance()->ButtonIsHeldDown(PLAYER_ONE, BACK_BUTTON) && InputSystem::SingletonInstance()->ButtonIsHeldDown(PLAYER_ONE, START_BUTTON)) &&
			(InputSystem::SingletonInstance()->ButtonIsHeldDown(PLAYER_TWO, BACK_BUTTON) && InputSystem::SingletonInstance()->ButtonIsHeldDown(PLAYER_TWO, START_BUTTON)))
		{
			exitTimer += deltaTimeInSeconds;
		}
		else
		{
			if (exitTimer < TIMER_THRESHOLD)
			{
				exitTimer = 0.0f;
			}
		}
	}

	if (exitTimer >= TIMER_THRESHOLD)
	{
		g_LoadGame = false;
		exitTimer = 0.0f;
	}
}



void TheGame::CreateCrosshairsMesh()
{
	const size_t NUMBER_OF_CROSSHAIRS_VERTICES = 4;
	const size_t NUMBER_OF_CROSSHAIRS_INDICES = 4;
	
	float dividedValue = (g_TwoPlayerMode) ? 4.0f : 2.0f;

	Vector3 horizontalStartingPoint = Vector3((VIEW_WIDTH / dividedValue) - 0.25f, VIEW_HEIGHT / 2.0f, 0.0f);
	Vector3 horizontalEndingPoint = Vector3((VIEW_WIDTH / dividedValue) + 0.25f, VIEW_HEIGHT / 2.0f, 0.0f);

	Vector3 verticalStartingPoint = Vector3(VIEW_WIDTH / dividedValue, (VIEW_HEIGHT / 2.0f) - 0.25f, 0.0f);
	Vector3 verticalEndingPoint = Vector3(VIEW_WIDTH / dividedValue, (VIEW_HEIGHT / 2.0f) + 0.25f, 0.0f);

	Vertex3D crosshairsVertices[NUMBER_OF_CROSSHAIRS_VERTICES];
	uint32_t crosshairsIndices[NUMBER_OF_CROSSHAIRS_INDICES] = { 0, 1, 2, 3 };

	Vertex3D crosshairsVertex;
	crosshairsVertex.m_Color = RGBA::WHITE;

	crosshairsVertex.m_Position = horizontalStartingPoint;
	crosshairsVertices[0] = crosshairsVertex;

	crosshairsVertex.m_Position = horizontalEndingPoint;
	crosshairsVertices[1] = crosshairsVertex;

	crosshairsVertex.m_Position = verticalStartingPoint;
	crosshairsVertices[2] = crosshairsVertex;

	crosshairsVertex.m_Position = verticalEndingPoint;
	crosshairsVertices[3] = crosshairsVertex;

	crosshairsIndices;

	m_CrosshairsMesh = new Mesh(&crosshairsVertices[0], &crosshairsIndices[0], NUMBER_OF_CROSSHAIRS_VERTICES, NUMBER_OF_CROSSHAIRS_INDICES);
}



void TheGame::CreateBlockSelectionMeshAndMaterial()
{
	const size_t NUMBER_OF_BLOCK_SELECTION_VERTICES = 36U;
	const size_t NUMBER_OF_BLOCK_SELECTION_INDICES = 54U;
	
	Vertex3D blockSelectionVertices[NUMBER_OF_BLOCK_SELECTION_VERTICES];
	uint32_t blockSelectionIndices[NUMBER_OF_BLOCK_SELECTION_INDICES];

	Vertex3D blockSelectionVertex;
	blockSelectionVertex.m_Color = RGBA::WHITE;

	size_t currentNumberOfVertices = 0U;
	size_t currentNumberOfIndices = 0U;
	for (int HUDTileIndex = 0; HUDTileIndex < NUMBER_OF_HUD_TILES; ++HUDTileIndex)
	{
		blockSelectionIndices[currentNumberOfIndices++] = currentNumberOfVertices + 0U;
		blockSelectionIndices[currentNumberOfIndices++] = currentNumberOfVertices + 1U;
		blockSelectionIndices[currentNumberOfIndices++] = currentNumberOfVertices + 2U;
		blockSelectionIndices[currentNumberOfIndices++] = currentNumberOfVertices + 2U;
		blockSelectionIndices[currentNumberOfIndices++] = currentNumberOfVertices + 3U;
		blockSelectionIndices[currentNumberOfIndices++] = currentNumberOfVertices + 0U;

		AABB2 tileBounds = HUDTileDefinition::s_HUDTileDefinitions[HUDTileIndex].m_tileBounds;
		AABB2 tileTextureBounds = HUDTileDefinition::s_HUDTileDefinitions[HUDTileIndex].m_tileTextureBounds;

		blockSelectionVertex.m_Position = Vector3(tileBounds.minimums.X, tileBounds.minimums.Y, 0.0f);
		blockSelectionVertex.m_TextureCoordinates = Vector2(tileTextureBounds.minimums.X, tileTextureBounds.maximums.Y);
		blockSelectionVertices[currentNumberOfVertices++] = blockSelectionVertex;

		blockSelectionVertex.m_Position = Vector3(tileBounds.maximums.X, tileBounds.minimums.Y, 0.0f);
		blockSelectionVertex.m_TextureCoordinates = Vector2(tileTextureBounds.maximums.X, tileTextureBounds.maximums.Y);
		blockSelectionVertices[currentNumberOfVertices++] = blockSelectionVertex;

		blockSelectionVertex.m_Position = Vector3(tileBounds.maximums.X, tileBounds.maximums.Y, 0.0f);
		blockSelectionVertex.m_TextureCoordinates = Vector2(tileTextureBounds.maximums.X, tileTextureBounds.minimums.Y);
		blockSelectionVertices[currentNumberOfVertices++] = blockSelectionVertex;

		blockSelectionVertex.m_Position = Vector3(tileBounds.minimums.X, tileBounds.maximums.Y, 0.0f);
		blockSelectionVertex.m_TextureCoordinates = Vector2(tileTextureBounds.minimums.X, tileTextureBounds.minimums.Y);
		blockSelectionVertices[currentNumberOfVertices++] = blockSelectionVertex;
	}

	m_BlockSelectionMesh = new Mesh(blockSelectionVertices, blockSelectionIndices, NUMBER_OF_BLOCK_SELECTION_VERTICES, NUMBER_OF_BLOCK_SELECTION_INDICES);
	m_BlockSelectionMaterial = new Material("Data/Shaders/UIShader.vert", "Data/Shaders/UIShader.frag");
	m_BlockSelectionMaterial->SetDiffuseTexture(m_HUDSpriteSheet->GetSpriteSheet());
}



void TheGame::CreateDepthOfFieldAndDeathEffectMaterials()
{
	m_DepthOfFieldMaterial = new Material("Data/Shaders/DepthOfFieldShader.vert", "Data/Shaders/DepthOfFieldShader.frag");
	m_DeathEffectMaterial = new Material("Data/Shaders/DeathEffectShader.vert", "Data/Shaders/DeathEffectShader.frag");
}



void TheGame::CreateSplitScreenPartitionMesh()
{
	const size_t NUMBER_OF_PARTITION_VERTICES = 2;
	const size_t NUMBER_OF_PARTITION_INDICES = 2;

	Vertex3D lineVertices[NUMBER_OF_PARTITION_VERTICES];
	uint32_t lineIndices[NUMBER_OF_PARTITION_INDICES] = { 0, 1 };

	Vertex3D lineVertex;
	lineVertex.m_Color = RGBA::WHITE;

	lineVertex.m_Position = Vector3(VIEW_WIDTH / 4.0f, 0.0f, 0.0f);
	lineVertices[0] = lineVertex;

	lineVertex.m_Position = Vector3(VIEW_WIDTH / 4.0f, VIEW_HEIGHT, 0.0f);
	lineVertices[1] = lineVertex;

	m_SplitScreenPartitionMesh = new Mesh(&lineVertices[0], &lineIndices[0], NUMBER_OF_PARTITION_VERTICES, NUMBER_OF_PARTITION_INDICES);
}



void TheGame::CreatePlayerOneFBODepthMesh()
{
	m_PlayerOneFBODepthMesh = new Mesh();

	const size_t NUMBER_OF_MESH_VERTICES = 4;
	const size_t NUMBER_OF_MESH_INDICES = 6;

	Vertex3D meshVertices[NUMBER_OF_MESH_VERTICES];
	uint32_t meshIndices[NUMBER_OF_MESH_INDICES] = { 0, 1, 2, 2, 3, 0 };

	Vertex3D meshVertex;
	meshVertex.m_Color = RGBA::WHITE;

	meshVertex.m_Position = Vector3(-1.0f, -1.0f, 0.0f);
	meshVertex.m_TextureCoordinates = Vector2(0.0f, 1.0f);
	meshVertices[0] = meshVertex;

	meshVertex.m_Position = Vector3(1.0f, -1.0f, 0.0f);
	meshVertex.m_TextureCoordinates = Vector2(1.0f, 1.0f);
	meshVertices[1] = meshVertex;

	meshVertex.m_Position = Vector3(1.0f, 1.0f, 0.0f);
	meshVertex.m_TextureCoordinates = Vector2(1.0f, 0.0f);
	meshVertices[2] = meshVertex;

	meshVertex.m_Position = Vector3(-1.0f, 1.0f, 0.0f);
	meshVertex.m_TextureCoordinates = Vector2(0.0f, 0.0f);
	meshVertices[3] = meshVertex;

	m_PlayerOneFBODepthMesh->WriteToMesh(&meshVertices[0], &meshIndices[0], NUMBER_OF_MESH_VERTICES, NUMBER_OF_MESH_INDICES);
}



void TheGame::CreatePlayerOneFBOShatterMesh()
{
	m_PlayerOneFBOShatterMesh = new Mesh();
	GenerateNewPlayerOneFBOShatterMesh();
}



void TheGame::CreatePlayerTwoFBODepthMesh()
{
	if (g_TwoPlayerMode)
	{
		m_PlayerTwoFBODepthMesh = new Mesh();

		const size_t NUMBER_OF_MESH_VERTICES = 4;
		const size_t NUMBER_OF_MESH_INDICES = 6;

		Vertex3D meshVertices[NUMBER_OF_MESH_VERTICES];
		uint32_t meshIndices[NUMBER_OF_MESH_INDICES] = { 0, 1, 2, 2, 3, 0 };

		Vertex3D meshVertex;
		meshVertex.m_Color = RGBA::WHITE;

		meshVertex.m_Position = Vector3(-1.0f, -1.0f, 0.0f);
		meshVertex.m_TextureCoordinates = Vector2(0.0f, 1.0f);
		meshVertices[0] = meshVertex;

		meshVertex.m_Position = Vector3(1.0f, -1.0f, 0.0f);
		meshVertex.m_TextureCoordinates = Vector2(1.0f, 1.0f);
		meshVertices[1] = meshVertex;

		meshVertex.m_Position = Vector3(1.0f, 1.0f, 0.0f);
		meshVertex.m_TextureCoordinates = Vector2(1.0f, 0.0f);
		meshVertices[2] = meshVertex;

		meshVertex.m_Position = Vector3(-1.0f, 1.0f, 0.0f);
		meshVertex.m_TextureCoordinates = Vector2(0.0f, 0.0f);
		meshVertices[3] = meshVertex;

		m_PlayerTwoFBODepthMesh->WriteToMesh(&meshVertices[0], &meshIndices[0], NUMBER_OF_MESH_VERTICES, NUMBER_OF_MESH_INDICES);
	}
}



void TheGame::CreatePlayerTwoFBOShatterMesh()
{
	if (g_TwoPlayerMode)
	{
		m_PlayerTwoFBOShatterMesh = new Mesh();
		GenerateNewPlayerTwoFBOShatterMesh();
	}
}



void TheGame::CreateFullScreenFBOMesh()
{
	const size_t NUMBER_OF_FRAME_VERTICES = 4;
	const size_t NUMBER_OF_FRAME_INDICES = 6;
	
	Vertex3D frameVertices[NUMBER_OF_FRAME_VERTICES];
	uint32_t frameIndices[NUMBER_OF_FRAME_INDICES] = { 0, 1, 2, 2, 3, 0 };

	Vertex3D frameVertex;
	frameVertex.m_Color = RGBA::WHITE;

	frameVertex.m_Position = Vector3(-1.0f, -1.0f, 0.0f);
	frameVertex.m_TextureCoordinates = Vector2(0.0f, 1.0f);
	frameVertices[0] = frameVertex;

	frameVertex.m_Position = Vector3(1.0f, -1.0f, 0.0f);
	frameVertex.m_TextureCoordinates = Vector2(1.0f, 1.0f);
	frameVertices[1] = frameVertex;

	frameVertex.m_Position = Vector3(1.0f, 1.0f, 0.0f);
	frameVertex.m_TextureCoordinates = Vector2(1.0f, 0.0f);
	frameVertices[2] = frameVertex;

	frameVertex.m_Position = Vector3(-1.0f, 1.0f, 0.0f);
	frameVertex.m_TextureCoordinates = Vector2(0.0f, 0.0f);
	frameVertices[3] = frameVertex;

	m_FullScreenFBOMesh = new Mesh(&frameVertices[0], &frameIndices[0], NUMBER_OF_FRAME_VERTICES, NUMBER_OF_FRAME_INDICES);
}



void TheGame::ToggleDebugMode()
{
	if (InputSystem::SingletonInstance()->KeyWasJustPressed('I'))
	{
		g_DebugMode = !g_DebugMode;
	}
}



void TheGame::ToggleScreenshotMode()
{
	if (InputSystem::SingletonInstance()->KeyWasJustPressed('P'))
	{
		g_ScreenshotMode = !g_ScreenshotMode;
	}
}



void TheGame::SetUpDrawingIn3D(const Camera3D* playerCamera) const
{
	AdvancedRenderer::SingletonInstance()->EnableDepthTesting(true);

	float aspectRatio = (g_TwoPlayerMode) ? (ASPECT_RATIO / 2.0f) : ASPECT_RATIO;
	
	Matrix4 modelMatrix = AdvancedRenderer::SingletonInstance()->GetModelMatrix(Vector3::ONE, EulerAngles::ZERO, Vector3::ZERO);
	Matrix4 viewMatrix = AdvancedRenderer::SingletonInstance()->GetViewMatrix(playerCamera->m_Position, playerCamera->m_Orientation);
	Matrix4 projectionMatrix = AdvancedRenderer::SingletonInstance()->GetPerspectiveProjectionMatrix(FOV_ANGLE_IN_DEGREES, aspectRatio, PERSPECTIVE_NEAR_DISTANCE, PERSPECTIVE_FAR_DISTANCE);

	AdvancedRenderer::SingletonInstance()->UpdateModelMatrix(modelMatrix);
	AdvancedRenderer::SingletonInstance()->UpdateViewMatrix(viewMatrix);
	AdvancedRenderer::SingletonInstance()->UpdateProjectionMatrix(projectionMatrix);
}



void TheGame::SetUpDrawingIn2D() const
{
	AdvancedRenderer::SingletonInstance()->EnableDepthTesting(false);

	Vector2 topRight = (g_TwoPlayerMode) ? Vector2(VIEW_WIDTH / 2.0f, VIEW_HEIGHT) : Vector2(VIEW_WIDTH, VIEW_HEIGHT);

	Matrix4 modelMatrix = AdvancedRenderer::SingletonInstance()->GetModelMatrix(Vector3::ONE, EulerAngles::ZERO, Vector3::ZERO);
	Matrix4 viewMatrix = AdvancedRenderer::SingletonInstance()->GetViewMatrix(Vector3::ZERO, EulerAngles::ZERO);
	Matrix4 projectionMatrix = AdvancedRenderer::SingletonInstance()->GetOrthographicProjectionMatrix(Vector2::ZERO, topRight, ORTHOGRAPHIC_NEAR_DISTANCE, ORTHOGRAPHIC_FAR_DISTANCE);

	AdvancedRenderer::SingletonInstance()->UpdateModelMatrix(modelMatrix);
	AdvancedRenderer::SingletonInstance()->UpdateViewMatrix(viewMatrix);
	AdvancedRenderer::SingletonInstance()->UpdateProjectionMatrix(projectionMatrix);
}



void TheGame::CameraLookWithMouseMovement()
{
	IntVector2 deltaMousePosition;
	IntVector2 currentMousePosition = InputSystem::SingletonInstance()->GetMouseCursorScreenPosition();
	deltaMousePosition = currentMousePosition - DEFAULT_MOUSE_POSITION;
	InputSystem::SingletonInstance()->SetMouseCursorScreenPosition(DEFAULT_MOUSE_POSITION);

	m_PlayerOneCamera->m_Orientation.m_YawAngleInDegrees -= MOUSE_SENSITIVITY * deltaMousePosition.X;
	m_PlayerOneCamera->m_Orientation.m_PitchAngleInDegrees += MOUSE_SENSITIVITY * deltaMousePosition.Y;
	m_PlayerOneCamera->FixAndClampAngles();

	m_PlayerTwoCamera->m_Orientation.m_YawAngleInDegrees -= MOUSE_SENSITIVITY * deltaMousePosition.X;
	m_PlayerTwoCamera->m_Orientation.m_PitchAngleInDegrees += MOUSE_SENSITIVITY * deltaMousePosition.Y;
	m_PlayerTwoCamera->FixAndClampAngles();
}



void TheGame::CameraLook(Camera3D* playerCamera, uint8_t currentPlayerID, float deltaTimeInSeconds)
{
	Vector2 rightStickPosition = InputSystem::SingletonInstance()->AnalogStickCartesianPosition(currentPlayerID, RIGHT_STICK);

	playerCamera->m_Orientation.m_YawAngleInDegrees -= rightStickPosition.X * LOOK_SENSITIVITY * deltaTimeInSeconds;
	playerCamera->m_Orientation.m_PitchAngleInDegrees += rightStickPosition.Y * LOOK_SENSITIVITY * deltaTimeInSeconds;
	playerCamera->FixAndClampAngles();
}



void TheGame::UpdatePlayerDirections(Player* currentPlayer, const Camera3D* playerCamera)
{
	currentPlayer->m_ForwardXY = playerCamera->GetForwardXY();
	currentPlayer->m_LeftXY = playerCamera->GetLeftXY();
	currentPlayer->m_ForwardXYZ = playerCamera->GetForwardXYZ();
}



void TheGame::KillPlayerAccordingly(Player* currentPlayer)
{
	if (currentPlayer->GetRemainingHealth() <= 0)
	{
		currentPlayer->SetAlive(false);
	}
}



void TheGame::ResurrectPlayerAccordingly(Player* currentPlayer, uint8_t currentPlayerID)
{
	if (!currentPlayer->IsAlive())
	{
		if (InputSystem::SingletonInstance()->ButtonWasJustPressed(currentPlayerID, X_BUTTON))
		{
			currentPlayer->SetAlive(true);
			currentPlayer->ResetHealth();

			switch (currentPlayerID)
			{
			case PLAYER_ONE:
				GenerateNewPlayerOneFBOShatterMesh();
				break;

			case PLAYER_TWO:
				GenerateNewPlayerTwoFBOShatterMesh();
				break;

			default:
				break;
			}
		}
	}
}



void TheGame::ToggleCameraMode(uint8_t& playerCameraMode, uint8_t currentPlayerID)
{
	if (InputSystem::SingletonInstance()->ButtonWasJustPressed(currentPlayerID, Y_BUTTON))
	{
		if (playerCameraMode == FIRST_PERSON_MODE)
		{
			playerCameraMode = THIRD_PERSON_MODE;
		}
		else if (playerCameraMode == THIRD_PERSON_MODE)
		{
			playerCameraMode = FIRST_PERSON_MODE;
		}
	}
}



void TheGame::UpdateCameraPosition(const Player* currentPlayer, Camera3D* playerCamera, uint8_t playerCameraMode)
{
	Vector3 playerEye = currentPlayer->m_Position + EYE_LEVEL_VIEW;
	Vector3 positionOfThirdPerson = playerEye - (playerCamera->GetForwardXYZ() * MAXIMUM_CAMERA_DISTANCE);
	Vector3 snappedPositionCorrection = playerCamera->GetForwardXYZ() * 0.5f;

	if (playerCameraMode == FIRST_PERSON_MODE)
	{
		playerCamera->m_Position = playerEye;
	}
	else if(playerCameraMode == THIRD_PERSON_MODE)
	{
		RaycastResult3D cameraRaycast = m_World->GetAWRaycast(playerEye, positionOfThirdPerson);

		Vector3 idealCameraPosition = (cameraRaycast.m_impactedSolidBlock) ? (cameraRaycast.m_impactedPosition + snappedPositionCorrection) : positionOfThirdPerson;
		playerCamera->m_Position = idealCameraPosition;
	}
}



void TheGame::SelectBlocksWithMouseWheel(uint8_t& selectedBlock)
{
	int8_t mouseScroll = (int8_t)selectedBlock;

	if (InputSystem::SingletonInstance()->GetMouseWheelDirection() > 0)
	{
		SoundID mouseScrollSound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/HUDSelection.ogg");
		AudioSystem::SingletonInstance()->PlaySound(mouseScrollSound, FORWARD_PLAYBACK_MODE, 0.5f);

		mouseScroll--;
		if (mouseScroll < 1)
		{
			mouseScroll = NUMBER_OF_HUD_TILES;
		}

		selectedBlock = (uint8_t)mouseScroll;
	}
	else if (InputSystem::SingletonInstance()->GetMouseWheelDirection() < 0)
	{
		SoundID mouseScrollSound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/HUDSelection.ogg");
		AudioSystem::SingletonInstance()->PlaySound(mouseScrollSound, FORWARD_PLAYBACK_MODE, 0.5f);

		mouseScroll++;
		if (mouseScroll > NUMBER_OF_HUD_TILES)
		{
			mouseScroll = 1;
		}

		selectedBlock = (uint8_t)mouseScroll;
	}
}



void TheGame::SelectBlocksWithKeyboardInput(uint8_t& selectedBlock)
{
	if (InputSystem::SingletonInstance()->KeyWasJustPressed('1') && selectedBlock != GRASS_BLOCK)
	{
		SoundID mouseScrollSound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/HUDSelection.ogg");
		AudioSystem::SingletonInstance()->PlaySound(mouseScrollSound, FORWARD_PLAYBACK_MODE, 0.5f);

		selectedBlock = GRASS_BLOCK;
	}
	else if (InputSystem::SingletonInstance()->KeyWasJustPressed('2') && selectedBlock != DIRT_BLOCK)
	{
		SoundID mouseScrollSound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/HUDSelection.ogg");
		AudioSystem::SingletonInstance()->PlaySound(mouseScrollSound, FORWARD_PLAYBACK_MODE, 0.5f);

		selectedBlock = DIRT_BLOCK;
	}
	else if (InputSystem::SingletonInstance()->KeyWasJustPressed('3') && selectedBlock != STONE_BLOCK)
	{
		SoundID mouseScrollSound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/HUDSelection.ogg");
		AudioSystem::SingletonInstance()->PlaySound(mouseScrollSound, FORWARD_PLAYBACK_MODE, 0.5f);

		selectedBlock = STONE_BLOCK;
	}
	else if (InputSystem::SingletonInstance()->KeyWasJustPressed('4') && selectedBlock != SAND_BLOCK)
	{
		SoundID mouseScrollSound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/HUDSelection.ogg");
		AudioSystem::SingletonInstance()->PlaySound(mouseScrollSound, FORWARD_PLAYBACK_MODE, 0.5f);

		selectedBlock = SAND_BLOCK;
	}
	else if (InputSystem::SingletonInstance()->KeyWasJustPressed('5') && selectedBlock != CLAY_BLOCK)
	{
		SoundID mouseScrollSound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/HUDSelection.ogg");
		AudioSystem::SingletonInstance()->PlaySound(mouseScrollSound, FORWARD_PLAYBACK_MODE, 0.5f);

		selectedBlock = CLAY_BLOCK;
	}
	else if (InputSystem::SingletonInstance()->KeyWasJustPressed('6') && selectedBlock != GLOWSTONE_BLOCK)
	{
		SoundID mouseScrollSound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/HUDSelection.ogg");
		AudioSystem::SingletonInstance()->PlaySound(mouseScrollSound, FORWARD_PLAYBACK_MODE, 0.5f);

		selectedBlock = GLOWSTONE_BLOCK;
	}
	else if (InputSystem::SingletonInstance()->KeyWasJustPressed('7') && selectedBlock != COBBLESTONE_BLOCK)
	{
		SoundID mouseScrollSound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/HUDSelection.ogg");
		AudioSystem::SingletonInstance()->PlaySound(mouseScrollSound, FORWARD_PLAYBACK_MODE, 0.5f);

		selectedBlock = COBBLESTONE_BLOCK;
	}
	else if (InputSystem::SingletonInstance()->KeyWasJustPressed('8') && selectedBlock != CLAY_BRICK_BLOCK)
	{
		SoundID mouseScrollSound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/HUDSelection.ogg");
		AudioSystem::SingletonInstance()->PlaySound(mouseScrollSound, FORWARD_PLAYBACK_MODE, 0.5f);

		selectedBlock = CLAY_BRICK_BLOCK;
	}
	else if (InputSystem::SingletonInstance()->KeyWasJustPressed('9') && selectedBlock != STONE_BRICK_BLOCK)
	{
		SoundID mouseScrollSound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/HUDSelection.ogg");
		AudioSystem::SingletonInstance()->PlaySound(mouseScrollSound, FORWARD_PLAYBACK_MODE, 0.5f);

		selectedBlock = STONE_BRICK_BLOCK;
	}
}



void TheGame::SelectBlocks(uint8_t& selectedBlock, uint8_t currentPlayerID)
{
	int8_t blockNumber = (int8_t)selectedBlock;

	if (InputSystem::SingletonInstance()->ButtonWasJustPressed(currentPlayerID, D_PAD_LEFT))
	{
		SoundID mouseScrollSound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/HUDSelection.ogg");
		AudioSystem::SingletonInstance()->PlaySound(mouseScrollSound, FORWARD_PLAYBACK_MODE, 0.5f);

		blockNumber--;
		if (blockNumber < 1)
		{
			blockNumber = NUMBER_OF_HUD_TILES;
		}

		selectedBlock = (uint8_t)blockNumber;
	}
	else if (InputSystem::SingletonInstance()->ButtonWasJustPressed(currentPlayerID, D_PAD_RIGHT))
	{
		SoundID mouseScrollSound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/HUDSelection.ogg");
		AudioSystem::SingletonInstance()->PlaySound(mouseScrollSound, FORWARD_PLAYBACK_MODE, 0.5f);

		blockNumber++;
		if (blockNumber > NUMBER_OF_HUD_TILES)
		{
			blockNumber = 1;
		}

		selectedBlock = (uint8_t)blockNumber;
	}
}



void TheGame::DrawOriginLinesIn3D() const
{
	AdvancedRenderer::SingletonInstance()->EnableDepthTesting(true);
	//g_AdvancedRenderer->Draw3DOriginLines(Vector3::ZERO, 2.0f, 255, 3.0f); TODO
	AdvancedRenderer::SingletonInstance()->EnableDepthTesting(false);
	//g_AdvancedRenderer->Draw3DOriginLines(Vector3::ZERO, 2.0f, 153, 1.0f); TODO
}



void TheGame::DrawCrosshairs2D() const
{
	size_t numberOfVertices = m_CrosshairsMesh->m_VertexBufferObject->GetElementCount();
	size_t numberOfIndices = m_CrosshairsMesh->m_IndexBufferObject->GetElementCount();
	
	AdvancedRenderer::SingletonInstance()->BlendPixels(DESTINATION_COLOR_INVERSE, ZERO_BLEND);
	AdvancedRenderer::SingletonInstance()->DrawLinesMesh(m_CrosshairsMesh, numberOfVertices, numberOfIndices);
	AdvancedRenderer::SingletonInstance()->BlendPixels(SOURCE_ALPHA, SOURCE_ALPHA_INVERSE);
}



// void TheGame::DrawDeathScreen2D(const Vector2& bottomLeft, const Vector2& topRight) const
// {
// 	AABB2 fadeQuadBoundingPoints = AABB2(bottomLeft, topRight);
// 	g_BasicRenderer->DrawSingle2DQuadVA(fadeQuadBoundingPoints, false, RGBA(0, 0, 0, 128));
// 
// 	Vector2 offset = (g_TwoPlayerMode) ? Vector2(2.0f, 2.5f) : Vector2(6.0f, 2.5f);
// 
// 	AABB2 texturedQuadBoundingPoints = AABB2(bottomLeft + offset, topRight - offset);
// 	g_BasicRenderer->DrawSingle2DTexturedQuadVA(texturedQuadBoundingPoints, AABB2::UNIT_AABB2, m_DeathMessageTexture);
// }



void TheGame::DisplayCameraStatsIn2D(const Camera3D& playerCamera, uint8_t playerCameraMode, uint8_t playerPhysicsMode, uint8_t currentPlayerID) const
{
	Vector3 cameraPosition = playerCamera.m_Position;
	
	float cameraRoll = playerCamera.m_Orientation.m_RollAngleInDegrees;
	float cameraPitch = playerCamera.m_Orientation.m_PitchAngleInDegrees;
	float cameraYaw = playerCamera.m_Orientation.m_YawAngleInDegrees;

	IntVector2 currentPlayerChunkCoordinates = m_World->GetChunkCoordinatesForPlayerPosition(currentPlayerID);

	const char* cameraMode = "";
	switch (playerCameraMode)
	{
	case FIRST_PERSON_MODE:
		cameraMode = "FIRST PERSON MODE";
		break;

	case THIRD_PERSON_MODE:
		cameraMode = "THIRD PERSON MODE";
		break;

	default:
		break;
	}

	const char* physicsMode = "";
	switch (playerPhysicsMode)
	{
	case WALKING_MODE:
		physicsMode = "WALKING MODE";
		break;

	case FLYING_MODE:
		physicsMode = "FLYING MODE";
		break;

	case NO_CLIP_MODE:
		physicsMode = "NO-CLIP MODE";
		break;

	default:
		break;
	}

	char stringBuffer[128];
	Vector2 statMinimums = Vector2(0.1f, VIEW_HEIGHT);

	statMinimums.Y -= 0.3f;
	sprintf_s(stringBuffer, "Camera Position = (%0.2f, %0.2f, %0.2f)", cameraPosition.X, cameraPosition.Y, cameraPosition.Z);
	AdvancedRenderer::SingletonInstance()->Draw2DMonospacedText(statMinimums, stringBuffer, 0.2f, 1.0f, m_HUDFont);

	statMinimums.Y -= 0.3f;
	sprintf_s(stringBuffer, "Chunk Coordinates = (%i, %i)", currentPlayerChunkCoordinates.X, currentPlayerChunkCoordinates.Y);
	AdvancedRenderer::SingletonInstance()->Draw2DMonospacedText(statMinimums, stringBuffer, 0.2f, 1.0f, m_HUDFont);
	
	statMinimums.Y -= 0.3f;
	sprintf_s(stringBuffer, "Camera Roll = %0.2f, Camera Pitch = %0.2f, Camera Yaw = %0.2f", cameraRoll, cameraPitch, cameraYaw);
	AdvancedRenderer::SingletonInstance()->Draw2DMonospacedText(statMinimums, stringBuffer, 0.2f, 1.0f, m_HUDFont);
	
	statMinimums.Y -= 0.3f;
	sprintf_s(stringBuffer, "Camera Mode: %s", cameraMode);
	AdvancedRenderer::SingletonInstance()->Draw2DMonospacedText(statMinimums, stringBuffer, 0.2f, 1.0f, m_HUDFont);
	
	statMinimums.Y -= 0.3f;
	sprintf_s(stringBuffer, "Physics Mode: %s", physicsMode);
	AdvancedRenderer::SingletonInstance()->Draw2DMonospacedText(statMinimums, stringBuffer, 0.2f, 1.0f, m_HUDFont);
}



void TheGame::DrawBlockSelection2D(const uint8_t& selectedBlock) const
{
	size_t numberOfBlockSelectionVertices = m_BlockSelectionMesh->m_VertexBufferObject->GetElementCount();
	size_t numberOfBlockSelectionIndices = m_BlockSelectionMesh->m_IndexBufferObject->GetElementCount();
	
	AdvancedRenderer::SingletonInstance()->DrawPolygonMesh(m_BlockSelectionMesh, numberOfBlockSelectionVertices, numberOfBlockSelectionIndices, m_BlockSelectionMaterial);
	
	const size_t NUMBER_OF_OUTLINE_VERTICES = 36U;
	const size_t NUMBER_OF_OUTLINE_INDICES = 72U;
	
	Vertex3D outlineVertices[NUMBER_OF_OUTLINE_VERTICES];
	uint32_t outlineIndices[NUMBER_OF_OUTLINE_INDICES];

	size_t currentNumberOfOutlineVertices = 0U;
	size_t currentNumberOfOutlineIndices = 0U;
	for (int HUDTileIndex = 0; HUDTileIndex < NUMBER_OF_HUD_TILES; ++HUDTileIndex)
	{
		outlineIndices[currentNumberOfOutlineIndices++] = currentNumberOfOutlineVertices + 0U;
		outlineIndices[currentNumberOfOutlineIndices++] = currentNumberOfOutlineVertices + 1U;
		outlineIndices[currentNumberOfOutlineIndices++] = currentNumberOfOutlineVertices + 1U;
		outlineIndices[currentNumberOfOutlineIndices++] = currentNumberOfOutlineVertices + 2U;
		outlineIndices[currentNumberOfOutlineIndices++] = currentNumberOfOutlineVertices + 2U;
		outlineIndices[currentNumberOfOutlineIndices++] = currentNumberOfOutlineVertices + 3U;
		outlineIndices[currentNumberOfOutlineIndices++] = currentNumberOfOutlineVertices + 3U;
		outlineIndices[currentNumberOfOutlineIndices++] = currentNumberOfOutlineVertices + 0U;
		
		Vertex3D outlineVertex;
		outlineVertex.m_Color = (HUDTileIndex == selectedBlock - 1) ? RGBA::WHITE : RGBA::BLACK;

		AABB2 tileBounds = HUDTileDefinition::s_HUDTileDefinitions[HUDTileIndex].m_tileBounds;

		outlineVertex.m_Position = Vector3(tileBounds.minimums.X, tileBounds.minimums.Y, 0.0f);
		outlineVertices[currentNumberOfOutlineVertices++] = outlineVertex;

		outlineVertex.m_Position = Vector3(tileBounds.maximums.X, tileBounds.minimums.Y, 0.0f);
		outlineVertices[currentNumberOfOutlineVertices++] = outlineVertex;

		outlineVertex.m_Position = Vector3(tileBounds.maximums.X, tileBounds.maximums.Y, 0.0f);
		outlineVertices[currentNumberOfOutlineVertices++] = outlineVertex;

		outlineVertex.m_Position = Vector3(tileBounds.minimums.X, tileBounds.maximums.Y, 0.0f);
		outlineVertices[currentNumberOfOutlineVertices++] = outlineVertex;
	}

	m_BlockSelectionOutlineMesh->WriteToMesh(outlineVertices, outlineIndices, NUMBER_OF_OUTLINE_VERTICES, NUMBER_OF_OUTLINE_INDICES);
	AdvancedRenderer::SingletonInstance()->DrawLinesMesh(m_BlockSelectionOutlineMesh, NUMBER_OF_OUTLINE_VERTICES, NUMBER_OF_OUTLINE_INDICES, 2.0f);
}



void TheGame::DrawSplitScreenPartition2D() const
{
	size_t numberOfVertices = m_SplitScreenPartitionMesh->m_VertexBufferObject->GetElementCount();
	size_t numberOfIndices = m_SplitScreenPartitionMesh->m_IndexBufferObject->GetElementCount();
	
	AdvancedRenderer::SingletonInstance()->SetProjectionViewport(IntVector2::ZERO, IntVector2(WINDOW_PHYSICAL_WIDTH, WINDOW_PHYSICAL_HEIGHT));
	AdvancedRenderer::SingletonInstance()->DrawLinesMesh(m_SplitScreenPartitionMesh, numberOfVertices, numberOfIndices, 2.0f);
}



// void TheGame::DrawCompass2D(const Camera3D* playerCamera) const
// {
// 	const float compassMidpoint = 152.0f;
// 	const float compassQuadSize = 256.0f;
// 	const float compassOffsetFromLeftEdge = 0.2f;
// 
// 	float compassPositionOffset = (compassMidpoint * VIEW_HEIGHT) / static_cast<float>(WINDOW_PHYSICAL_HEIGHT);
// 	Vector2 compassPosition = Vector2(compassPositionOffset, VIEW_HEIGHT - compassPositionOffset);
// 
// 	float compassQuadOffset = (compassQuadSize * VIEW_HEIGHT) / static_cast<float>(WINDOW_PHYSICAL_HEIGHT);
// 	Vector2 compassMinimums = Vector2(compassOffsetFromLeftEdge, VIEW_HEIGHT - compassQuadOffset);
// 	Vector2 compassMaximums = Vector2(compassOffsetFromLeftEdge + compassQuadOffset, VIEW_HEIGHT);
// 	AABB2 compassBounds = AABB2(compassMinimums, compassMaximums);
// 
// 	Vector2 playerHorizontalForwardXY = Vector2(playerCamera->GetForwardXY().X, playerCamera->GetForwardXY().Y);
// 	float deltaAngle = ArcTangentInDegrees(playerHorizontalForwardXY);
// 	float compassTargetOrientation = 90.0f - deltaAngle;
// 
// 	Texture* compassTexture = m_CompassSpriteSheet->GetSpriteSheet();
// 
// 	g_BasicRenderer->PushViewMatrix();
// 	g_BasicRenderer->RotateAroundOwnAxis(compassPosition, compassTargetOrientation);
// 	g_BasicRenderer->DrawSingle2DTexturedQuadVA(compassBounds, m_CompassSpriteSheet->GetTextureCoordsForSpriteIndex(0), compassTexture);
// 	g_BasicRenderer->PopViewMatrix();
// 
// 	g_BasicRenderer->DrawSingle2DTexturedQuadVA(compassBounds, m_CompassSpriteSheet->GetTextureCoordsForSpriteIndex(1), compassTexture);
// }



void TheGame::GenerateNewPlayerOneFBOShatterMesh()
{
	m_PlayerOneFragments.clear();
	ScreenFragment initialFragment;

	Vertex3D fragmentVertex;
	fragmentVertex.m_Color = RGBA::WHITE;

	fragmentVertex.m_Position = Vector3(-1.0f, -1.0f, 0.0f);
	fragmentVertex.m_TextureCoordinates = Vector2(0.0f, 1.0f);
	initialFragment.m_Vertices.push_back(fragmentVertex);

	fragmentVertex.m_Position = (g_TwoPlayerMode) ? Vector3(0.0f, -1.0f, 0.0f) : Vector3(1.0f, -1.0f, 0.0f);
	fragmentVertex.m_TextureCoordinates = Vector2(1.0f, 1.0f);
	initialFragment.m_Vertices.push_back(fragmentVertex);

	fragmentVertex.m_Position = (g_TwoPlayerMode) ? Vector3(0.0f, 1.0f, 0.0f) : Vector3(1.0f, 1.0f, 0.0f);
	fragmentVertex.m_TextureCoordinates = Vector2(1.0f, 0.0f);
	initialFragment.m_Vertices.push_back(fragmentVertex);

	fragmentVertex.m_Position = Vector3(-1.0f, 1.0f, 0.0f);
	fragmentVertex.m_TextureCoordinates = Vector2(0.0f, 0.0f);
	initialFragment.m_Vertices.push_back(fragmentVertex);

	initialFragment.GeneratePivotMatrices();
	m_PlayerOneFragments.push_back(initialFragment);

	const float minimumX = -1.0f;
	const float maximumX = (g_TwoPlayerMode) ? 0.0f : 1.0f;

	FragmentScreen(minimumX, maximumX, m_PlayerOneFragments);
	UpdatePlayerFBOMesh(m_PlayerOneFBOShatterMesh, m_PlayerOneFragments);
}



void TheGame::GenerateNewPlayerTwoFBOShatterMesh()
{
	m_PlayerTwoFragments.clear();
	ScreenFragment initialFragment;

	Vertex3D fragmentVertex;
	fragmentVertex.m_Color = RGBA::WHITE;

	fragmentVertex.m_Position = Vector3(0.0f, -1.0f, 0.0f);
	fragmentVertex.m_TextureCoordinates = Vector2(0.0f, 1.0f);
	initialFragment.m_Vertices.push_back(fragmentVertex);

	fragmentVertex.m_Position = Vector3(1.0f, -1.0f, 0.0f);
	fragmentVertex.m_TextureCoordinates = Vector2(1.0f, 1.0f);
	initialFragment.m_Vertices.push_back(fragmentVertex);

	fragmentVertex.m_Position = Vector3(1.0f, 1.0f, 0.0f);
	fragmentVertex.m_TextureCoordinates = Vector2(1.0f, 0.0f);
	initialFragment.m_Vertices.push_back(fragmentVertex);

	fragmentVertex.m_Position = Vector3(0.0f, 1.0f, 0.0f);
	fragmentVertex.m_TextureCoordinates = Vector2(0.0f, 0.0f);
	initialFragment.m_Vertices.push_back(fragmentVertex);

	initialFragment.GeneratePivotMatrices();
	m_PlayerTwoFragments.push_back(initialFragment);

	const float minimumX = 0.0f;
	const float maximumX = 1.0f;

	FragmentScreen(minimumX, maximumX, m_PlayerTwoFragments);
	UpdatePlayerFBOMesh(m_PlayerTwoFBOShatterMesh, m_PlayerTwoFragments);
}



void TheGame::FragmentScreen(float minimumX, float maximumX, std::deque<ScreenFragment>& playerFragments)
{
	const float LINE_MINIMUM = 0.0f;
	const float LINE_MAXIMUM = 100.0f;

	const float MINIMUM_THRESHOLD = -1.1f;
	const float MAXIMUM_THRESHOLD = 1.1f;

	const int NUMBER_OF_ITERATIONS = 16;

	for (int currentIteration = 0; currentIteration < NUMBER_OF_ITERATIONS; ++currentIteration)
	{
		float lineStartX = GetRandomFloatWithinRange(LINE_MINIMUM, LINE_MAXIMUM);
		float mappedStartX = RangeMap(lineStartX, LINE_MINIMUM, LINE_MAXIMUM, minimumX, maximumX);
		Vector2 lineStartingPoint = Vector2(mappedStartX, MINIMUM_THRESHOLD);

		float lineEndX = GetRandomFloatWithinRange(LINE_MINIMUM, LINE_MAXIMUM);
		float mappedEndX = RangeMap(lineEndX, LINE_MINIMUM, LINE_MAXIMUM, minimumX, maximumX);
		Vector2 lineEndingPoint = Vector2(mappedEndX, MAXIMUM_THRESHOLD);

		GenerateNewFragments(lineStartingPoint, lineEndingPoint, playerFragments);
	}

	float minimumY = -1.0f;
	float maximumY = 1.0f;

	for (int currentIteration = 0; currentIteration < NUMBER_OF_ITERATIONS; ++currentIteration)
	{
		float lineStartY = GetRandomFloatWithinRange(LINE_MINIMUM, LINE_MAXIMUM);
		float mappedStartY = RangeMap(lineStartY, LINE_MINIMUM, LINE_MAXIMUM, minimumY, maximumY);
		Vector2 lineStartingPoint = Vector2(MINIMUM_THRESHOLD, mappedStartY);

		float lineEndY = GetRandomFloatWithinRange(LINE_MINIMUM, LINE_MAXIMUM);
		float mappedEndY = RangeMap(lineEndY, LINE_MINIMUM, LINE_MAXIMUM, minimumY, maximumY);
		Vector2 lineEndingPoint = Vector2(MAXIMUM_THRESHOLD, mappedEndY);

		GenerateNewFragments(lineStartingPoint, lineEndingPoint, playerFragments);
	}
}



void TheGame::UpdatePlayerFBOMesh(Mesh* playerFBOMesh, std::deque<ScreenFragment>& playerFragments, float deltaTimeInSeconds /*= 0.0f*/)
{
	std::vector<Vertex3D> frameVertices;
	std::vector<uint32_t> frameIndices;

	for (ScreenFragment& currentFragment : playerFragments)
	{
		size_t indexStride = frameVertices.size();
		size_t numberOfFragmentVertices = currentFragment.m_Vertices.size();

		std::vector<uint32_t> currentFragmentIndices;
		AdvancedRenderer::SingletonInstance()->TriangulatePolygon(numberOfFragmentVertices, currentFragmentIndices);

		for (size_t currentIndex = 0; currentIndex < currentFragmentIndices.size(); ++currentIndex)
		{
			frameIndices.push_back(currentFragmentIndices[currentIndex] + indexStride);
		}

		currentFragment.GetUpdatedVertices(deltaTimeInSeconds, frameVertices);
	}

	playerFBOMesh->WriteToMesh(frameVertices.data(), frameIndices.data(), frameVertices.size(), frameIndices.size());
}



void TheGame::GenerateNewFragments(const Vector2& lineStartingPoint, const Vector2& lineEndingPoint, std::deque<ScreenFragment>& playerFragments)
{
	size_t numberOfExistingFragments = playerFragments.size();
	for (size_t fragmentIndex = 0; fragmentIndex < numberOfExistingFragments; ++fragmentIndex)
	{
		ScreenFragment firstFragment;
		ScreenFragment secondFragment;
		bool fragmentSliced = SliceScreenFragment(playerFragments[fragmentIndex], lineStartingPoint, lineEndingPoint, firstFragment, secondFragment);
		if (fragmentSliced)
		{
			firstFragment.GeneratePivotMatrices();
			secondFragment.GeneratePivotMatrices();

			playerFragments.erase(playerFragments.begin() + fragmentIndex);
			playerFragments.push_back(firstFragment);
			playerFragments.push_back(secondFragment);
		}
	}
}



bool TheGame::ScreenFragmentGetsSliced(const ScreenFragment& currentFragment, const Vector2& lineStartingPoint, const Vector2& lineEndingPoint) const
{
	std::vector<int> pointSides;

	for (size_t vertexIndex = 0; vertexIndex < currentFragment.m_Vertices.size(); ++vertexIndex)
	{
		Vector2 currentPoint = Vector2(currentFragment.m_Vertices[vertexIndex].m_Position.X, currentFragment.m_Vertices[vertexIndex].m_Position.Y);
		int currentPointSide = GetPositionOfPointRelativeToLine(lineStartingPoint, lineEndingPoint, currentPoint);
		pointSides.push_back(currentPointSide);
	}

	int comparisonSide = pointSides[0];
	for (size_t pointIndex = 0; pointIndex < pointSides.size(); ++pointIndex)
	{
		if (pointSides[pointIndex] != comparisonSide)
		{
			return true;
		}
	}

	return false;
}



bool TheGame::SliceScreenFragment(const ScreenFragment& currentFragment, const Vector2& lineStartingPoint, const Vector2& lineEndingPoint, ScreenFragment& firstFragment, ScreenFragment& secondFragment) const
{
	if (!ScreenFragmentGetsSliced(currentFragment, lineStartingPoint, lineEndingPoint))
	{
		return false;
	}

	size_t numberOfPoints = currentFragment.m_Vertices.size();

	Vertex3D firstPointVertex;
	Vertex3D secondPointVertex;
	Vertex3D newVertex;

	int firstPointSide;
	int secondPointSide;

	firstPointVertex = currentFragment.m_Vertices[numberOfPoints - 1];
	firstPointSide = GetPositionOfPointRelativeToLine(lineStartingPoint, lineEndingPoint, Vector2(firstPointVertex.m_Position.X, firstPointVertex.m_Position.Y));

	for (size_t pointIndex = 0; pointIndex < numberOfPoints; ++pointIndex)
	{
		secondPointVertex = currentFragment.m_Vertices[pointIndex];
		secondPointSide = GetPositionOfPointRelativeToLine(lineStartingPoint, lineEndingPoint, Vector2(secondPointVertex.m_Position.X, secondPointVertex.m_Position.Y));

		if (secondPointSide == -1)
		{
			if (firstPointSide == 1)
			{
				newVertex = GenerateNewIntersectionVertex(lineStartingPoint, lineEndingPoint, firstPointVertex, secondPointVertex);
				firstFragment.m_Vertices.push_back(newVertex);
				secondFragment.m_Vertices.push_back(newVertex);
			}

			firstFragment.m_Vertices.push_back(secondPointVertex);
		}
		else if (secondPointSide == 1)
		{
			if (firstPointSide == -1)
			{
				newVertex = GenerateNewIntersectionVertex(lineStartingPoint, lineEndingPoint, firstPointVertex, secondPointVertex);
				firstFragment.m_Vertices.push_back(newVertex);
				secondFragment.m_Vertices.push_back(newVertex);
			}

			secondFragment.m_Vertices.push_back(secondPointVertex);
		}
		else
		{
			firstFragment.m_Vertices.push_back(secondPointVertex);
			secondFragment.m_Vertices.push_back(secondPointVertex);
		}

		firstPointVertex = secondPointVertex;
		firstPointSide = secondPointSide;
	}

	return true;
}



Vertex3D TheGame::GenerateNewIntersectionVertex(const Vector2& lineStartingPoint, const Vector2& lineEndingPoint, const Vertex3D& firstFragmentVertex, const Vertex3D& secondFragmentVertex) const
{
	Vector2 firstFragmentPoint = Vector2(firstFragmentVertex.m_Position.X, firstFragmentVertex.m_Position.Y);
	Vector2 secondFragmentPoint = Vector2(secondFragmentVertex.m_Position.X, secondFragmentVertex.m_Position.Y);
	
	Vector2 intersectionPoint;
	bool intersectionPointFound = GetIntersectionPointFor2DLines(lineStartingPoint, lineEndingPoint, firstFragmentPoint, secondFragmentPoint, intersectionPoint);
	ASSERT_OR_DIE(intersectionPointFound, "Intersection point not found.");

	float fractionOfIntersection = CalculateEuclidianDistanceIn2D(firstFragmentPoint, intersectionPoint) / CalculateEuclidianDistanceIn2D(firstFragmentPoint, secondFragmentPoint);

	float textureCoordinateX = firstFragmentVertex.m_TextureCoordinates.X + (secondFragmentVertex.m_TextureCoordinates.X - firstFragmentVertex.m_TextureCoordinates.X) * fractionOfIntersection;
	float textureCoordinateY = firstFragmentVertex.m_TextureCoordinates.Y + (secondFragmentVertex.m_TextureCoordinates.Y - firstFragmentVertex.m_TextureCoordinates.Y) * fractionOfIntersection;

	Vertex3D newVertex;
	newVertex.m_Color = RGBA::WHITE;

	newVertex.m_Position = Vector3(intersectionPoint.X, intersectionPoint.Y, 0.0f);
	newVertex.m_TextureCoordinates = Vector2(textureCoordinateX, textureCoordinateY);

	return newVertex;
}