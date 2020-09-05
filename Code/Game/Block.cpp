#include "Game/Block.hpp"



BlockDefinition::BlockDefinition()
{

}



void BlockDefinition::InitializeBlockDefinitions(const SpriteSheet& blockSpriteSheet)
{
	s_BlockDefinitions[AIR_BLOCK].m_illuminationValue = 0;
	s_BlockDefinitions[AIR_BLOCK].m_Toughness = 0.0f;
	s_BlockDefinitions[AIR_BLOCK].m_IsSolid = false;
	s_BlockDefinitions[AIR_BLOCK].m_IsOpaque = false;
	s_BlockDefinitions[AIR_BLOCK].m_IsVisible = false;
	s_BlockDefinitions[AIR_BLOCK].m_topFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(0, 0));
	s_BlockDefinitions[AIR_BLOCK].m_bottomFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(0, 0));
	s_BlockDefinitions[AIR_BLOCK].m_sideFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(0, 0));

	s_BlockDefinitions[GRASS_BLOCK].m_illuminationValue = 0;
	s_BlockDefinitions[GRASS_BLOCK].m_Toughness = 3.0f;
	s_BlockDefinitions[GRASS_BLOCK].m_IsSolid = true;
	s_BlockDefinitions[GRASS_BLOCK].m_IsOpaque = true;
	s_BlockDefinitions[GRASS_BLOCK].m_IsVisible = true;
	s_BlockDefinitions[GRASS_BLOCK].m_topFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(2, 0));
	s_BlockDefinitions[GRASS_BLOCK].m_bottomFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(3, 0));
	s_BlockDefinitions[GRASS_BLOCK].m_sideFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(1, 0));
	s_BlockDefinitions[GRASS_BLOCK].m_Breaking_Sound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/GrassPlacingAndBreaking.ogg");
	s_BlockDefinitions[GRASS_BLOCK].m_Placing_Sound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/GrassPlacingAndBreaking.ogg");
	s_BlockDefinitions[GRASS_BLOCK].m_Walking_Sound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/GrassWalking.ogg");

	s_BlockDefinitions[DIRT_BLOCK].m_illuminationValue = 0;
	s_BlockDefinitions[DIRT_BLOCK].m_Toughness = 3.0f;
	s_BlockDefinitions[DIRT_BLOCK].m_IsSolid = true;
	s_BlockDefinitions[DIRT_BLOCK].m_IsOpaque = true;
	s_BlockDefinitions[DIRT_BLOCK].m_IsVisible = true;
	s_BlockDefinitions[DIRT_BLOCK].m_topFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(3, 0));
	s_BlockDefinitions[DIRT_BLOCK].m_bottomFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(3, 0));
	s_BlockDefinitions[DIRT_BLOCK].m_sideFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(3, 0));
	s_BlockDefinitions[DIRT_BLOCK].m_Breaking_Sound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/DirtPlacingAndBreaking.ogg");
	s_BlockDefinitions[DIRT_BLOCK].m_Placing_Sound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/DirtPlacingAndBreaking.ogg");

	s_BlockDefinitions[STONE_BLOCK].m_illuminationValue = 0;
	s_BlockDefinitions[STONE_BLOCK].m_Toughness = 5.0f;
	s_BlockDefinitions[STONE_BLOCK].m_IsSolid = true;
	s_BlockDefinitions[STONE_BLOCK].m_IsOpaque = true;
	s_BlockDefinitions[STONE_BLOCK].m_IsVisible = true;
	s_BlockDefinitions[STONE_BLOCK].m_topFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(0, 1));
	s_BlockDefinitions[STONE_BLOCK].m_bottomFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(0, 1));
	s_BlockDefinitions[STONE_BLOCK].m_sideFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(0, 1));
	s_BlockDefinitions[STONE_BLOCK].m_Breaking_Sound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/StonePlacingAndBreaking.ogg");
	s_BlockDefinitions[STONE_BLOCK].m_Placing_Sound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/StonePlacingAndBreaking.ogg");

	s_BlockDefinitions[SAND_BLOCK].m_illuminationValue = 0;
	s_BlockDefinitions[SAND_BLOCK].m_Toughness = 1.0f;
	s_BlockDefinitions[SAND_BLOCK].m_IsSolid = true;
	s_BlockDefinitions[SAND_BLOCK].m_IsOpaque = true;
	s_BlockDefinitions[SAND_BLOCK].m_IsVisible = true;
	s_BlockDefinitions[SAND_BLOCK].m_topFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(1, 1));
	s_BlockDefinitions[SAND_BLOCK].m_bottomFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(1, 1));
	s_BlockDefinitions[SAND_BLOCK].m_sideFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(1, 1));
	s_BlockDefinitions[SAND_BLOCK].m_Breaking_Sound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/SandPlacingAndBreaking.ogg");
	s_BlockDefinitions[SAND_BLOCK].m_Placing_Sound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/SandPlacingAndBreaking.ogg");

	s_BlockDefinitions[CLAY_BLOCK].m_illuminationValue = 0;
	s_BlockDefinitions[CLAY_BLOCK].m_Toughness = 2.0f;
	s_BlockDefinitions[CLAY_BLOCK].m_IsSolid = true;
	s_BlockDefinitions[CLAY_BLOCK].m_IsOpaque = true;
	s_BlockDefinitions[CLAY_BLOCK].m_IsVisible = true;
	s_BlockDefinitions[CLAY_BLOCK].m_topFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(2, 1));
	s_BlockDefinitions[CLAY_BLOCK].m_bottomFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(2, 1));
	s_BlockDefinitions[CLAY_BLOCK].m_sideFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(2, 1));
	s_BlockDefinitions[CLAY_BLOCK].m_Breaking_Sound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/ClayPlacingAndBreaking.ogg");
	s_BlockDefinitions[CLAY_BLOCK].m_Placing_Sound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/ClayPlacingAndBreaking.ogg");

	s_BlockDefinitions[GLOWSTONE_BLOCK].m_illuminationValue = 12;
	s_BlockDefinitions[GLOWSTONE_BLOCK].m_Toughness = 4.0f;
	s_BlockDefinitions[GLOWSTONE_BLOCK].m_IsSolid = true;
	s_BlockDefinitions[GLOWSTONE_BLOCK].m_IsOpaque = true;
	s_BlockDefinitions[GLOWSTONE_BLOCK].m_IsVisible = true;
	s_BlockDefinitions[GLOWSTONE_BLOCK].m_topFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(3, 1));
	s_BlockDefinitions[GLOWSTONE_BLOCK].m_bottomFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(3, 1));
	s_BlockDefinitions[GLOWSTONE_BLOCK].m_sideFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(3, 1));
	s_BlockDefinitions[GLOWSTONE_BLOCK].m_Breaking_Sound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/GlowstonePlacingAndBreaking.ogg");
	s_BlockDefinitions[GLOWSTONE_BLOCK].m_Placing_Sound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/GlowstonePlacingAndBreaking.ogg");

	s_BlockDefinitions[COBBLESTONE_BLOCK].m_illuminationValue = 0;
	s_BlockDefinitions[COBBLESTONE_BLOCK].m_Toughness = 4.0f;
	s_BlockDefinitions[COBBLESTONE_BLOCK].m_IsSolid = true;
	s_BlockDefinitions[COBBLESTONE_BLOCK].m_IsOpaque = true;
	s_BlockDefinitions[COBBLESTONE_BLOCK].m_IsVisible = true;
	s_BlockDefinitions[COBBLESTONE_BLOCK].m_topFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(0, 2));
	s_BlockDefinitions[COBBLESTONE_BLOCK].m_bottomFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(0, 2));
	s_BlockDefinitions[COBBLESTONE_BLOCK].m_sideFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(0, 2));
	s_BlockDefinitions[COBBLESTONE_BLOCK].m_Breaking_Sound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/CobblestonePlacingAndBreaking.ogg");
	s_BlockDefinitions[COBBLESTONE_BLOCK].m_Placing_Sound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/CobblestonePlacingAndBreaking.ogg");
	s_BlockDefinitions[COBBLESTONE_BLOCK].m_Walking_Sound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/CobblestoneWalking.ogg");

	s_BlockDefinitions[CLAY_BRICK_BLOCK].m_illuminationValue = 0;
	s_BlockDefinitions[CLAY_BRICK_BLOCK].m_Toughness = 4.0f;
	s_BlockDefinitions[CLAY_BRICK_BLOCK].m_IsSolid = true;
	s_BlockDefinitions[CLAY_BRICK_BLOCK].m_IsOpaque = true;
	s_BlockDefinitions[CLAY_BRICK_BLOCK].m_IsVisible = true;
	s_BlockDefinitions[CLAY_BRICK_BLOCK].m_topFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(1, 2));
	s_BlockDefinitions[CLAY_BRICK_BLOCK].m_bottomFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(1, 2));
	s_BlockDefinitions[CLAY_BRICK_BLOCK].m_sideFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(1, 2));
	s_BlockDefinitions[CLAY_BRICK_BLOCK].m_Breaking_Sound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/ClayBrickPlacingAndBreaking.ogg");
	s_BlockDefinitions[CLAY_BRICK_BLOCK].m_Placing_Sound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/ClayBrickPlacingAndBreaking.ogg");

	s_BlockDefinitions[STONE_BRICK_BLOCK].m_illuminationValue = 0;
	s_BlockDefinitions[STONE_BRICK_BLOCK].m_Toughness = 4.0f;
	s_BlockDefinitions[STONE_BRICK_BLOCK].m_IsSolid = true;
	s_BlockDefinitions[STONE_BRICK_BLOCK].m_IsOpaque = true;
	s_BlockDefinitions[STONE_BRICK_BLOCK].m_IsVisible = true;
	s_BlockDefinitions[STONE_BRICK_BLOCK].m_topFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(2, 2));
	s_BlockDefinitions[STONE_BRICK_BLOCK].m_bottomFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(2, 2));
	s_BlockDefinitions[STONE_BRICK_BLOCK].m_sideFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(2, 2));
	s_BlockDefinitions[STONE_BRICK_BLOCK].m_Breaking_Sound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/StoneBrickPlacingAndBreaking.ogg");
	s_BlockDefinitions[STONE_BRICK_BLOCK].m_Placing_Sound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/StoneBrickPlacingAndBreaking.ogg");

	s_BlockDefinitions[WATER_BLOCK].m_illuminationValue = 0;
	s_BlockDefinitions[WATER_BLOCK].m_Toughness = 0.0f;
	s_BlockDefinitions[WATER_BLOCK].m_IsSolid = false;
	s_BlockDefinitions[WATER_BLOCK].m_IsOpaque = false;
	s_BlockDefinitions[WATER_BLOCK].m_IsVisible = true;
	s_BlockDefinitions[WATER_BLOCK].m_topFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(0, 0));
	s_BlockDefinitions[WATER_BLOCK].m_bottomFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(0, 0));
	s_BlockDefinitions[WATER_BLOCK].m_sideFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(0, 0));
	s_BlockDefinitions[WATER_BLOCK].m_Breaking_Sound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/WaterBreaking.ogg");

	s_BlockDefinitions[BEDROCK_BLOCK].m_illuminationValue = 0;
	s_BlockDefinitions[BEDROCK_BLOCK].m_Toughness = 10.0f;
	s_BlockDefinitions[BEDROCK_BLOCK].m_IsSolid = true;
	s_BlockDefinitions[BEDROCK_BLOCK].m_IsOpaque = true;
	s_BlockDefinitions[BEDROCK_BLOCK].m_IsVisible = true;
	s_BlockDefinitions[BEDROCK_BLOCK].m_topFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(3, 2));
	s_BlockDefinitions[BEDROCK_BLOCK].m_bottomFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(3, 2));
	s_BlockDefinitions[BEDROCK_BLOCK].m_sideFaceTextureAABB = blockSpriteSheet.GetTextureCoordsForSpriteCoords(IntVector2(3, 2));
	s_BlockDefinitions[BEDROCK_BLOCK].m_Breaking_Sound = AudioSystem::SingletonInstance()->CreateOrGetSound("Data/Sounds/BedrockBreaking.ogg");
}



