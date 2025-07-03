#include "ChessKishi.h"

ChessKishi::ChessKishi(int playerID, std::string name)
    : m_playerId(playerID)
    , m_name(name)
{
    if (m_playerId == 0)
    {
        m_colorName = "Blue";
    }
    if (m_playerId==1)
    {
        m_colorName = "Yellow";
    }
}

ChessKishi::~ChessKishi()
{
}
