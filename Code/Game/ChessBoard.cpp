#include "ChessBoard.h"
#include "ChessPiece.h"
#include "Game.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"

ChessBoardCube::ChessBoardCube(ChessBoard* owner, ChessBoardCubeType type, Vec3 position)
    : m_type(type)
    , m_owner(owner)
    , m_position(position)
{
    InitializeVerts();
}

ChessBoardCube::~ChessBoardCube()
{
}

void ChessBoardCube::Update(float deltaSeconds)
{
    UNUSED(deltaSeconds);
}

void ChessBoardCube::Render() const
{
}

void ChessBoardCube::InitializeVerts()
{
    if (m_type == ChessBoardCubeType::FrameSquare)
    {
        AddVertsForIndexAABB3D(m_owner->m_verts, m_owner->m_indeces,
            AABB3(m_position - Vec3(0.f,0.f,0.3f), m_position + Vec3(1.f,1.f,0.1f)),
            Rgba8::AQUA);
    }
    if (m_type == ChessBoardCubeType::BlackSquare)
    {
        AddVertsForIndexAABB3D(m_owner->m_verts, m_owner->m_indeces,
            AABB3(m_position - Vec3(0.f,0.f,0.3f), m_position + Vec3(1.f,1.f,0.f)),
            Rgba8::BLACK);
    }
    if (m_type == ChessBoardCubeType::WhiteSquare)
    {
        AddVertsForIndexAABB3D(m_owner->m_verts, m_owner->m_indeces,
            AABB3(m_position - Vec3(0.f,0.f,0.3f), m_position + Vec3(1.f,1.f,0.f)),
            Rgba8::WHITE);
    }
}

//---------------------------------------------------------------------------------------------
//const BoardCoordinate BoardCoordinate::INVALID(-1, -1);
//const BoardCoordinate BoardCoordinate::OUTSIDE(100, 100);

ChessBoard::ChessBoard(ChessReferee* owner)
    : m_owner(owner)
{
    m_boardShader = g_theRenderer->CreateOrGetShader("Data/Shaders/BlinnPhong", VertexType::VERTEX_PCUTBN);
    m_boardTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/woodfloor_d.png");
    m_boardNormalTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/woodfloor_n.png");
    m_sgeTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Bricks_sge.png");
    
    InitializeBoardSquaresAndBuffers();
    InitializeChessPieces();
}

ChessBoard::~ChessBoard()
{
	for (ChessPiece* piece : m_chessPieces)
	{
		if (piece != nullptr)
		{
            delete piece;
            piece = nullptr;
		}
	}
    m_chessPieces.clear();

    delete m_indexBuffer;
    m_indexBuffer = nullptr;
    
    delete m_vertexBuffer;
    m_vertexBuffer = nullptr;
}

void ChessBoard::InitializeBoardSquaresAndBuffers()
{
    //frame
    for (int frameI1 = -1; frameI1 < 8; frameI1++)
    {
        ChessBoardCube chessBoardCube = ChessBoardCube(this, ChessBoardCubeType::FrameSquare,
            Vec3(-1.f, float(frameI1), 0.f));
        ChessBoardCube chessBoardCube1 = ChessBoardCube(this, ChessBoardCubeType::FrameSquare,
            Vec3(float(frameI1)+1.f, -1.f, 0.f)); 
    }
    for (int frameI2 = 0; frameI2 < 9; frameI2++)
    {
        ChessBoardCube chessBoardCube = ChessBoardCube(this, ChessBoardCubeType::FrameSquare,
            Vec3(8.f, float(frameI2), 0.f));
        ChessBoardCube chessBoardCube1 = ChessBoardCube(this, ChessBoardCubeType::FrameSquare,
            Vec3(float(frameI2)-1.f, 8.f, 0.f)); 
    }
    //black&white square
    for (int squareY = 0; squareY < 8; squareY++)
    {
        for (int squareX = 0; squareX < 8; squareX++)
        {
            ChessBoardCubeType type = ((squareX + squareY) % 2 == 0)
            ? ChessBoardCubeType::WhiteSquare
            : ChessBoardCubeType::BlackSquare;

            ChessBoardCube chessBoardCube(this, type, Vec3((float)squareX, (float)squareY, 0.f));
        }
    }
    
    m_vertexBuffer = g_theRenderer->CreateVertexBuffer((unsigned int)m_verts.size() * sizeof(Vertex_PCUTBN), sizeof(Vertex_PCUTBN));
    m_indexBuffer = g_theRenderer->CreateIndexBuffer((unsigned int)m_indeces.size()* sizeof(unsigned int), sizeof(unsigned int));

    g_theRenderer->CopyCPUToGPU(m_verts.data(), (unsigned int)(m_verts.size() * sizeof(Vertex_PCUTBN)), m_vertexBuffer);
    g_theRenderer->CopyCPUToGPU(m_indeces.data(), (unsigned int)(m_indeces.size() * sizeof(unsigned int)), m_indexBuffer);
}

