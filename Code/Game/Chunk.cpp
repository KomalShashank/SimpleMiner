#include "Game/Chunk.hpp"
#include "Game/BlockInfo.hpp"



void ChunkProxy::CompressToRLEBuffer(uint8_t* rleBuffer, size_t& bufferSize)
{
	uint8_t currentBlockType = m_BlockTypes[0];
	uint8_t currentBlockTypeCount = 0;

	for (int blockIndex = 0; blockIndex < NUMBER_OF_BLOCKS_PER_CHUNK; ++blockIndex)
	{
		if (m_BlockTypes[blockIndex] == currentBlockType && currentBlockTypeCount < 255)
		{
			++currentBlockTypeCount;
		}
		else
		{
			rleBuffer[bufferSize] = currentBlockType;
			++bufferSize;
			rleBuffer[bufferSize] = currentBlockTypeCount;
			++bufferSize;

			currentBlockType = m_BlockTypes[blockIndex];
			currentBlockTypeCount = 1;
		}
	}
	if (currentBlockTypeCount > 0)
	{
		rleBuffer[bufferSize] = currentBlockType;
		++bufferSize;
		rleBuffer[bufferSize] = currentBlockTypeCount;
		++bufferSize;
	}
}



void ChunkProxy::DecompressFromRLEBuffer(const uint8_t* rleBuffer, size_t bufferSize)
{
	int blockIndex = 0;

	for (size_t byteIndex = 0; byteIndex < bufferSize; byteIndex += 2U)
	{
		uint8_t currentBlockType = rleBuffer[byteIndex];
		int currentBlockTypeCount = (int)rleBuffer[byteIndex + 1];

		for (int blockCount = 0; blockCount < currentBlockTypeCount; ++blockCount)
		{
			m_BlockTypes[blockIndex] = currentBlockType;
			++blockIndex;
		}
	}
}



void ChunkProxy::PopulateFromPerlinNoise()
{
	int perlinHeights[NUMBER_OF_BLOCKS_PER_LAYER];

	for (int blockIndexZ = 0; blockIndexZ < NUMBER_OF_BLOCKS_Z; ++blockIndexZ)
	{
		for (int blockIndexY = 0; blockIndexY < NUMBER_OF_BLOCKS_Y; ++blockIndexY)
		{
			for (int blockIndexX = 0; blockIndexX < NUMBER_OF_BLOCKS_X; ++blockIndexX)
			{
				IntVector3 localCoordinates = IntVector3(blockIndexX, blockIndexY, blockIndexZ);
				int blockIndexXY = (localCoordinates.Y << BLOCK_BITS_X) + localCoordinates.X;

				if (blockIndexZ == 0)
				{
					IntVector3 worldCoordinates;
					worldCoordinates.X = localCoordinates.X + (m_ChunkCoordinates.X << BLOCK_BITS_X);
					worldCoordinates.Y = localCoordinates.Y + (m_ChunkCoordinates.Y << BLOCK_BITS_Y);
					worldCoordinates.Z = localCoordinates.Z;

					int heightValue = GenerateHeightValueForBlock(worldCoordinates.X, worldCoordinates.Y);
					perlinHeights[blockIndexXY] = heightValue;
				}

				int localIndex = (localCoordinates.Z << BLOCK_BITS_PER_LAYER) + (localCoordinates.Y << BLOCK_BITS_X) + localCoordinates.X;

				if (blockIndexZ == perlinHeights[blockIndexXY])
				{
					m_BlockTypes[localIndex] = GRASS_BLOCK;
				}
				else if (blockIndexZ < perlinHeights[blockIndexXY] && blockIndexZ >= perlinHeights[blockIndexXY] - 4)
				{
					m_BlockTypes[localIndex] = DIRT_BLOCK;
				}
				else if (blockIndexZ < perlinHeights[blockIndexXY] - 4 && blockIndexZ > 0)
				{
					m_BlockTypes[localIndex] = STONE_BLOCK;
				}
				else if (blockIndexZ == 0)
				{
					m_BlockTypes[localIndex] = BEDROCK_BLOCK;
				}
			}
		}
	}

	for (int blockIndex = 0; blockIndex < NUMBER_OF_BLOCKS_PER_CHUNK; ++blockIndex)
	{
		uint8_t& currentBlockType = m_BlockTypes[blockIndex];
		int layerIndex = blockIndex >> BLOCK_BITS_PER_LAYER;

		if (layerIndex <= NUMBER_OF_BLOCKS_Z / 2)
		{
			if (currentBlockType == GRASS_BLOCK || currentBlockType == DIRT_BLOCK)
			{
				currentBlockType = SAND_BLOCK;
			}
		}

		if (layerIndex < NUMBER_OF_BLOCKS_Z / 2)
		{
			if (currentBlockType == AIR_BLOCK)
			{
				currentBlockType = WATER_BLOCK;
			}
		}
	}
}