BlockDefinition BlockDefinition::s_BlockDefinitions[NUMBER_OF_BLOCKS_TYPES];



Block::Block()
{

}



Block::Block(uint8_t blockType) : m_lightAndFlags(0)
{
	m_BlockType = blockType;
}



int Block::GetInternalIlluminationValue() const
{
	return BlockDefinition::s_BlockDefinitions[m_BlockType].m_illuminationValue;
}



void Block::SetSkyBit()
{
	SetBits(m_lightAndFlags, SKY_FLAG_MASK);
}



void Block::ClearSkyBit()
{
	ClearBits(m_lightAndFlags, SKY_FLAG_MASK);
}



void Block::SetLightingDirty()
{
	SetBits(m_lightAndFlags, LIGHTING_DIRTY_MASK);
}



void Block::ClearLightingDirty()
{
	ClearBits(m_lightAndFlags, LIGHTING_DIRTY_MASK);
}



bool Block::IsSky()
{
	return ((m_lightAndFlags & SKY_FLAG_MASK) == SKY_FLAG_MASK);
}



bool Block::LightingIsDirty()
{
	return ((m_lightAndFlags & LIGHTING_DIRTY_MASK) == LIGHTING_DIRTY_MASK);
}



void Block::SetLightValue(int lightValue)
{
	unsigned char requiredLightValue = static_cast<unsigned char>(lightValue);

	ClearBits(m_lightAndFlags, LIGHT_VALUE_MASK);
	SetBits(m_lightAndFlags, requiredLightValue);
}



int Block::GetLightValue()
{
	int lightValue = static_cast<int>(m_lightAndFlags & LIGHT_VALUE_MASK);

	return lightValue;
}