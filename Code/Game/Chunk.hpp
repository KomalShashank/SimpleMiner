#pragma once

#include "Game/GameCommons.hpp"
#include "Game/Block.hpp"



const int GROUND_HEIGHT = 70;



class ChunkProxy
{
public:
	ChunkProxy() :
	m_IsValid(false)
	{
		
	}

	void CompressToRLEBuffer(uint8_t* rleBuffer, size_t& bufferSize);
	void DecompressFromRLEBuffer(const uint8_t* rleBuffer, size_t bufferSize);

	void PopulateFromPerlinNoise();

private:
	int GenerateHeightValueForBlock(int blockX, int blockY);

public:
	IntVector2 m_ChunkCoordinates;

	uint8_t m_BlockTypes[NUMBER_OF_BLOCKS_PER_CHUNK];
	bool m_IsValid;
};



class Chunk
{
public:
	Chunk();
	~Chunk();

	void InitializeChunk(const IntVector2& chunkCoordinates);

	ChunkProxy GetChunkProxyFromChunk();
	void PopulateChunkFromChunkProxy(const ChunkProxy& chunkProxy);
	
	void RebuildChunkMesh();
	void RebuildWaterMesh();

	void RenderChunk(Material* chunkMaterial) const;
	void RenderWater(Material* waterMaterial) const;

	bool IsModified();
	void SetModified(bool modified);

	IntVector2 GetChunkCoordinates();
	Vector3 GetChunkWorldMinimums();
	int GetLocalIndexForBlockWorldCoordinates(const Vector3& blockWorldCoordinates) const;

	IntVector2 GetLocalIndexRangeForLayerIndex(int layerIndex) const;
	int GetLayerIndexforLocalIndex(int localIndex) const;
	int GetLayerIndexforLocalCoordinates(const IntVector3& localCoordinates) const;

	IntVector3 GetLocalCoordinatesForLocalIndex(int localIndex) const;
	int GetLocalIndexForLocalCoordinates(const IntVector3& localCoordinates) const;

	IntVector3 GetBlockWorldCoordinatesForLocalCoordinates(const IntVector3& localCoordinates) const;
	IntVector3 GetBlockWorldCoordinatesForLocalIndex(int localIndex) const;
	Vector3 GetBlockMinimumsForBlockCoordinates(const IntVector3& blockCoordinates) const;

	Vector3 GetChunkWorldMinimumsForChunkCoordinates(const IntVector2& chunkCoordinates) const;
	
	bool EasternNeighbourDoesNotExist(int blockIndex);
	bool WesternNeighbourDoesNotExist(int blockIndex);

	bool NorthernNeighbourDoesNotExist(int blockIndex);
	bool SouthernNeighbourDoesNotExist(int blockIndex);

	bool AboveNeighbourDoesNotExist(int blockIndex);
	bool BelowNeighbourDoesNotExist(int blockIndex);

	int GetLightValueForEasternNeighbour(int blockIndex);
	int GetLightValueForWesternNeighbour(int blockIndex);

	int GetLightValueForNorthernNeighbour(int blockIndex);
	int GetLightValueForSouthernNeighbour(int blockIndex);

	int GetLightValueForAboveNeighbour(int blockIndex);
	int GetLightValueForBelowNeighbour(int blockIndex);

public:
	Chunk* m_NorthernNeighbour;
	Chunk* m_SouthernNeighbour;
	Chunk* m_EasternNeighbour;
	Chunk* m_WesternNeighbour;

	Block m_Blocks[NUMBER_OF_BLOCKS_PER_CHUNK];

private:
	bool m_IsModified;
	Mesh* m_ChunkMesh;
	Mesh* m_WaterMesh;

	IntVector2 m_ChunkCoordinates;
	Vector3 m_ChunkWorldMinimums;
};



inline bool Chunk::IsModified()
{
	return m_IsModified;
}



inline void Chunk::SetModified(bool modified)
{
	m_IsModified = modified;
}



inline IntVector2 Chunk::GetChunkCoordinates()
{
	return m_ChunkCoordinates;
}



inline Vector3 Chunk::GetChunkWorldMinimums()
{
	return m_ChunkWorldMinimums;
}



