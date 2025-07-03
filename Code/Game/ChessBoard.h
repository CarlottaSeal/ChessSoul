#pragma once
#include "ChessObject.h"
#include "ChessPiece.h"

class ChessBoard;
class ChessReferee;

enum ChessBoardCubeType
{
    BlackSquare,
    WhiteSquare,
    FrameSquare,
    Count
};

class ChessBoardCube:public ChessObject
{
public:
    ChessBoardCube(ChessBoard* owner, ChessBoardCubeType type, Vec3 position);
    ~ChessBoardCube() override;

    void Update(float deltaSeconds) override;
    void Render() const override;
    
    void InitializeVerts();

public:
    bool m_canOwnPiece = true;
    ChessBoardCubeType m_type;
    ChessBoard* m_owner;
    Vec3 m_position;
};

//----------------------------------------------------------------------------------------------------------
// struct BoardCoordinate
// {
//     int m_X;
//     int m_Y;
//
//     BoardCoordinate(int _x, int _y) : m_X(_x), m_Y(_y) {}
//
//     static const BoardCoordinate INVALID;
//     static const BoardCoordinate OUTSIDE;
//
//     // For std::map key comparison
//     bool operator<(const BoardCoordinate& other) const
//     {
//         return std::tie(m_X, m_Y) < std::tie(other.m_X, other.m_Y);
//     }
//
//     bool operator==(const BoardCoordinate& other) const
//     {
//         return std::tie(m_X, m_Y) == std::tie(other.m_X, other.m_Y);
//     }
//     
//     bool operator!=(const BoardCoordinate& other) const
//     {
//         return std::tie(m_X, m_Y) != std::tie(other.m_X, other.m_Y);
//     }
// };

class ChessBoard : public ChessObject
{
    friend class ChessReferee;
    friend class ChessPiece;
    friend class ChessBoardCube;
        
public:
    ChessBoard(ChessReferee* owner);
    ~ChessBoard() override;

    //Only for chess
    void InitializeBoardSquaresAndBuffers();
    void InitializeChessPieces();

    void Update(float deltaSeconds) override;
    void Render() const override;

    //Utils
    //bool IsOutOfBoard() const;
    Vec3 GetCenterPosition(IntVec2 tileCoord) const;
    bool IsTherePiece(IntVec2 coordinate) const;
    ChessPiece* GetPiece(IntVec2 coordinate) const;
    IntVec2 ParseCoordinate(std::string const& text);
    bool CaptureAnotherPiece(IntVec2 from, IntVec2 to);
    bool CaptureAnotherPiece(IntVec2 to);
    bool IsAxial(IntVec2 from, IntVec2 to) const;
    bool IsDiagonal(IntVec2 from, IntVec2 to) const;
    bool IsMovingKnight(IntVec2 from, IntVec2 to) const;
    bool HasBlockedOnAxial(IntVec2 from, IntVec2 to) const;
    bool HasBlockedOnDiagonal(IntVec2 from, IntVec2 to) const;

    AABB3 GetAABB() const;

protected:
    ChessReferee* m_owner = nullptr;
    Shader* m_boardShader = nullptr;
    Texture* m_boardTexture = nullptr; //Diffuse
    Texture* m_boardNormalTexture = nullptr; //normal
    Texture* m_sgeTexture = nullptr; 
    
    std::vector<ChessBoardCube> m_chessBoardCubes;
    std::vector<Vertex_PCUTBN> m_verts;
    std::vector<unsigned int> m_indeces;
    IndexBuffer* m_indexBuffer = nullptr;
    VertexBuffer* m_vertexBuffer = nullptr;

    std::vector<ChessPiece*> m_chessPieces;
    //std::map<IntVec2, ChessPiece*> m_chessPieces;
};