int ChunkProxy::GenerateHeightValueForBlock(int blockX, int blockY)
{
	float horizontalX = static_cast<float>(blockX);
	float horizontalY = static_cast<float>(blockY);

	float columnNoiseValue = ComputePerlinNoiseIn2D(Vector2(horizontalX, horizontalY), 125.0f, 4U);
	float heightOffset = RangeMap(columnNoiseValue, -1.0f, 1.0f, -30.0f, 30.0f);

	int heightValue = GROUND_HEIGHT + static_cast<int>(heightOffset);

	return heightValue;
}



Chunk::Chunk() :
m_ChunkCoordinates(IntVector2::ZERO),
m_ChunkWorldMinimums(Vector3::ZERO),
m_IsModified(false),
m_ChunkMesh(new Mesh()),
m_WaterMesh(new Mesh()),
m_EasternNeighbour(nullptr),
m_WesternNeighbour(nullptr),
m_NorthernNeighbour(nullptr),
m_SouthernNeighbour(nullptr)
{
	for (int localIndex = 0; localIndex < NUMBER_OF_BLOCKS_PER_CHUNK; ++localIndex)
	{
		m_Blocks[localIndex] = Block(AIR_BLOCK);
	}
}



Chunk::~Chunk()
{
	delete m_WaterMesh;
	delete m_ChunkMesh;
}



void Chunk::InitializeChunk(const IntVector2& chunkCoordinates)
{
	m_ChunkCoordinates = chunkCoordinates;
	m_ChunkWorldMinimums = GetChunkWorldMinimumsForChunkCoordinates(chunkCoordinates);
}



ChunkProxy Chunk::GetChunkProxyFromChunk()
{
	ChunkProxy chunkProxy;
	chunkProxy.m_ChunkCoordinates = m_ChunkCoordinates;
	chunkProxy.m_IsValid = true;

	for (int blockIndex = 0; blockIndex < NUMBER_OF_BLOCKS_PER_CHUNK; ++blockIndex)
	{
		chunkProxy.m_BlockTypes[blockIndex] = m_Blocks[blockIndex].GetType();
	}

	return chunkProxy;
}



void Chunk::PopulateChunkFromChunkProxy(const ChunkProxy& chunkProxy)
{
	for (int blockIndex = 0; blockIndex < NUMBER_OF_BLOCKS_PER_CHUNK; ++blockIndex)
	{
		m_Blocks[blockIndex].SetType(chunkProxy.m_BlockTypes[blockIndex]);
	}

	m_IsModified = true;
}