inline int Chunk::GetLocalIndexForBlockWorldCoordinates(const Vector3& blockWorldCoordinates) const
{
	Vector3 blockRelativePosition = blockWorldCoordinates - m_ChunkWorldMinimums;

	int blockLocalCoordinatesX = static_cast<int>(blockRelativePosition.X);
	int blockLocalCoordinatesY = static_cast<int>(blockRelativePosition.Y);
	int blockLocalCoordinatesZ = static_cast<int>(blockRelativePosition.Z);
	IntVector3 localCoordinates = IntVector3(blockLocalCoordinatesX, blockLocalCoordinatesY, blockLocalCoordinatesZ);

	int blockIndex = GetLocalIndexForLocalCoordinates(localCoordinates);

	return blockIndex;
}



inline IntVector2 Chunk::GetLocalIndexRangeForLayerIndex(int layerIndex) const
{
	int startingBlockIndex = layerIndex << BLOCK_BITS_PER_LAYER;
	int endingBlockIndex = startingBlockIndex + NUMBER_OF_BLOCKS_PER_LAYER - 1;

	return IntVector2(startingBlockIndex, endingBlockIndex);
}



inline int Chunk::GetLayerIndexforLocalIndex(int localIndex) const
{
	int layerIndex = localIndex >> BLOCK_BITS_PER_LAYER;

	return layerIndex;
}



inline int Chunk::GetLayerIndexforLocalCoordinates(const IntVector3& localCoordinates) const
{
	int localIndex = GetLocalIndexForLocalCoordinates(localCoordinates);
	int layerIndex = GetLayerIndexforLocalIndex(localIndex);

	return layerIndex;
}



inline IntVector3 Chunk::GetLocalCoordinatesForLocalIndex(int localIndex) const
{
	IntVector3 localCoordinates;
	localCoordinates.X = localIndex & BLOCKS_X_MASK;
	localCoordinates.Y = (localIndex >> BLOCK_BITS_X) & BLOCKS_Y_MASK;
	localCoordinates.Z = localIndex >> BLOCK_BITS_PER_LAYER;

	return localCoordinates;
}



inline int Chunk::GetLocalIndexForLocalCoordinates(const IntVector3& localCoordinates) const
{
	int localIndex = (localCoordinates.Z << BLOCK_BITS_PER_LAYER) + (localCoordinates.Y << BLOCK_BITS_X) + localCoordinates.X;

	return localIndex;
}



inline IntVector3 Chunk::GetBlockWorldCoordinatesForLocalCoordinates(const IntVector3& localCoordinates) const
{
	IntVector3 blockWorldCoordinates;
	blockWorldCoordinates.X = localCoordinates.X + (m_ChunkCoordinates.X << BLOCK_BITS_X);
	blockWorldCoordinates.Y = localCoordinates.Y + (m_ChunkCoordinates.Y << BLOCK_BITS_Y);
	blockWorldCoordinates.Z = localCoordinates.Z;

	return blockWorldCoordinates;
}



inline IntVector3 Chunk::GetBlockWorldCoordinatesForLocalIndex(int localIndex) const
{
	IntVector3 blockLocalCoordinates = GetLocalCoordinatesForLocalIndex(localIndex);
	IntVector3 blockWorldCoordinates = GetBlockWorldCoordinatesForLocalCoordinates(blockLocalCoordinates);

	return blockWorldCoordinates;
}



inline Vector3 Chunk::GetBlockMinimumsForBlockCoordinates(const IntVector3& blockCoordinates) const
{
	Vector3 blockMinimums;
	blockMinimums.X = static_cast<float>(blockCoordinates.X);
	blockMinimums.Y = static_cast<float>(blockCoordinates.Y);
	blockMinimums.Z = static_cast<float>(blockCoordinates.Z);

	return blockMinimums;
}



inline Vector3 Chunk::GetChunkWorldMinimumsForChunkCoordinates(const IntVector2& chunkCoordinates) const
{
	float worldMinimumsX = static_cast<float>(chunkCoordinates.X << BLOCK_BITS_X);
	float worldMinimumsY = static_cast<float>(chunkCoordinates.Y << BLOCK_BITS_Y);
	float worldMinimumsZ = 0.0f;

	return Vector3(worldMinimumsX, worldMinimumsY, worldMinimumsZ);
}