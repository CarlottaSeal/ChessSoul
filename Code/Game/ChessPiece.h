#pragma once

#include "ChessObject.h"
#include "ChessPieceDefinition.h"

#include "Gamecommon.hpp"

struct BoardCoordinate;

class ChessKishi;

class ChessPiece : public ChessObject
{
public:
    ChessPiece(int ownerID, ChessPieceType type, Vec3 position, EulerAngles orientation);
    ~ChessPiece() override;

    void Update(float deltaSeconds) override;
    void Render() const override;

    void OnMove(IntVec2 from, IntVec2 to, ChessPieceType promoteType = ChessPieceType::Pawn);
    void OnSemiLegalMove(IntVec2 from, IntVec2 to, ChessPieceType newPromoteType = ChessPieceType::Count);

    ChessMoveResult OnRaycastMoveTest(IntVec2 from, IntVec2 to);
    ChessMoveResult OnRaycastSemiMoveTest(IntVec2 from, IntVec2 to);
    void OnRaycastValidMove(IntVec2 from, IntVec2 to, ChessMoveResult result);
    void OnRaycastSemiMove(IntVec2 from, IntVec2 to, ChessMoveResult result);
    
    std::vector<ChessPiece*> GetChessPiecesAroundAxially(IntVec2 pos);
    
    void ResetMyCoords(IntVec2 last, IntVec2 current);
    bool PromoteTo(ChessPieceType type);
    void UpdateMyTint(float deltaSeconds);

    void SetMyColor(int ownerID);
    int GetMyOwnerID() const;

    //for debug
    void InitializeDebugDraw();
    
private:
    ChessKishi* GetMyKishi() const;
    ChessKishi* GetAnotherKishi() const;

public:
    ChessPieceDefinition m_definition;

    //ChessPieceType m_type;
    int m_ownerKishiID;

    // IntVec2 m_lastCoord; move to parent class
    // IntVec2 m_currentCoord;
    // float m_lerpT = 1.0f;
    // float m_lerpSpeed = 1.0f; //vel

    //for pawn, king, knight
    bool m_hasMoved = false;
    //for pawn
    bool m_hasMoved2Squares = false;

    float m_varyTime = 0.f;

    std::vector<Vertex_PCU> m_debugVertices; 
};