void Chunk::RebuildChunkMesh()
{
	std::vector<Vertex3D> chunkVertices;
	std::vector<uint32_t> chunkIndices;
	
	Vertex3D chunkVertex;
	chunkVertex.m_Color = RGBA::WHITE;

	Vector3 eastDirection = Vector3::X_AXIS;
	Vector3 westDirection = eastDirection.GetNegatedVector3();

	Vector3 northDirection = Vector3::Y_AXIS;
	Vector3 southDirection = northDirection.GetNegatedVector3();

	Vector3 upDirection = Vector3::Z_AXIS;
	Vector3 downDirection = upDirection.GetNegatedVector3();

	for (int blockIndex = 0; blockIndex < NUMBER_OF_BLOCKS_PER_CHUNK; ++blockIndex)
	{
		Block currentBlock = m_Blocks[blockIndex];

		if (currentBlock.IsVisible() && currentBlock.IsOpaque())
		{
			IntVector3 blockLocalCoordinates = GetLocalCoordinatesForLocalIndex(blockIndex);
			IntVector3 blockWorldCoordinates = GetBlockWorldCoordinatesForLocalCoordinates(blockLocalCoordinates);

			Vector3 blockMinimums = GetBlockMinimumsForBlockCoordinates(blockWorldCoordinates);
			Vector3 blockMaximums = blockMinimums + Vector3::ONE;

			if (EasternNeighbourDoesNotExist(blockIndex))
			{
				size_t previousIndex = chunkVertices.size();
				chunkIndices.push_back(previousIndex + 0);
				chunkIndices.push_back(previousIndex + 1);
				chunkIndices.push_back(previousIndex + 2);
				chunkIndices.push_back(previousIndex + 0);
				chunkIndices.push_back(previousIndex + 2);
				chunkIndices.push_back(previousIndex + 3);
				
				int lightValue = GetLightValueForEasternNeighbour(blockIndex);
				float light = static_cast<float>(lightValue);
				float lightFraction = RangeMap(light, 0.0f, 15.0f, 50.0f, 255.0f);
				unsigned char newLight = static_cast<unsigned char>(lightFraction);
				chunkVertex.m_Color = RGBA(newLight, newLight, newLight, 255);

				chunkVertex.m_Tangent = northDirection;
				chunkVertex.m_Bitangent = upDirection;
				chunkVertex.m_Normal = eastDirection;

				AABB2 eastFace = currentBlock.GetSideFaceTextureAABB();
				Vector2 eastFaceMin = eastFace.minimums;
				Vector2 eastFaceMax = eastFace.maximums;

				chunkVertex.m_Position = Vector3(blockMaximums.X, blockMaximums.Y, blockMinimums.Z);
				chunkVertex.m_TextureCoordinates = Vector2(eastFaceMax.X, eastFaceMax.Y);
				chunkVertices.push_back(chunkVertex);

				chunkVertex.m_Position = Vector3(blockMaximums.X, blockMaximums.Y, blockMaximums.Z);
				chunkVertex.m_TextureCoordinates = Vector2(eastFaceMax.X, eastFaceMin.Y);
				chunkVertices.push_back(chunkVertex);

				chunkVertex.m_Position = Vector3(blockMaximums.X, blockMinimums.Y, blockMaximums.Z);
				chunkVertex.m_TextureCoordinates = Vector2(eastFaceMin.X, eastFaceMin.Y);
				chunkVertices.push_back(chunkVertex);

				chunkVertex.m_Position = Vector3(blockMaximums.X, blockMinimums.Y, blockMinimums.Z);
				chunkVertex.m_TextureCoordinates = Vector2(eastFaceMin.X, eastFaceMax.Y);
				chunkVertices.push_back(chunkVertex);
			}

			if (WesternNeighbourDoesNotExist(blockIndex))
			{
				size_t previousIndex = chunkVertices.size();
				chunkIndices.push_back(previousIndex + 0);
				chunkIndices.push_back(previousIndex + 1);
				chunkIndices.push_back(previousIndex + 2);
				chunkIndices.push_back(previousIndex + 0);
				chunkIndices.push_back(previousIndex + 2);
				chunkIndices.push_back(previousIndex + 3);

				int lightValue = GetLightValueForWesternNeighbour(blockIndex);
				float light = static_cast<float>(lightValue);
				float lightFraction = RangeMap(light, 0.0f, 15.0f, 50.0f, 255.0f);
				unsigned char newLight = static_cast<unsigned char>(lightFraction);
				chunkVertex.m_Color = RGBA(newLight, newLight, newLight, 255);

				chunkVertex.m_Tangent = southDirection;
				chunkVertex.m_Bitangent = upDirection;
				chunkVertex.m_Normal = westDirection;

				AABB2 westFace = currentBlock.GetSideFaceTextureAABB();
				Vector2 westFaceMin = westFace.minimums;
				Vector2 westFaceMax = westFace.maximums;

				chunkVertex.m_Position = Vector3(blockMinimums.X, blockMaximums.Y, blockMinimums.Z);
				chunkVertex.m_TextureCoordinates = Vector2(westFaceMin.X, westFaceMax.Y);
				chunkVertices.push_back(chunkVertex);

				chunkVertex.m_Position = Vector3(blockMinimums.X, blockMinimums.Y, blockMinimums.Z);
				chunkVertex.m_TextureCoordinates = Vector2(westFaceMax.X, westFaceMax.Y);
				chunkVertices.push_back(chunkVertex);

				chunkVertex.m_Position = Vector3(blockMinimums.X, blockMinimums.Y, blockMaximums.Z);
				chunkVertex.m_TextureCoordinates = Vector2(westFaceMax.X, westFaceMin.Y);
				chunkVertices.push_back(chunkVertex);

				chunkVertex.m_Position = Vector3(blockMinimums.X, blockMaximums.Y, blockMaximums.Z);
				chunkVertex.m_TextureCoordinates = Vector2(westFaceMin.X, westFaceMin.Y);
				chunkVertices.push_back(chunkVertex);
			}

			if (NorthernNeighbourDoesNotExist(blockIndex))
			{
				size_t previousIndex = chunkVertices.size();
				chunkIndices.push_back(previousIndex + 0);
				chunkIndices.push_back(previousIndex + 1);
				chunkIndices.push_back(previousIndex + 2);
				chunkIndices.push_back(previousIndex + 0);
				chunkIndices.push_back(previousIndex + 2);
				chunkIndices.push_back(previousIndex + 3);

				int lightValue = GetLightValueForNorthernNeighbour(blockIndex);
				float light = static_cast<float>(lightValue);
				float lightFraction = RangeMap(light, 0.0f, 15.0f, 50.0f, 255.0f);
				unsigned char newLight = static_cast<unsigned char>(lightFraction);
				chunkVertex.m_Color = RGBA(newLight, newLight, newLight, 255);

				chunkVertex.m_Tangent = westDirection;
				chunkVertex.m_Bitangent = upDirection;
				chunkVertex.m_Normal = northDirection;

				AABB2 northFace = currentBlock.GetSideFaceTextureAABB();
				Vector2 northFaceMin = northFace.minimums;
				Vector2 northFaceMax = northFace.maximums;

				chunkVertex.m_Position = Vector3(blockMaximums.X, blockMaximums.Y, blockMinimums.Z);
				chunkVertex.m_TextureCoordinates = Vector2(northFaceMin.X, northFaceMax.Y);
				chunkVertices.push_back(chunkVertex);

				chunkVertex.m_Position = Vector3(blockMinimums.X, blockMaximums.Y, blockMinimums.Z);
				chunkVertex.m_TextureCoordinates = Vector2(northFaceMax.X, northFaceMax.Y);
				chunkVertices.push_back(chunkVertex);

				chunkVertex.m_Position = Vector3(blockMinimums.X, blockMaximums.Y, blockMaximums.Z);
				chunkVertex.m_TextureCoordinates = Vector2(northFaceMax.X, northFaceMin.Y);
				chunkVertices.push_back(chunkVertex);

				chunkVertex.m_Position = Vector3(blockMaximums.X, blockMaximums.Y, blockMaximums.Z);
				chunkVertex.m_TextureCoordinates = Vector2(northFaceMin.X, northFaceMin.Y);
				chunkVertices.push_back(chunkVertex);
			}

			if (SouthernNeighbourDoesNotExist(blockIndex))
			{
				size_t previousIndex = chunkVertices.size();
				chunkIndices.push_back(previousIndex + 0);
				chunkIndices.push_back(previousIndex + 1);
				chunkIndices.push_back(previousIndex + 2);
				chunkIndices.push_back(previousIndex + 0);
				chunkIndices.push_back(previousIndex + 2);
				chunkIndices.push_back(previousIndex + 3);

				int lightValue = GetLightValueForSouthernNeighbour(blockIndex);
				float light = static_cast<float>(lightValue);
				float lightFraction = RangeMap(light, 0.0f, 15.0f, 50.0f, 255.0f);
				unsigned char newLight = static_cast<unsigned char>(lightFraction);
				chunkVertex.m_Color = RGBA(newLight, newLight, newLight, 255);

				chunkVertex.m_Tangent = eastDirection;
				chunkVertex.m_Bitangent = upDirection;
				chunkVertex.m_Normal = southDirection;

				AABB2 southFace = currentBlock.GetSideFaceTextureAABB();
				Vector2 southFaceMin = southFace.minimums;
				Vector2 southFaceMax = southFace.maximums;

				chunkVertex.m_Position = Vector3(blockMaximums.X, blockMinimums.Y, blockMinimums.Z);
				chunkVertex.m_TextureCoordinates = Vector2(southFaceMax.X, southFaceMax.Y);
				chunkVertices.push_back(chunkVertex);

				chunkVertex.m_Position = Vector3(blockMaximums.X, blockMinimums.Y, blockMaximums.Z);
				chunkVertex.m_TextureCoordinates = Vector2(southFaceMax.X, southFaceMin.Y);
				chunkVertices.push_back(chunkVertex);

				chunkVertex.m_Position = Vector3(blockMinimums.X, blockMinimums.Y, blockMaximums.Z);
				chunkVertex.m_TextureCoordinates = Vector2(southFaceMin.X, southFaceMin.Y);
				chunkVertices.push_back(chunkVertex);

				chunkVertex.m_Position = Vector3(blockMinimums.X, blockMinimums.Y, blockMinimums.Z);
				chunkVertex.m_TextureCoordinates = Vector2(southFaceMin.X, southFaceMax.Y);
				chunkVertices.push_back(chunkVertex);
			}

			if (AboveNeighbourDoesNotExist(blockIndex))
			{
				size_t previousIndex = chunkVertices.size();
				chunkIndices.push_back(previousIndex + 0);
				chunkIndices.push_back(previousIndex + 1);
				chunkIndices.push_back(previousIndex + 2);
				chunkIndices.push_back(previousIndex + 0);
				chunkIndices.push_back(previousIndex + 2);
				chunkIndices.push_back(previousIndex + 3);

				int lightValue = GetLightValueForAboveNeighbour(blockIndex);
				float light = static_cast<float>(lightValue);
				float lightFraction = RangeMap(light, 0.0f, 15.0f, 50.0f, 255.0f);
				unsigned char newLight = static_cast<unsigned char>(lightFraction);
				chunkVertex.m_Color = RGBA(newLight, newLight, newLight, 255);

				chunkVertex.m_Tangent = eastDirection;
				chunkVertex.m_Bitangent = northDirection;
				chunkVertex.m_Normal = upDirection;

				AABB2 topFace = currentBlock.GetTopFaceTextureAABB();
				Vector2 topFaceMin = topFace.minimums;
				Vector2 topFaceMax = topFace.maximums;

				chunkVertex.m_Position = Vector3(blockMaximums.X, blockMinimums.Y, blockMaximums.Z);
				chunkVertex.m_TextureCoordinates = Vector2(topFaceMax.X, topFaceMax.Y);
				chunkVertices.push_back(chunkVertex);

				chunkVertex.m_Position = Vector3(blockMaximums.X, blockMaximums.Y, blockMaximums.Z);
				chunkVertex.m_TextureCoordinates = Vector2(topFaceMax.X, topFaceMin.Y);
				chunkVertices.push_back(chunkVertex);

				chunkVertex.m_Position = Vector3(blockMinimums.X, blockMaximums.Y, blockMaximums.Z);
				chunkVertex.m_TextureCoordinates = Vector2(topFaceMin.X, topFaceMin.Y);
				chunkVertices.push_back(chunkVertex);

				chunkVertex.m_Position = Vector3(blockMinimums.X, blockMinimums.Y, blockMaximums.Z);
				chunkVertex.m_TextureCoordinates = Vector2(topFaceMin.X, topFaceMax.Y);
				chunkVertices.push_back(chunkVertex);
			}

			if (BelowNeighbourDoesNotExist(blockIndex))
			{
				size_t previousIndex = chunkVertices.size();
				chunkIndices.push_back(previousIndex + 0);
				chunkIndices.push_back(previousIndex + 1);
				chunkIndices.push_back(previousIndex + 2);
				chunkIndices.push_back(previousIndex + 0);
				chunkIndices.push_back(previousIndex + 2);
				chunkIndices.push_back(previousIndex + 3);

				int lightValue = GetLightValueForBelowNeighbour(blockIndex);
				float light = static_cast<float>(lightValue);
				float lightFraction = RangeMap(light, 0.0f, 15.0f, 50.0f, 255.0f);
				unsigned char newLight = static_cast<unsigned char>(lightFraction);
				chunkVertex.m_Color = RGBA(newLight, newLight, newLight, 255);

				chunkVertex.m_Tangent = westDirection;
				chunkVertex.m_Bitangent = northDirection;
				chunkVertex.m_Normal = downDirection;

				AABB2 bottomFace = currentBlock.GetBottomFaceTextureAABB();
				Vector2 bottomFaceMin = bottomFace.minimums;
				Vector2 bottomFaceMax = bottomFace.maximums;

				chunkVertex.m_Position = Vector3(blockMaximums.X, blockMinimums.Y, blockMinimums.Z);
				chunkVertex.m_TextureCoordinates = Vector2(bottomFaceMin.X, bottomFaceMax.Y);
				chunkVertices.push_back(chunkVertex);

				chunkVertex.m_Position = Vector3(blockMinimums.X, blockMinimums.Y, blockMinimums.Z);
				chunkVertex.m_TextureCoordinates = Vector2(bottomFaceMax.X, bottomFaceMax.Y);
				chunkVertices.push_back(chunkVertex);

				chunkVertex.m_Position = Vector3(blockMinimums.X, blockMaximums.Y, blockMinimums.Z);
				chunkVertex.m_TextureCoordinates = Vector2(bottomFaceMax.X, bottomFaceMin.Y);
				chunkVertices.push_back(chunkVertex);

				chunkVertex.m_Position = Vector3(blockMaximums.X, blockMaximums.Y, blockMinimums.Z);
				chunkVertex.m_TextureCoordinates = Vector2(bottomFaceMin.X, bottomFaceMin.Y);
				chunkVertices.push_back(chunkVertex);
			}
		}
	}

	m_ChunkMesh->WriteToMesh(chunkVertices.data(), chunkIndices.data(), chunkVertices.size(), chunkIndices.size());

	m_ChunkMesh->m_RenderInstructions.clear();
	m_ChunkMesh->AddRenderInstruction(chunkVertices.size(), chunkIndices.size(), TRIANGLES_PRIMITIVE);
}



