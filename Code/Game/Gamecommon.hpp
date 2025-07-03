#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/EngineCommon.hpp"

void DebugDrawRing( Vec2 const& center, float radius, float thickness, Rgba8 const& color );
void DebugDrawLine( Vec2 const& start, Vec2 const& end, Rgba8 color, float thickness );

constexpr float PI = 3.1415926535897932384626433832795f;

constexpr int NUM_LINE_TRIS = 2;
constexpr int NUM_LINE_VERTS = 3 * NUM_LINE_TRIS;

constexpr int NUM_SIDES = 32;  
constexpr int NUM_TRIS = NUM_SIDES;  
constexpr int NUM_VERTS = 3 * NUM_TRIS;

constexpr int NUM_KISHI =2;

enum class ChessPieceType
{
    Bishop,
    Knight,
    Rook,
    Queen,
    King,
    Pawn,
    Count
};

enum class ChessMoveResult
{
    UNKNOWN,
    VALID_MOVE_NORMAL,
    VALID_MOVE_PAWN_2SQUARE,
    VALID_MOVE_PROMOTION,
    VALID_CASTLE_KINGSIDE,
    VALID_CASTLE_QUEENSIDE,
    VALID_CAPTURE_NORMAL,
    VALID_CAPTURE_ENPASSANT,

    INVALID_MOVE_BAD_LOCATION,
    INVALID_MOVE_NO_PIECE,
    INVALID_MOVE_NOT_YOUR_PIECE,
    INVALID_MOVE_WRONG_MOVE_SHAPE,
    INVALID_MOVE_ZERO_DISTANCE,
    INVALID_MOVE_PAWN_BLOCKED,
    INVALID_MOVE_DESTINATION_BLOCKED,
    INVALID_MOVE_PATH_BLOCKED,
    INVALID_MOVE_ENDS_IN_CHECK,
    INVALID_MOVE_KING_TOGETHER,
    INVALID_MOVE_NO_PROMOTION,
    INVALID_ENPASSANT_STALE,
    INVALID_CASTLE_KING_HAS_MOVED,
    INVALID_CASTLE_ROOK_HAS_MOVED,
    INVALID_CASTLE_PATH_BLOCKED,
    INVALID_CASTLE_THROUGH_CHECK,
    INVALID_CASTLE_OUT_OF_CHECK,
    COUNT
};

class App;
extern App* g_theApp;

class Game;
extern Game* g_theGame;

class InputSystem;

extern InputSystem* g_theInput;

class Renderer;

extern Renderer* g_theRenderer;

