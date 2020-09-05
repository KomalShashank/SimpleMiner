#include "Game/MainMenu.hpp"



const float ORTHOGRAPHIC_NEAR_DISTANCE = -1.0f;
const float ORTHOGRAPHIC_FAR_DISTANCE = 1.0f;

const size_t NUMBER_OF_VERTICES = 4;
const size_t NUMBER_OF_INDICES = 6;



Texture* MainMenu::s_MenuTextures[NUMBER_OF_MENU_TEXTURES];



MainMenu::MainMenu() :
m_StartMenuIsOpen(false),
m_ControlsMenuIsOpen(false),
m_CreditsMenuIsOpen(false),
m_selectedMenuButton(START_MENU_BUTTON)
{
	SoundID menuMusicID = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Music/MainMenu/MenuMusic.mp3");
	m_MenuMusic = AudioSystem::SingletonInstance()->PlaySound(menuMusicID, LOOPING_PLAYBACK_MODE, 0.125f);

	m_MenuElementMesh = new Mesh();
	m_MenuMaterial = new Material("Data/Shaders/UIShader.vert", "Data/Shaders/UIShader.frag");

	ResetButtonHighlights();
}



MainMenu::~MainMenu()
{
	AudioSystem::SingletonInstance()->StopSound(m_MenuMusic);

	delete m_MenuElementMesh;
	delete m_MenuMaterial;
}



void MainMenu::InitializeMainMenuTextures()
{
	Texture* currentMenuTexture;
	SamplerData textureSamplerData = SamplerData(REPEAT_WRAP, REPEAT_WRAP, LINEAR_FILTER, NEAREST_FILTER);

	currentMenuTexture = Texture::CreateOrGetTexture("Data/Images/MainMenu/TitleSplash.png", textureSamplerData);
	s_MenuTextures[TITLE_SPLASH_TEXTURE] = currentMenuTexture;

	currentMenuTexture = Texture::CreateOrGetTexture("Data/Images/MainMenu/StartButton.png", textureSamplerData);
	s_MenuTextures[START_BUTTON_TEXTURE] = currentMenuTexture;

	currentMenuTexture = Texture::CreateOrGetTexture("Data/Images/MainMenu/ControlsButton.png", textureSamplerData);
	s_MenuTextures[CONTROLS_BUTTON_TEXTURE] = currentMenuTexture;

	currentMenuTexture = Texture::CreateOrGetTexture("Data/Images/MainMenu/CreditsButton.png", textureSamplerData);
	s_MenuTextures[CREDITS_BUTTON_TEXTURE] = currentMenuTexture;

	currentMenuTexture = Texture::CreateOrGetTexture("Data/Images/MainMenu/QuitButton.png", textureSamplerData);
	s_MenuTextures[QUIT_BUTTON_TEXTURE] = currentMenuTexture;

	currentMenuTexture = Texture::CreateOrGetTexture("Data/Images/MainMenu/StartScreen.png", textureSamplerData);
	s_MenuTextures[START_SCREEN_TEXTURE] = currentMenuTexture;

	currentMenuTexture = Texture::CreateOrGetTexture("Data/Images/MainMenu/OnePlayerButton.png", textureSamplerData);
	s_MenuTextures[ONE_PLAYER_BUTTON_TEXTURE] = currentMenuTexture;

	currentMenuTexture = Texture::CreateOrGetTexture("Data/Images/MainMenu/TwoPlayerButton.png", textureSamplerData);
	s_MenuTextures[TWO_PLAYER_BUTTON_TEXTURE] = currentMenuTexture;

	currentMenuTexture = Texture::CreateOrGetTexture("Data/Images/MainMenu/ControlsScreen.png", textureSamplerData);
	s_MenuTextures[CONTROLS_SCREEN_TEXTURE] = currentMenuTexture;

	currentMenuTexture = Texture::CreateOrGetTexture("Data/Images/MainMenu/CreditsScreen.png", textureSamplerData);
	s_MenuTextures[CREDITS_SCREEN_TEXTURE] = currentMenuTexture;

	currentMenuTexture = Texture::CreateOrGetTexture("Data/Images/MainMenu/BackButton.png", textureSamplerData);
	s_MenuTextures[BACK_BUTTON_TEXTURE] = currentMenuTexture;
}