void Chunk::RebuildWaterMesh()
{
	std::vector<Vertex3D> waterVertices;
	std::vector<uint32_t> waterIndices;

	Vertex3D waterVertex;
	waterVertex.m_Color = RGBA(0, 191, 255, 255);

	waterVertex.m_Tangent = Vector3::X_AXIS;
	waterVertex.m_Bitangent = Vector3::Y_AXIS;
	waterVertex.m_Normal = Vector3::Z_AXIS;

	const float OFFSET_HEIGHT = 0.25f * BLOCK_HEIGHT;
	for (int blockIndex = 0; blockIndex < NUMBER_OF_BLOCKS_PER_CHUNK; ++blockIndex)
	{
		Block currentBlock = m_Blocks[blockIndex];
		if (currentBlock.IsVisible() && !currentBlock.IsOpaque())
		{
			BlockInfo currentBlockInfo = BlockInfo(this, blockIndex);
			BlockInfo aboveBlockInfo = currentBlockInfo.GetAboveNeighbour();
			
			int aboveBlockIndex = aboveBlockInfo.GetBlockIndex();
			if (m_Blocks[aboveBlockIndex].GetType() == AIR_BLOCK)
			{
				size_t previousIndex = waterVertices.size();
				waterIndices.push_back(previousIndex + 0);
				waterIndices.push_back(previousIndex + 1);
				waterIndices.push_back(previousIndex + 2);
				waterIndices.push_back(previousIndex + 0);
				waterIndices.push_back(previousIndex + 2);
				waterIndices.push_back(previousIndex + 3);

				IntVector3 blockLocalCoordinates = GetLocalCoordinatesForLocalIndex(blockIndex);
				IntVector3 blockWorldCoordinates = GetBlockWorldCoordinatesForLocalCoordinates(blockLocalCoordinates);

				Vector3 blockMinimums = GetBlockMinimumsForBlockCoordinates(blockWorldCoordinates);
				Vector3 blockMaximums = blockMinimums + Vector3::ONE;

				AABB2 topFace = currentBlock.GetTopFaceTextureAABB();
				Vector2 topFaceMin = topFace.minimums;
				Vector2 topFaceMax = topFace.maximums;

				waterVertex.m_Position = Vector3(blockMaximums.X, blockMinimums.Y, blockMaximums.Z - OFFSET_HEIGHT);
				waterVertex.m_TextureCoordinates = Vector2(topFaceMax.X, topFaceMax.Y);
				waterVertices.push_back(waterVertex);

				waterVertex.m_Position = Vector3(blockMaximums.X, blockMaximums.Y, blockMaximums.Z - OFFSET_HEIGHT);
				waterVertex.m_TextureCoordinates = Vector2(topFaceMax.X, topFaceMin.Y);
				waterVertices.push_back(waterVertex);

				waterVertex.m_Position = Vector3(blockMinimums.X, blockMaximums.Y, blockMaximums.Z - OFFSET_HEIGHT);
				waterVertex.m_TextureCoordinates = Vector2(topFaceMin.X, topFaceMin.Y);
				waterVertices.push_back(waterVertex);

				waterVertex.m_Position = Vector3(blockMinimums.X, blockMinimums.Y, blockMaximums.Z - OFFSET_HEIGHT);
				waterVertex.m_TextureCoordinates = Vector2(topFaceMin.X, topFaceMax.Y);
				waterVertices.push_back(waterVertex);
			}
		}
	}

	m_WaterMesh->WriteToMesh(waterVertices.data(), waterIndices.data(), waterVertices.size(), waterIndices.size());

	m_WaterMesh->m_RenderInstructions.clear();
	m_WaterMesh->AddRenderInstruction(waterVertices.size(), waterIndices.size(), TRIANGLES_PRIMITIVE);
}



