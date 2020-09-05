#pragma once

#include "Game/GameCommons.hpp"



enum MenuTextureID : uint8_t
{
	TITLE_SPLASH_TEXTURE,
	START_BUTTON_TEXTURE,
	CONTROLS_BUTTON_TEXTURE,
	CREDITS_BUTTON_TEXTURE,
	QUIT_BUTTON_TEXTURE,
	START_SCREEN_TEXTURE,
	ONE_PLAYER_BUTTON_TEXTURE,
	TWO_PLAYER_BUTTON_TEXTURE,
	CONTROLS_SCREEN_TEXTURE,
	CREDITS_SCREEN_TEXTURE,
	BACK_BUTTON_TEXTURE,
	NUMBER_OF_MENU_TEXTURES,
	INVALID_TEXTURE = 255U
};



enum MenuButton : uint8_t
{
	START_MENU_BUTTON,
	CONTROLS_MENU_BUTTON,
	CREDITS_MENU_BUTTON,
	QUIT_MENU_BUTTON,
	ONE_PLAYER_MENU_BUTTON,
	TWO_PLAYER_MENU_BUTTON,
	BACK_MENU_BUTTON,
	NUMBER_OF_MENU_BUTTONS,
	INVALID_MENU_BUTTON = 255U
};



class MainMenu
{
public:
	MainMenu();
	~MainMenu();

	static void InitializeMainMenuTextures();

	void Update();
	void Render() const;

private:
	void ResetButtonHighlights();
	void DetermineSelectedButton();
	void ActivateSelectedButton();

	void GetMenuElementMesh(const RGBA& elementTint) const;
	void DrawMenuElement(Texture* elementTexture, const RGBA& elementTint = RGBA::WHITE) const;

private:
	static Texture* s_MenuTextures[NUMBER_OF_MENU_TEXTURES];

	bool m_StartMenuIsOpen;
	bool m_ControlsMenuIsOpen;
	bool m_CreditsMenuIsOpen;

	uint8_t m_selectedMenuButton;
	AudioChannelHandle m_MenuMusic;
	RGBA m_MenuButtonHighlights[NUMBER_OF_MENU_BUTTONS];

	Mesh* m_MenuElementMesh;
	Material* m_MenuMaterial;
};