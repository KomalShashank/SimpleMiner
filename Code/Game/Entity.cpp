#include "Game/Entity.hpp"



Entity::Entity(const Vector3& entityPosition, float entityWidth, float entityLength, float entityHeight) :
m_Position(entityPosition),
m_Velocity(Vector3::ZERO),
m_Width(entityWidth),
m_Length(entityLength),
m_Height(entityHeight),
m_InitialPosition(entityPosition)
{
	BuildEntityIndices();
	DefineBoundingBox();
}



Entity::~Entity()
{
	delete m_EntityMesh;
}



void Entity::BuildEntityIndices()
{
	m_EntityIndices[0] = 0;
	m_EntityIndices[1] = 1;
	m_EntityIndices[2] = 1;
	m_EntityIndices[3] = 2;
	m_EntityIndices[4] = 2;
	m_EntityIndices[5] = 3;

	m_EntityIndices[6] = 3;
	m_EntityIndices[7] = 0;
	m_EntityIndices[8] = 4;
	m_EntityIndices[9] = 5;
	m_EntityIndices[10] = 5;
	m_EntityIndices[11] = 6;

	m_EntityIndices[12] = 6;
	m_EntityIndices[13] = 7;
	m_EntityIndices[14] = 7;
	m_EntityIndices[15] = 4;
	m_EntityIndices[16] = 0;
	m_EntityIndices[17] = 4;

	m_EntityIndices[18] = 1;
	m_EntityIndices[19] = 5;
	m_EntityIndices[20] = 2;
	m_EntityIndices[21] = 6;
	m_EntityIndices[22] = 3;
	m_EntityIndices[23] = 7;
}



void Entity::BuildEntityVertices(const RGBA& entityColor)
{
	Vertex3D entityVertex;
	entityVertex.m_Color = entityColor;

	entityVertex.m_Position = BottomSouthWestPoint();
	m_EntityVertices[0] = entityVertex;

	entityVertex.m_Position = BottomSouthEastPoint();
	m_EntityVertices[1] = entityVertex;

	entityVertex.m_Position = BottomNorthEastPoint();
	m_EntityVertices[2] = entityVertex;

	entityVertex.m_Position = BottomNorthWestPoint();
	m_EntityVertices[3] = entityVertex;

	entityVertex.m_Position = TopSouthWestPoint();
	m_EntityVertices[4] = entityVertex;

	entityVertex.m_Position = TopSouthEastPoint();
	m_EntityVertices[5] = entityVertex;

	entityVertex.m_Position = TopNorthEastPoint();
	m_EntityVertices[6] = entityVertex;

	entityVertex.m_Position = TopNorthWestPoint();
	m_EntityVertices[7] = entityVertex;
}



void Entity::DefineBoundingBox()
{
	m_BoundingBox = AABB3(BottomSouthWestPoint(), TopNorthEastPoint());
}



void Entity::TakeDamage()
{
	m_Health = m_Health - 1;

	if (m_Health < 0)
	{
		m_Health = 0;
	}
}



int Entity::GetRemainingHealth()
{
	return m_Health;
}



AABB3 Entity::GetBoundingBox()
{
	return m_BoundingBox;
}



void Entity::Render() const
{
	Vector3 translationDisplacement = m_Position - m_InitialPosition;
	
	Matrix4 modelMatrix = AdvancedRenderer::SingletonInstance()->GetModelMatrix(Vector3::ONE, EulerAngles::ZERO, translationDisplacement);
	AdvancedRenderer::SingletonInstance()->UpdateModelMatrix(modelMatrix);

	AdvancedRenderer::SingletonInstance()->DrawLinesMesh(m_EntityMesh, NUMBER_OF_ENTITY_VERTICES, NUMBER_OF_ENTITY_INDICES, 2.0f);

	modelMatrix = AdvancedRenderer::SingletonInstance()->GetModelMatrix(Vector3::ONE, EulerAngles::ZERO, Vector3::ZERO);
	AdvancedRenderer::SingletonInstance()->UpdateModelMatrix(modelMatrix);
}