void Chunk::RenderChunk(Material* chunkMaterial) const
{
	AdvancedRenderer::SingletonInstance()->DrawMeshWithVAO(m_ChunkMesh, chunkMaterial);
}



void Chunk::RenderWater(Material* waterMaterial) const
{
	AdvancedRenderer::SingletonInstance()->DrawMeshWithVAO(m_WaterMesh, waterMaterial);
}



inline bool Chunk::EasternNeighbourDoesNotExist(int blockIndex)
{
	BlockInfo currentBlockInfo = BlockInfo(this, blockIndex);
	BlockInfo easternBlockInfo = currentBlockInfo.GetEasternNeighbour();

	Chunk* easternBlockChunk = easternBlockInfo.GetChunk();

	if (easternBlockChunk != nullptr)
	{
		int easternBlockIndex = easternBlockInfo.GetBlockIndex();

		if (easternBlockChunk->m_Blocks[easternBlockIndex].IsOpaque())
		{
			return false;
		}
	}
	else if (easternBlockChunk == nullptr)
	{
		return false;
	}

	return true;
}



inline bool Chunk::WesternNeighbourDoesNotExist(int blockIndex)
{
	BlockInfo currentBlockInfo = BlockInfo(this, blockIndex);
	BlockInfo westernBlockInfo = currentBlockInfo.GetWesternNeighbour();

	Chunk* westernBlockChunk = westernBlockInfo.GetChunk();

	if (westernBlockChunk != nullptr)
	{
		int westernBlockIndex = westernBlockInfo.GetBlockIndex();

		if (westernBlockChunk->m_Blocks[westernBlockIndex].IsOpaque())
		{
			return false;
		}
	}
	else if (westernBlockChunk == nullptr)
	{
		return false;
	}

	return true;
}



