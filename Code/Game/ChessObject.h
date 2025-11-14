#pragma once
#include <vector>

#include "Gamecommon.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec3.hpp"

enum class ChessPieceType;
class VertexBuffer;
class IndexBuffer;

class ChessObject
{
public:
    //ChessObject();
    virtual ~ChessObject();

    virtual void Update(float deltaSeconds) = 0;
    virtual void Render() const = 0;

    virtual Mat44 GetModelToWorldTransform() const;

    void OnImpacted();
    void OnUnImpacted();
    void OnGrabbed(); //only for piece
    void OnUnGrabbed();

public:
    Vec3 m_position;
    EulerAngles m_orientation;

    //for piece
    ChessPieceType m_type = ChessPieceType::Pawn;
    Rgba8 m_tint = Rgba8::WHITE;
    Rgba8 m_originalTint;
    IntVec2 m_lastCoord = IntVec2::ZERO;
    IntVec2 m_currentCoord = IntVec2::ZERO;
    float m_lerpT = 1.0f;
    float m_lerpSpeed = 1.0f; //vel

    //Only used for piece
    bool m_isGrabbed = false;
    //Used for chess cube or piece
    bool m_isImpacted = false;
};
