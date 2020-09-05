#include "Game/BlockInfo.hpp"



BlockInfo::BlockInfo() : m_ChunkOfBlock(nullptr), m_BlockIndex(0)
{

}



BlockInfo::BlockInfo(Chunk* chunkOfBlock, int blockIndex) : m_ChunkOfBlock(chunkOfBlock), m_BlockIndex(blockIndex)
{

}



BlockInfo::BlockInfo(const BlockInfo& blockInfo)
{
	m_ChunkOfBlock = blockInfo.m_ChunkOfBlock;
	m_BlockIndex = blockInfo.m_BlockIndex;
}



bool BlockInfo::operator==(const BlockInfo& blockInfo)
{
	return (m_ChunkOfBlock == blockInfo.m_ChunkOfBlock && m_BlockIndex == blockInfo.m_BlockIndex);
}



Block* BlockInfo::GetBlock()
{
	if (m_ChunkOfBlock != nullptr)
	{
		return &(m_ChunkOfBlock->m_Blocks[m_BlockIndex]);
	}

	return nullptr;
}



Chunk* BlockInfo::GetChunk()
{
	return m_ChunkOfBlock;
}



int BlockInfo::GetBlockIndex()
{
	return m_BlockIndex;
}



BlockInfo BlockInfo::GetEasternNeighbour()
{
	Chunk* chunkOfBlock;
	int blockIndex;

	if ((m_BlockIndex & BLOCKS_X_MASK) == BLOCKS_X_MASK)
	{
		chunkOfBlock = m_ChunkOfBlock->m_EasternNeighbour;

		if (chunkOfBlock != nullptr)
		{
			blockIndex = m_BlockIndex - BLOCKS_X_MASK;
		}
		else
		{
			blockIndex = 0;
		}
	}
	else
	{
		chunkOfBlock = m_ChunkOfBlock;
		blockIndex = m_BlockIndex + 1;
	}

	return BlockInfo(chunkOfBlock, blockIndex);
}



BlockInfo BlockInfo::GetWesternNeighbour()
{
	Chunk* chunkOfBlock;
	int blockIndex;

	if ((m_BlockIndex & BLOCKS_X_MASK) == 0)
	{
		chunkOfBlock = m_ChunkOfBlock->m_WesternNeighbour;

		if (chunkOfBlock != nullptr)
		{
			blockIndex = m_BlockIndex + BLOCKS_X_MASK;
		}
		else
		{
			blockIndex = 0;
		}
	}
	else
	{
		chunkOfBlock = m_ChunkOfBlock;
		blockIndex = m_BlockIndex - 1;
	}

	return BlockInfo(chunkOfBlock, blockIndex);
}



BlockInfo BlockInfo::GetNorthernNeighbour()
{
	Chunk* chunkOfBlock;
	int blockIndex;

	if (((m_BlockIndex >> BLOCK_BITS_X) & BLOCKS_Y_MASK) == BLOCKS_Y_MASK)
	{
		chunkOfBlock = m_ChunkOfBlock->m_NorthernNeighbour;
		
		if (chunkOfBlock != nullptr)
		{
			blockIndex = m_BlockIndex - (BLOCKS_Y_MASK << 4);
		}
		else
		{
			blockIndex = 0;
		}
	}
	else
	{
		chunkOfBlock = m_ChunkOfBlock;
		blockIndex = m_BlockIndex + NUMBER_OF_BLOCKS_X;
	}

	return BlockInfo(chunkOfBlock, blockIndex);
}



BlockInfo BlockInfo::GetSouthernNeighbour()
{
	Chunk* chunkOfBlock;
	int blockIndex;

	if (((m_BlockIndex >> BLOCK_BITS_X) & BLOCKS_Y_MASK) == 0)
	{
		chunkOfBlock = m_ChunkOfBlock->m_SouthernNeighbour;

		if (chunkOfBlock != nullptr)
		{
			blockIndex = m_BlockIndex + (BLOCKS_Y_MASK << 4);
		}
		else
		{
			blockIndex = 0;
		}
	}
	else
	{
		chunkOfBlock = m_ChunkOfBlock;
		blockIndex = m_BlockIndex - NUMBER_OF_BLOCKS_X;
	}

	return BlockInfo(chunkOfBlock, blockIndex);
}



BlockInfo BlockInfo::GetAboveNeighbour()
{
	Chunk* chunkOfBlock;
	int blockIndex;

	if (((m_BlockIndex >> BLOCK_BITS_PER_LAYER) & BLOCKS_Z_MASK) == BLOCKS_Z_MASK)
	{
		chunkOfBlock = nullptr;
		blockIndex = 0;
	}
	else
	{
		chunkOfBlock = m_ChunkOfBlock;
		blockIndex = m_BlockIndex + NUMBER_OF_BLOCKS_PER_LAYER;
	}

	return BlockInfo(chunkOfBlock, blockIndex);
}



BlockInfo BlockInfo::GetBelowNeighbour()
{
	Chunk* chunkOfBlock;
	int blockIndex;

	if (((m_BlockIndex >> BLOCK_BITS_PER_LAYER) & BLOCKS_Z_MASK) == 0)
	{
		chunkOfBlock = nullptr;
		blockIndex = 0;
	}
	else
	{
		chunkOfBlock = m_ChunkOfBlock;
		blockIndex = m_BlockIndex - NUMBER_OF_BLOCKS_PER_LAYER;
	}

	return BlockInfo(chunkOfBlock, blockIndex);
}



bool BlockInfo::IsEasternEdgeBlock()
{
	return ((m_BlockIndex & BLOCKS_X_MASK) == BLOCKS_X_MASK);
}



bool BlockInfo::IsWesternEdgeBlock()
{
	return ((m_BlockIndex & BLOCKS_X_MASK) == 0);
}



bool BlockInfo::IsNorthernEdgeBlock()
{
	return (((m_BlockIndex >> BLOCK_BITS_X) & BLOCKS_Y_MASK) == BLOCKS_Y_MASK);
}



bool BlockInfo::IsSouthernEdgeBlock()
{
	return (((m_BlockIndex >> BLOCK_BITS_X) & BLOCKS_Y_MASK) == 0);
}



bool BlockInfo::IsEdgeBlock()
{
	return (IsEasternEdgeBlock() || IsWesternEdgeBlock() || IsNorthernEdgeBlock() || IsSouthernEdgeBlock());
}



bool BlockInfo::IsTopMostBlock()
{
	return (((m_BlockIndex >> BLOCK_BITS_PER_LAYER) & BLOCKS_Z_MASK) == BLOCKS_Z_MASK);
}



bool BlockInfo::IsBottomMostBlock()
{
	return (((m_BlockIndex >> BLOCK_BITS_PER_LAYER) & BLOCKS_Z_MASK) == 0);
}