inline bool Chunk::NorthernNeighbourDoesNotExist(int blockIndex)
{
	BlockInfo currentBlockInfo = BlockInfo(this, blockIndex);
	BlockInfo northernBlockInfo = currentBlockInfo.GetNorthernNeighbour();

	Chunk* northernBlockChunk = northernBlockInfo.GetChunk();

	if (northernBlockChunk != nullptr)
	{
		int northernBlockIndex = northernBlockInfo.GetBlockIndex();

		if (northernBlockChunk->m_Blocks[northernBlockIndex].IsOpaque())
		{
			return false;
		}
	}
	else if (northernBlockChunk == nullptr)
	{
		return false;
	}

	return true;
}



inline bool Chunk::SouthernNeighbourDoesNotExist(int blockIndex)
{
	BlockInfo currentBlockInfo = BlockInfo(this, blockIndex);
	BlockInfo southernBlockInfo = currentBlockInfo.GetSouthernNeighbour();

	Chunk* southernBlockChunk = southernBlockInfo.GetChunk();

	if (southernBlockChunk != nullptr)
	{
		int southernBlockIndex = southernBlockInfo.GetBlockIndex();

		if (southernBlockChunk->m_Blocks[southernBlockIndex].IsOpaque())
		{
			return false;
		}
	}
	else if (southernBlockChunk == nullptr)
	{
		return false;
	}

	return true;
}