void ChessBoard::InitializeChessPieces()
{
    int whiteID = 1;
    int blackID = 0;

    std::vector<ChessPieceType> whiteBackRow =
    {
        ChessPieceType::Rook, ChessPieceType::Knight, ChessPieceType::Bishop, ChessPieceType::Queen,
        ChessPieceType::King, ChessPieceType::Bishop, ChessPieceType::Knight, ChessPieceType::Rook
    };

    for (int file = 0; file < 8; ++file)
    {
        Vec3 backPos((float)file + 0.5f, + 0.5f, 0.f);
        IntVec2 backCoord(file, 0);
        //m_chessPieces[backCoord] = new ChessPiece(whiteID, whiteBackRow[file], backPos, EulerAngles(90.f, 0.f, 0.f));
        ChessPiece* p1 = new ChessPiece(whiteID, whiteBackRow[file], backPos, EulerAngles(90.f, 0.f, 0.f));
        m_chessPieces.push_back(p1);
        
        Vec3 pawnPos((float)file+ 0.5f, 1.f+ 0.5f, 0.f);
        IntVec2 pawnCoord(file, 1);
        //m_chessPieces[pawnCoord] = new ChessPiece(whiteID, ChessPieceType::Pawn, pawnPos, EulerAngles(90.f, 0.f, 0.f));
        ChessPiece* p2 = new ChessPiece(whiteID, ChessPieceType::Pawn, pawnPos, EulerAngles(90.f, 0.f, 0.f));
        m_chessPieces.push_back(p2);
    }

    std::vector<ChessPieceType> blackBackRow =
    {
        ChessPieceType::Rook, ChessPieceType::Knight, ChessPieceType::Bishop, ChessPieceType::Queen,
        ChessPieceType::King, ChessPieceType::Bishop, ChessPieceType::Knight, ChessPieceType::Rook
    };

    for (int file = 0; file < 8; ++file)
    {
        Vec3 pawnPos((float)file+ 0.5f, 6.f+ 0.5f, 0.f);
        IntVec2 pawnCoord(file, 6);
        //m_chessPieces[pawnCoord] = new ChessPiece(blackID, ChessPieceType::Pawn, pawnPos, EulerAngles(-90.f, 0.f, 0.f)); 
        ChessPiece* p1 = new ChessPiece(blackID, ChessPieceType::Pawn, pawnPos, EulerAngles(-90.f, 0.f, 0.f)); 
        m_chessPieces.push_back(p1);
        
        Vec3 backPos((float)file+ 0.5f, 7.f+ 0.5f, 0.f);
        IntVec2 backCoord(file, 7);
        //m_chessPieces[backCoord] = new ChessPiece(blackID, blackBackRow[file], backPos, EulerAngles(-90.f, 0.f, 0.f));
        ChessPiece* p2 = new ChessPiece(blackID, blackBackRow[file], backPos, EulerAngles(-90.f, 0.f, 0.f));
        m_chessPieces.push_back(p2);
    }
}

void ChessBoard::Update(float deltaSeconds)
{
    //std::vector<ChessPiece*> pieces = m_chessPieces;
    for (ChessPiece* piece : m_chessPieces)
    {
        if (piece != nullptr)
        {
            piece->Update(deltaSeconds);
        }
    }
}

void ChessBoard::Render() const
{
    //Draw board first
    g_theRenderer->BindTexture(m_boardTexture, 0);
    g_theRenderer->BindTexture(m_boardNormalTexture, 1);
    g_theRenderer->BindTexture(m_sgeTexture, 2);
    g_theRenderer->BindShader(m_boardShader);
    //g_theRenderer->BindShader(nullptr);
    g_theRenderer->SetModelConstants();
    g_theRenderer->DrawIndexBuffer(m_vertexBuffer, m_indexBuffer, (unsigned int)m_indeces.size());

    //DrawPieces
	for (ChessPiece* piece : m_chessPieces)
	{
		if (piece != nullptr)
		{
			piece->Render();
		}
	}
}

Vec3 ChessBoard::GetCenterPosition(IntVec2 tileCoord) const
{
    return Vec3((float)tileCoord.x+0.5f, (float)tileCoord.y+0.5f, 0.0f);
}