void MainMenu::Update()
{
	ResetButtonHighlights();

	DetermineSelectedButton();
	ActivateSelectedButton();
}



void MainMenu::Render() const
{
	AdvancedRenderer::SingletonInstance()->SetProjectionViewport(IntVector2::ZERO, IntVector2(WINDOW_PHYSICAL_WIDTH, WINDOW_PHYSICAL_HEIGHT));
	AdvancedRenderer::SingletonInstance()->EnableDepthTesting(false);

	Matrix4 modelMatrix = AdvancedRenderer::SingletonInstance()->GetModelMatrix(Vector3::ONE, EulerAngles::ZERO, Vector3::ZERO);
	Matrix4 viewMatrix = AdvancedRenderer::SingletonInstance()->GetViewMatrix(Vector3::ZERO, EulerAngles::ZERO);
	Matrix4 projectionMatrix = AdvancedRenderer::SingletonInstance()->GetOrthographicProjectionMatrix(Vector2::ZERO, Vector2(VIEW_WIDTH, VIEW_HEIGHT), ORTHOGRAPHIC_NEAR_DISTANCE, ORTHOGRAPHIC_FAR_DISTANCE);

	AdvancedRenderer::SingletonInstance()->UpdateModelMatrix(modelMatrix);
	AdvancedRenderer::SingletonInstance()->UpdateViewMatrix(viewMatrix);
	AdvancedRenderer::SingletonInstance()->UpdateProjectionMatrix(projectionMatrix);

	if (!m_StartMenuIsOpen && !m_ControlsMenuIsOpen && !m_CreditsMenuIsOpen)
	{
		DrawMenuElement(s_MenuTextures[TITLE_SPLASH_TEXTURE]);
		DrawMenuElement(s_MenuTextures[START_BUTTON_TEXTURE], m_MenuButtonHighlights[START_MENU_BUTTON]);
		DrawMenuElement(s_MenuTextures[CONTROLS_BUTTON_TEXTURE], m_MenuButtonHighlights[CONTROLS_MENU_BUTTON]);
		DrawMenuElement(s_MenuTextures[CREDITS_BUTTON_TEXTURE], m_MenuButtonHighlights[CREDITS_MENU_BUTTON]);
		DrawMenuElement(s_MenuTextures[QUIT_BUTTON_TEXTURE], m_MenuButtonHighlights[QUIT_MENU_BUTTON]);
	}
	else if (m_StartMenuIsOpen && !m_ControlsMenuIsOpen && !m_CreditsMenuIsOpen)
	{
		DrawMenuElement(s_MenuTextures[START_SCREEN_TEXTURE]);
		DrawMenuElement(s_MenuTextures[ONE_PLAYER_BUTTON_TEXTURE], m_MenuButtonHighlights[ONE_PLAYER_MENU_BUTTON]);
		DrawMenuElement(s_MenuTextures[TWO_PLAYER_BUTTON_TEXTURE], m_MenuButtonHighlights[TWO_PLAYER_MENU_BUTTON]);
		DrawMenuElement(s_MenuTextures[BACK_BUTTON_TEXTURE], m_MenuButtonHighlights[BACK_MENU_BUTTON]);
	}
	else if (!m_StartMenuIsOpen && m_ControlsMenuIsOpen && !m_CreditsMenuIsOpen)
	{
		DrawMenuElement(s_MenuTextures[CONTROLS_SCREEN_TEXTURE]);
		DrawMenuElement(s_MenuTextures[BACK_BUTTON_TEXTURE], m_MenuButtonHighlights[BACK_MENU_BUTTON]);
	}
	else if (!m_StartMenuIsOpen && !m_ControlsMenuIsOpen && m_CreditsMenuIsOpen)
	{
		DrawMenuElement(s_MenuTextures[CREDITS_SCREEN_TEXTURE]);
		DrawMenuElement(s_MenuTextures[BACK_BUTTON_TEXTURE], m_MenuButtonHighlights[BACK_MENU_BUTTON]);
	}
}