inline bool Chunk::AboveNeighbourDoesNotExist(int blockIndex)
{
	BlockInfo currentBlockInfo = BlockInfo(this, blockIndex);
	BlockInfo aboveBlockInfo = currentBlockInfo.GetAboveNeighbour();

	Chunk* aboveBlockChunk = aboveBlockInfo.GetChunk();

	if (aboveBlockChunk != nullptr)
	{
		int aboveBlockIndex = aboveBlockInfo.GetBlockIndex();

		if (aboveBlockChunk->m_Blocks[aboveBlockIndex].IsOpaque())
		{
			return false;
		}
	}
	else if (aboveBlockChunk == nullptr)
	{
		return false;
	}

	return true;
}



inline bool Chunk::BelowNeighbourDoesNotExist(int blockIndex)
{
	BlockInfo currentBlockInfo = BlockInfo(this, blockIndex);
	BlockInfo belowBlockInfo = currentBlockInfo.GetBelowNeighbour();

	Chunk* belowBlockChunk = belowBlockInfo.GetChunk();

	if (belowBlockChunk != nullptr)
	{
		int belowBlockIndex = belowBlockInfo.GetBlockIndex();

		if (belowBlockChunk->m_Blocks[belowBlockIndex].IsOpaque())
		{
			return false;
		}
	}
	else if (belowBlockChunk == nullptr)
	{
		return false;
	}

	return true;
}

