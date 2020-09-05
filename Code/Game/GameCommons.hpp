#pragma once

#include <vector>
#include <deque>
#include <set>

#include "Engine/Audio/Audio.hpp"
#include "Engine/DataStructures/ObjectPool.hpp"
#include "Engine/DebugTools/MemoryAnalytics/MemoryAnalytics.hpp"
#include "Engine/ErrorHandling/ErrorWarningAssert.hpp"
#include "Engine/ErrorHandling/StringUtils.hpp"
#include "Engine/FileUtilities/FileUtilities.hpp"
#include "Engine/Math/MathUtilities/MathUtilities.hpp"
#include "Engine/Math/VectorMath/2D/Vector2.hpp"
#include "Engine/Math/VectorMath/2D/AABB2.hpp"
#include "Engine/Math/VectorMath/2D/IntVector2.hpp"
#include "Engine/Math/VectorMath/3D/Vector3.hpp"
#include "Engine/Math/VectorMath/3D/AABB3.hpp"
#include "Engine/Math/VectorMath/3D/IntVector3.hpp"
#include "Engine/Math/EulerAngles/EulerAngles.hpp"
#include "Engine/Math/Noise/Noise.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/IO Utilities/BinaryFileIO.hpp"
#include "Engine/Renderer/RenderUtilities/AdvancedRenderer.hpp"
#include "Engine/Renderer/Camera/SimpleCamera3D.hpp"
#include "Engine/Renderer/Lighting/Light.hpp"
#include "Engine/Renderer/FrameBuffers/FrameBuffer.hpp"
#include "Engine/Threading/Thread.hpp"
#include "Engine/Time/Time.hpp"
#include "ThirdParty/stb/STB_Image.hpp"



extern bool g_FullScreen;
extern bool g_TwoPlayerMode;
extern bool g_DebugMode;
extern bool g_ScreenshotMode;
extern bool g_LoadGame;

extern bool g_IsQuitting;



const int OFFSET_FROM_WINDOWS_DESKTOP = 50;
const int WINDOW_PHYSICAL_WIDTH = (g_FullScreen) ? 1920 : 1600;
const int WINDOW_PHYSICAL_HEIGHT = (g_FullScreen) ? 1080 : 900;
const IntVector2 WINDOW_DIMENSIONS = IntVector2(WINDOW_PHYSICAL_WIDTH, WINDOW_PHYSICAL_HEIGHT);

const double VIEW_LEFT = 0.0;
const double VIEW_RIGHT = 16.0;
const double VIEW_BOTTOM = 0.0;
const double VIEW_TOP = 9.0;

extern int g_FrameNumber;



const int BLOCK_BITS_X = 4;
const int BLOCK_BITS_Y = 4;
const int BLOCK_BITS_Z = 7;
const int BLOCK_BITS_PER_LAYER = BLOCK_BITS_X + BLOCK_BITS_Y;
const int BLOCK_BITS_PER_CHUNK = BLOCK_BITS_PER_LAYER + BLOCK_BITS_Z;

const int NUMBER_OF_BLOCKS_X = 1 << BLOCK_BITS_X;
const int NUMBER_OF_BLOCKS_Y = 1 << BLOCK_BITS_Y;
const int NUMBER_OF_BLOCKS_Z = 1 << BLOCK_BITS_Z;
const int NUMBER_OF_BLOCKS_PER_LAYER = 1 << BLOCK_BITS_PER_LAYER;
const int NUMBER_OF_BLOCKS_PER_CHUNK = 1 << BLOCK_BITS_PER_CHUNK;

const int BLOCKS_X_MASK = NUMBER_OF_BLOCKS_X - 1;
const int BLOCKS_Y_MASK = NUMBER_OF_BLOCKS_Y - 1;
const int BLOCKS_Z_MASK = NUMBER_OF_BLOCKS_Z - 1;



const float BLOCK_WIDTH = 1.0f;
const float BLOCK_LENGTH = 1.0f;
const float BLOCK_HEIGHT = 1.0f;



const float MOUSE_SENSITIVITY = 0.022f;
const float LOOK_SENSITIVITY = 125.0f;

const float MOVEMENT_SPEED = 8.0f;
const float SPEED_MULTIPLIER = 2.0f;

const int ACTIVE_CHUNK_RADIUS = 8;
const int FLUSH_CHUNK_RADIUS = 10;

const float ACTIVATION_DISTANCE = static_cast<float>(ACTIVE_CHUNK_RADIUS) * 16.0f;
const float SQUARED_ACTIVATION_DISTANCE = ACTIVATION_DISTANCE * ACTIVATION_DISTANCE;

const float FLUSHING_DISTANCE = static_cast<float>(FLUSH_CHUNK_RADIUS)* 16.0f;
const float SQUARED_FLUSHING_DISTANCE = FLUSHING_DISTANCE * FLUSHING_DISTANCE;



const float VIEW_WIDTH = 16.0f;
const float VIEW_HEIGHT = 9.0f;

const float MAXIMUM_CAMERA_HEIGHT = static_cast<float>(NUMBER_OF_BLOCKS_Z) - 0.5f;
const float MINIMUM_CAMERA_HEIGHT = 1.5f;
const Vector3 EYE_LEVEL_VIEW = Vector3(0.0f, 0.0f, 0.69f);
const float DAY_DURATION_IN_MINUTES = 20.0f;
const float WATER_LEVEL = static_cast<float>(NUMBER_OF_BLOCKS_Z / 2) - 0.25f * BLOCK_HEIGHT;
const float WATER_RIPPLE_SPEED = 0.025f;

const IntVector2 DEFAULT_MOUSE_POSITION(850, 500);

const int NUMBER_OF_HUD_TILES = 9;



const float MAXIMUM_CAMERA_DISTANCE = 4.0f;
const float MAXIMUM_PLAYER_RANGE = 8.0f;



const float STANDARD_GRAVITY = -9.8f;
const float JUMP_IMPULSE = 5.0f;
const float COEFFICIENT_OF_FRICTION = 0.99f;
const size_t NUMBER_OF_BOUNDING_POINTS = 12U;
const float COLLISION_CORRECTION_VALUE = 0.001f;



const float ENEMY_RANGE_OF_VIEW = 10.0f;
const float ENEMY_SQUARED_RANGE_OF_VIEW = ENEMY_RANGE_OF_VIEW * ENEMY_RANGE_OF_VIEW;
const size_t MAXIMUM_NUMBER_OF_ENEMIES = 100U;
const size_t MAXIMUM_NUMBER_OF_CHUNKS = 768U;

const float BULLET_SPEED = 32.0f;
const float MAXIMUM_BULLET_LIFETIME = 1.0f;
const float PLAYER_RATE_OF_FIRE = 0.125f;
const float ENEMY_RATE_OF_FIRE = 0.25f;



const unsigned char SKY_LIGHT_VALUE = 15;

const unsigned char LIGHT_VALUE_MASK = 0x0F;
const unsigned char SKY_FLAG_MASK = 0x80;
const unsigned char LIGHTING_DIRTY_MASK = 0x40;
const unsigned char OPACITY_MASK = 0x20;