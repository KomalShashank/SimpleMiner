#pragma once

#include "Game/GameCommons.hpp"



enum BlockType : uint8_t
{
	AIR_BLOCK,
	GRASS_BLOCK,
	DIRT_BLOCK,
	STONE_BLOCK,
	SAND_BLOCK,
	CLAY_BLOCK,
	GLOWSTONE_BLOCK,
	COBBLESTONE_BLOCK,
	CLAY_BRICK_BLOCK,
	STONE_BRICK_BLOCK,
	WATER_BLOCK,
	BEDROCK_BLOCK,
	NUMBER_OF_BLOCKS_TYPES,
	INVALID_BLOCK = 255U
};



class BlockDefinition
{
public:
	BlockDefinition();
	static void InitializeBlockDefinitions(const SpriteSheet& blockSpriteSheet);

public:
	static BlockDefinition s_BlockDefinitions[NUMBER_OF_BLOCKS_TYPES];

public:
	int m_illuminationValue;
	float m_Toughness;

	bool m_IsSolid;
	bool m_IsOpaque;
	bool m_IsVisible;

	AABB2 m_topFaceTextureAABB;
	AABB2 m_bottomFaceTextureAABB;
	AABB2 m_sideFaceTextureAABB;

	SoundID m_Breaking_Sound;
	SoundID m_Placing_Sound;
	SoundID m_Walking_Sound;
};



class Block
{
public:
	Block();
	Block(uint8_t blockType);

	uint8_t GetType() const;
	void SetType(uint8_t blockType);

	bool IsSolid() const;
	bool IsOpaque() const;
	bool IsVisible() const;

	AABB2 GetTopFaceTextureAABB() const;
	AABB2 GetBottomFaceTextureAABB() const;
	AABB2 GetSideFaceTextureAABB() const;

	SoundID GetBreakingSound() const;
	SoundID GetPlacingSound() const;
	SoundID GetWalkingSound() const;

	int GetInternalIlluminationValue() const;

	void SetSkyBit();
	void ClearSkyBit();

	void SetLightingDirty();
	void ClearLightingDirty();

	bool IsSky();
	bool LightingIsDirty();

	void SetLightValue(int lightValue);
	int GetLightValue();

private:
	uint8_t m_BlockType;
	uint8_t m_lightAndFlags;
};



inline uint8_t Block::GetType() const
{
	return m_BlockType;
}



inline void Block::SetType(uint8_t blockType)
{
	m_BlockType = blockType;
}



inline bool Block::IsSolid() const
{
	return BlockDefinition::s_BlockDefinitions[m_BlockType].m_IsSolid;
}



inline bool Block::IsOpaque() const
{
	return BlockDefinition::s_BlockDefinitions[m_BlockType].m_IsOpaque;
}



inline bool Block::IsVisible() const
{
	return BlockDefinition::s_BlockDefinitions[m_BlockType].m_IsVisible;
}



inline AABB2 Block::GetTopFaceTextureAABB() const
{
	return BlockDefinition::s_BlockDefinitions[m_BlockType].m_topFaceTextureAABB;
}



inline AABB2 Block::GetBottomFaceTextureAABB() const
{
	return BlockDefinition::s_BlockDefinitions[m_BlockType].m_bottomFaceTextureAABB;
}



inline AABB2 Block::GetSideFaceTextureAABB() const
{
	return BlockDefinition::s_BlockDefinitions[m_BlockType].m_sideFaceTextureAABB;
}



inline SoundID Block::GetBreakingSound() const
{
	return BlockDefinition::s_BlockDefinitions[m_BlockType].m_Breaking_Sound;
}



inline SoundID Block::GetPlacingSound() const
{
	return BlockDefinition::s_BlockDefinitions[m_BlockType].m_Placing_Sound;
}



inline SoundID Block::GetWalkingSound() const
{
	return BlockDefinition::s_BlockDefinitions[m_BlockType].m_Walking_Sound;
}