int Chunk::GetLightValueForEasternNeighbour(int blockIndex)
{
	BlockInfo currentBlockInfo = BlockInfo(this, blockIndex);
	BlockInfo easternBlockInfo = currentBlockInfo.GetEasternNeighbour();
	Block* easternBlock = easternBlockInfo.GetBlock();

	return easternBlock->GetLightValue();
}



int Chunk::GetLightValueForWesternNeighbour(int blockIndex)
{
	BlockInfo currentBlockInfo = BlockInfo(this, blockIndex);
	BlockInfo westernBlockInfo = currentBlockInfo.GetWesternNeighbour();
	Block* westernBlock = westernBlockInfo.GetBlock();

	return westernBlock->GetLightValue();
}



int Chunk::GetLightValueForNorthernNeighbour(int blockIndex)
{
	BlockInfo currentBlockInfo = BlockInfo(this, blockIndex);
	BlockInfo northernBlockInfo = currentBlockInfo.GetNorthernNeighbour();
	Block* northernBlock = northernBlockInfo.GetBlock();

	return northernBlock->GetLightValue();
}



int Chunk::GetLightValueForSouthernNeighbour(int blockIndex)
{
	BlockInfo currentBlockInfo = BlockInfo(this, blockIndex);
	BlockInfo southernBlockInfo = currentBlockInfo.GetSouthernNeighbour();
	Block* southernBlock = southernBlockInfo.GetBlock();

	return southernBlock->GetLightValue();
}



int Chunk::GetLightValueForAboveNeighbour(int blockIndex)
{
	BlockInfo currentBlockInfo = BlockInfo(this, blockIndex);
	BlockInfo aboveBlockInfo = currentBlockInfo.GetAboveNeighbour();
	Block* aboveBlock = aboveBlockInfo.GetBlock();

	return aboveBlock->GetLightValue();
}



int Chunk::GetLightValueForBelowNeighbour(int blockIndex)
{
	BlockInfo currentBlockInfo = BlockInfo(this, blockIndex);
	BlockInfo belowBlockInfo = currentBlockInfo.GetBelowNeighbour();
	Block* belowBlock = belowBlockInfo.GetBlock();

	return belowBlock->GetLightValue();
}