#pragma once
#include <string>
#include <vector>

#include "Gamecommon.hpp"

class ChessKishi
{
    friend class ChessBoard;
    friend class ChessPiece;
public:
    ChessKishi(int playerID, std::string name = "Hikari");
    ~ChessKishi();

    int GetPlayerID() const {return m_playerId;}
    std::string GetName() const {return m_name;}

public:
    ChessPiece* m_lastMovedPiece = nullptr;

protected:
    int m_playerId;
    std::string m_name;
    std::string m_colorName;

    //std::vector<ChessPiece*> m_myChessPieces;
};