void MainMenu::ResetButtonHighlights()
{
	for (int buttonIndex = 0; buttonIndex < NUMBER_OF_MENU_BUTTONS; ++buttonIndex)
	{
		m_MenuButtonHighlights[buttonIndex] = RGBA::WHITE;
	}
}



void MainMenu::DetermineSelectedButton()
{
	int8_t selection = (int8_t)m_selectedMenuButton;

	if (InputSystem::SingletonInstance()->ButtonWasJustPressed(0, D_PAD_DOWN) || InputSystem::SingletonInstance()->ButtonWasJustPressed(1, D_PAD_DOWN))
	{
		SoundID selectionSound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/HUDSelection.ogg");
		AudioSystem::SingletonInstance()->PlaySound(selectionSound, FORWARD_PLAYBACK_MODE, 0.5f);

		++selection;
	}
	else if (InputSystem::SingletonInstance()->ButtonWasJustPressed(0, D_PAD_UP) || InputSystem::SingletonInstance()->ButtonWasJustPressed(1, D_PAD_UP))
	{
		SoundID selectionSound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/HUDSelection.ogg");
		AudioSystem::SingletonInstance()->PlaySound(selectionSound, FORWARD_PLAYBACK_MODE, 0.5f);

		--selection;
	}

	if (!m_StartMenuIsOpen && !m_ControlsMenuIsOpen && !m_CreditsMenuIsOpen)
	{
		selection = (int8_t)ClampInt(selection, START_MENU_BUTTON, QUIT_MENU_BUTTON);
	}
	else if (m_StartMenuIsOpen && !m_ControlsMenuIsOpen && !m_CreditsMenuIsOpen)
	{
		selection = (int8_t)ClampInt(selection, ONE_PLAYER_MENU_BUTTON, BACK_MENU_BUTTON);
	}
	else if (!m_StartMenuIsOpen && m_ControlsMenuIsOpen && !m_CreditsMenuIsOpen)
	{
		selection = (int8_t)ClampInt(selection, BACK_MENU_BUTTON, BACK_MENU_BUTTON);
	}
	else if (!m_StartMenuIsOpen && !m_ControlsMenuIsOpen && m_CreditsMenuIsOpen)
	{
		selection = (int8_t)ClampInt(selection, BACK_MENU_BUTTON, BACK_MENU_BUTTON);
	}

	m_selectedMenuButton = (uint8_t)selection;
}



