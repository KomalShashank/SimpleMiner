#pragma once

#include "Game/GameCommons.hpp"
#include "Game/Chunk.hpp"



class BlockInfo
{
public:
	BlockInfo();
	BlockInfo(Chunk* chunkOfBlock, int blockIndex);
	BlockInfo(const BlockInfo& blockInfo);

	bool operator==(const BlockInfo& blockInfo);

	Block* GetBlock();

	Chunk* GetChunk();
	int GetBlockIndex();

	BlockInfo GetEasternNeighbour();
	BlockInfo GetWesternNeighbour();

	BlockInfo GetNorthernNeighbour();
	BlockInfo GetSouthernNeighbour();
	
	BlockInfo GetAboveNeighbour();
	BlockInfo GetBelowNeighbour();

	bool IsEasternEdgeBlock();
	bool IsWesternEdgeBlock();

	bool IsNorthernEdgeBlock();
	bool IsSouthernEdgeBlock();

	bool IsEdgeBlock();

	bool IsTopMostBlock();
	bool IsBottomMostBlock();
	
private:
	Chunk* m_ChunkOfBlock;
	int m_BlockIndex;
};