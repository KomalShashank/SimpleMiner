#pragma once

#include "Game/GameCommons.hpp"
#include "Game/MainMenu.hpp"
#include "Game/World.hpp"



const int NUMBER_OF_MUSIC_TRACKS = 8;



enum CameraMode : uint8_t
{
	FIRST_PERSON_MODE,
	THIRD_PERSON_MODE,
	NUMBER_OF_CAMERA_MODES,
	INVALID_CAMERA_MODE = 255U
};



class HUDTileDefinition
{
public:
	HUDTileDefinition();
	static void InitializeHUDTileDefinitions(const SpriteSheet& tileSpriteSheet);

public:
	static HUDTileDefinition s_HUDTileDefinitions[NUMBER_OF_HUD_TILES];

public:
	AABB2 m_tileTextureBounds;
	AABB2 m_tileBounds;
};



class ScreenFragment
{
public:
	ScreenFragment();
	void GeneratePivotMatrices();
	void GetUpdatedVertices(float deltaTimeInSeconds, std::vector<Vertex3D>& fragmentVertices);

private:
	Vector3 GetRandomLinearVelocity() const;

public:
	std::vector<Vertex3D> m_Vertices;

private:
	Vector3 m_LinearVelocity;
	float m_AngularVelocity;

	Vector3 m_LinearOffset;
	float m_AngularOffset;

	Matrix4 m_PivotMatrix;
	Matrix4 m_InversePivotMatrix;
};



class TheGame
{
public:
	TheGame();
	~TheGame();

	void Update(float deltaTimeInSeconds);
	void Render() const;

private:
	void OnePlayerUpdateCall(float deltaTimeInSeconds);
	void TwoPlayerUpdateCall(float deltaTimeInSeconds);

	void OnePlayerRenderCall() const;
	void TwoPlayerRenderCall() const;

	void RenderPlayerOne() const;
	void RenderPlayerTwo() const;

	void RenderPlayerWorld(const Camera3D* playerCamera, FrameBuffer* currentPlayerFBO, uint8_t currentPlayerID) const;

	void RenderPlayerOneUI(const IntVector2& bottomLeft, const IntVector2& viewSize) const;
	void RenderPlayerTwoUI(const IntVector2& bottomLeft, const IntVector2& viewSize) const;

	static void InitializeMusicTracks(void*);
	void PlayBackgroundMusic();

	void InitializePlayerDefaults();
	void UninitializePlayerDefaults();

	void LoadGame();
	void UnloadGame();

	void ExitToMainMenu(float deltaTimeInSeconds);

	void CreateCrosshairsMesh();
	void CreateBlockSelectionMeshAndMaterial();
	void CreateDepthOfFieldAndDeathEffectMaterials();
	void CreateSplitScreenPartitionMesh();

	void CreatePlayerOneFBODepthMesh();
	void CreatePlayerOneFBOShatterMesh();
	void CreatePlayerTwoFBODepthMesh();
	void CreatePlayerTwoFBOShatterMesh();
	void CreateFullScreenFBOMesh();

	void ToggleDebugMode();
	void ToggleScreenshotMode();

	void SetUpDrawingIn3D(const Camera3D* playerCamera) const;
	void SetUpDrawingIn2D() const;

	void CameraLookWithMouseMovement();
	void CameraLook(Camera3D* playerCamera, uint8_t currentPlayerID, float deltaTimeInSeconds);

	void UpdatePlayerDirections(Player* currentPlayer, const Camera3D* playerCamera);

	void KillPlayerAccordingly(Player* currentPlayer);
	void ResurrectPlayerAccordingly(Player* currentPlayer, uint8_t currentPlayerID);

	void ToggleCameraMode(uint8_t& playerCameraMode, uint8_t currentPlayerID);
	void UpdateCameraPosition(const Player* currentPlayer, Camera3D* playerCamera, uint8_t playerCameraMode);