void MainMenu::ActivateSelectedButton()
{
	if (InputSystem::SingletonInstance()->ButtonWasJustPressed(0, A_BUTTON) || InputSystem::SingletonInstance()->ButtonWasJustPressed(1, A_BUTTON))
	{
		if (!m_StartMenuIsOpen && !m_ControlsMenuIsOpen && !m_CreditsMenuIsOpen)
		{
			SoundID activationSound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/ButtonActivation.ogg");
			AudioSystem::SingletonInstance()->PlaySound(activationSound, FORWARD_PLAYBACK_MODE);

			switch (m_selectedMenuButton)
			{
			case START_MENU_BUTTON:
				m_StartMenuIsOpen = true;
				m_selectedMenuButton = ONE_PLAYER_MENU_BUTTON;
				break;

			case CONTROLS_MENU_BUTTON:
				m_ControlsMenuIsOpen = true;
				m_selectedMenuButton = BACK_MENU_BUTTON;
				break;

			case CREDITS_MENU_BUTTON:
				m_CreditsMenuIsOpen = true;
				m_selectedMenuButton = BACK_MENU_BUTTON;
				break;

			case QUIT_MENU_BUTTON:
				g_IsQuitting = true;
				break;
			}
		}
		else if (m_StartMenuIsOpen && !m_ControlsMenuIsOpen && !m_CreditsMenuIsOpen)
		{
			SoundID activationSound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/ButtonActivation.ogg");
			AudioSystem::SingletonInstance()->PlaySound(activationSound, FORWARD_PLAYBACK_MODE);

			switch (m_selectedMenuButton)
			{
			case ONE_PLAYER_MENU_BUTTON:
				g_TwoPlayerMode = false;
				g_LoadGame = true;
				break;

			case TWO_PLAYER_MENU_BUTTON:
				g_TwoPlayerMode = true;
				g_LoadGame = true;
				break;

			case BACK_MENU_BUTTON:
				m_StartMenuIsOpen = false;
				m_selectedMenuButton = START_MENU_BUTTON;
				break;
			}
		}
		else if (!m_StartMenuIsOpen && m_ControlsMenuIsOpen && !m_CreditsMenuIsOpen)
		{
			SoundID activationSound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/ButtonActivation.ogg");
			AudioSystem::SingletonInstance()->PlaySound(activationSound, FORWARD_PLAYBACK_MODE);

			m_ControlsMenuIsOpen = false;
			m_selectedMenuButton = CONTROLS_MENU_BUTTON;
		}
		else if (!m_StartMenuIsOpen && !m_ControlsMenuIsOpen && m_CreditsMenuIsOpen)
		{
			SoundID activationSound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/ButtonActivation.ogg");
			AudioSystem::SingletonInstance()->PlaySound(activationSound, FORWARD_PLAYBACK_MODE);

			m_CreditsMenuIsOpen = false;
			m_selectedMenuButton = CREDITS_MENU_BUTTON;
		}
	}
	else if (InputSystem::SingletonInstance()->ButtonWasJustPressed(0, B_BUTTON) || InputSystem::SingletonInstance()->ButtonWasJustPressed(1, B_BUTTON))
	{
		if (m_StartMenuIsOpen && !m_ControlsMenuIsOpen && !m_CreditsMenuIsOpen)
		{
			m_StartMenuIsOpen = false;
			m_selectedMenuButton = START_MENU_BUTTON;
		}
		else if (!m_StartMenuIsOpen && m_ControlsMenuIsOpen && !m_CreditsMenuIsOpen)
		{
			m_ControlsMenuIsOpen = false;
			m_selectedMenuButton = CONTROLS_MENU_BUTTON;
		}
		else if (!m_StartMenuIsOpen && !m_ControlsMenuIsOpen && m_CreditsMenuIsOpen)
		{
			m_CreditsMenuIsOpen = false;
			m_selectedMenuButton = CREDITS_MENU_BUTTON;
		}
	}

	m_MenuButtonHighlights[m_selectedMenuButton] = RGBA::RED;
}



void MainMenu::GetMenuElementMesh(const RGBA& elementTint) const
{
	Vertex3D meshVertices[NUMBER_OF_VERTICES];
	uint32_t meshIndices[NUMBER_OF_INDICES] = { 0, 1, 2, 2, 3, 0 };

	Vertex3D meshVertex;
	meshVertex.m_Color = elementTint;

	meshVertex.m_Position = Vector3(0.0f, 0.0f, 0.0f);
	meshVertex.m_TextureCoordinates = Vector2(0.0f, 1.0f);
	meshVertices[0] = meshVertex;

	meshVertex.m_Position = Vector3(VIEW_WIDTH, 0.0f, 0.0f);
	meshVertex.m_TextureCoordinates = Vector2(1.0f, 1.0f);
	meshVertices[1] = meshVertex;

	meshVertex.m_Position = Vector3(VIEW_WIDTH, VIEW_HEIGHT, 0.0f);
	meshVertex.m_TextureCoordinates = Vector2(1.0f, 0.0f);
	meshVertices[2] = meshVertex;

	meshVertex.m_Position = Vector3(0.0f, VIEW_HEIGHT, 0.0f);
	meshVertex.m_TextureCoordinates = Vector2(0.0f, 0.0f);
	meshVertices[3] = meshVertex;

	m_MenuElementMesh->WriteToMesh(&meshVertices[0], &meshIndices[0], NUMBER_OF_VERTICES, NUMBER_OF_INDICES);
}



void MainMenu::DrawMenuElement(Texture* elementTexture, const RGBA& elementTint /*= RGBA::WHITE*/) const
{
	GetMenuElementMesh(elementTint);
	m_MenuMaterial->SetDiffuseTexture(elementTexture);
	AdvancedRenderer::SingletonInstance()->DrawPolygonMesh(m_MenuElementMesh, NUMBER_OF_VERTICES, NUMBER_OF_INDICES, m_MenuMaterial);
}