bool ChessBoard::IsTherePiece(IntVec2 coordinate) const
{
    for (ChessPiece* piece : m_chessPieces)
    {
        if (piece != nullptr)
        {
            if (piece->m_currentCoord == coordinate)
            {
                return true;
            }
        }
    }
    return false;
    //auto iter = m_chessPieces.find(coordinate);
    //return (iter != m_chessPieces.end() && iter->second != nullptr);
}

ChessPiece* ChessBoard::GetPiece(IntVec2 coordinate) const
{
    for (ChessPiece* piece : m_chessPieces)
    {
        if (piece != nullptr)
        {
            if (piece->m_currentCoord == coordinate)
            {
                return piece;
            }
        }
    }
    // auto iter = m_chessPieces.find(coordinate);
    // if (iter != m_chessPieces.end())
    // {
    //     return iter->second;
    // }
    return nullptr;
}

IntVec2 ChessBoard::ParseCoordinate(std::string const& text)
{
    if (text.length() != 2)
        return IntVec2::NEGATIVEONE; 

    char fileChar = text[0]; // 'a' - 'h'
    char rankChar = text[1]; // '1' - '8'

    int x = fileChar - 'a';           // 'a' → 0, 'b' → 1, ..., 'h' → 7
    int y = rankChar - '1';           // '1' → 0, ..., '8' → 7

    if (x < 0 || x > 7 || y < 0 || y > 7)
        return IntVec2(100,100); 

    return IntVec2(x, y);
}

bool ChessBoard::CaptureAnotherPiece(IntVec2 from, IntVec2 to) //只有吃子的时候有用了
{
    ChessPiece* fromPiece = GetPiece(from);
    ChessPiece* toPiece = GetPiece(to);

    if (fromPiece == nullptr)
        return false;

    //bool didCapture = false;

    if (toPiece != nullptr)
    {
        if (toPiece->m_definition.m_type == ChessPieceType::King)
        {
            g_theGame->m_hasWon = true;
        }
        delete toPiece;
        m_chessPieces.erase(std::remove(m_chessPieces.begin(), m_chessPieces.end(), toPiece), m_chessPieces.end());
        //didCapture = true;
    }

    //m_chessPieces[to] = fromPiece;
    //fromPiece->m_position = GetCenterPosition(to); TODO:???挪动位置
    //m_chessPieces.erase(from);
    return true;
}

bool ChessBoard::CaptureAnotherPiece(IntVec2 to)
{
    ChessPiece* toPiece = GetPiece(to);
    if (toPiece != nullptr)
    {
        if (toPiece->m_definition.m_type == ChessPieceType::King)
        {
            g_theGame->m_hasWon = true;
        }
        delete toPiece;
        m_chessPieces.erase(std::remove(m_chessPieces.begin(), m_chessPieces.end(), toPiece), m_chessPieces.end());
        return true;
    }
    return false;
}

bool ChessBoard::IsAxial(IntVec2 from, IntVec2 to) const
{
    return from.x == to.x || from.y == to.y;
}

bool ChessBoard::IsDiagonal(IntVec2 from, IntVec2 to) const
{
    return abs(from.x - to.x) == abs(from.y - to.y);
}

bool ChessBoard::IsMovingKnight(IntVec2 from, IntVec2 to) const
{
    int dx = abs(to.x - from.x);
    int dy = abs(to.y - from.y);
    return (dx == 2 && dy == 1) || (dx == 1 && dy == 2);
}

bool ChessBoard::HasBlockedOnAxial(IntVec2 from, IntVec2 to) const
{
    if (from.x != to.x && from.y != to.y)
        return false; // Not axial //only called when its axial plz

    int dx = (to.x > from.x) ? 1 : (to.x < from.x ? -1 : 0);
    int dy = (to.y > from.y) ? 1 : (to.y < from.y ? -1 : 0);

    IntVec2 check = from;
    check.x += dx;
    check.y += dy;

    while (check != to)
    {
        if (IsTherePiece(check))
            return true; // Blocked

        check.x += dx;
        check.y += dy;
    }
    return false; 
}

bool ChessBoard::HasBlockedOnDiagonal(IntVec2 from, IntVec2 to) const
{
    int dx = to.x - from.x;
    int dy = to.y - from.y;

    if (abs(dx) != abs(dy))
        return false; // Not diagonal //Only call when its diagonal plz

    dx = (dx > 0) ? 1 : -1;
    dy = (dy > 0) ? 1 : -1;

    IntVec2 check = from;
    check.x += dx;
    check.y += dy;

    while (check != to)
    {
        if (IsTherePiece(check))
            return true; // Blocked!

        check.x += dx;
        check.y += dy;
    }

    return false;
}

AABB3 ChessBoard::GetAABB() const
{
    return AABB3(Vec3(0.f,0.f,-1.f), Vec3(8.f,8.f,0.f));
}