	void SelectBlocksWithMouseWheel(uint8_t& selectedBlock);
	void SelectBlocksWithKeyboardInput(uint8_t& selectedBlock);

	void SelectBlocks(uint8_t& selectedBlock, uint8_t currentPlayerID);

	void DrawOriginLinesIn3D() const;

	void DrawCrosshairs2D() const;
	//void DrawDeathScreen2D(const Vector2& bottomLeft, const Vector2& topRight) const;
	void DisplayCameraStatsIn2D(const Camera3D& playerCamera, uint8_t playerCameraMode, uint8_t playerPhysicsMode, uint8_t currentPlayerID) const;

	void DrawBlockSelection2D(const uint8_t& selectedBlock) const;
	void DrawSplitScreenPartition2D() const;

	//void DrawCompass2D(const Camera3D* playerCamera) const;

	void GenerateNewPlayerOneFBOShatterMesh();
	void GenerateNewPlayerTwoFBOShatterMesh();

	void FragmentScreen(float minimumX, float maximumX, std::deque<ScreenFragment>& playerFragments);
	void UpdatePlayerFBOMesh(Mesh* playerFBOMesh, std::deque<ScreenFragment>& playerFragments, float deltaTimeInSeconds = 0.0f);

	void GenerateNewFragments(const Vector2& lineStartingPoint, const Vector2& lineEndingPoint, std::deque<ScreenFragment>& playerFragments);
	bool ScreenFragmentGetsSliced(const ScreenFragment& currentFragment, const Vector2& lineStartingPoint, const Vector2& lineEndingPoint) const;
	bool SliceScreenFragment(const ScreenFragment& currentFragment, const Vector2& lineStartingPoint, const Vector2& lineEndingPoint, ScreenFragment& firstFragment, ScreenFragment& secondFragment) const;
	Vertex3D GenerateNewIntersectionVertex(const Vector2& lineStartingPoint, const Vector2& lineEndingPoint, const Vertex3D& firstFragmentVertex, const Vertex3D& secondFragmentVertex) const;

public:
	Camera3D* m_PlayerOneCamera;
	uint8_t m_PlayerOneSelectedBlock;

	Camera3D* m_PlayerTwoCamera;
	uint8_t m_PlayerTwoSelectedBlock;

private:
	static SoundID s_MusicTracks[NUMBER_OF_MUSIC_TRACKS];
	AudioChannelHandle m_MusicChannel;
	Thread* m_MusicLoadingThread;

	uint8_t m_PlayerOneCameraMode;
	uint8_t m_PlayerTwoCameraMode;

	MonospaceFont* m_HUDFont;
	SpriteSheet* m_HUDSpriteSheet;

	MainMenu* m_MainMenu;
	World* m_World;

	Mesh* m_CrosshairsMesh;

	Mesh* m_BlockSelectionMesh;
	Mesh* m_BlockSelectionOutlineMesh;
	Material* m_BlockSelectionMaterial;

	Material* m_DepthOfFieldMaterial;
	Material* m_DeathEffectMaterial;

	Mesh* m_SplitScreenPartitionMesh;

	std::deque<ScreenFragment> m_PlayerOneFragments;
	std::deque<ScreenFragment> m_PlayerTwoFragments;

	FrameBuffer* m_PlayerOneFirstFBO;
	FrameBuffer* m_PlayerOneSecondFBO;

	FrameBuffer* m_PlayerTwoFirstFBO;
	FrameBuffer* m_PlayerTwoSecondFBO;

	FrameBuffer* m_ReflectionFBO;
	FrameBuffer* m_RefractionFBO;

	FrameBuffer* m_FullScreenFBO;

	Mesh* m_PlayerOneFBODepthMesh;
	Mesh* m_PlayerOneFBOShatterMesh;

	Mesh* m_PlayerTwoFBODepthMesh;
	Mesh* m_PlayerTwoFBOShatterMesh;

	Mesh* m_FullScreenFBOMesh;
};

extern TheGame* g_TheGame;