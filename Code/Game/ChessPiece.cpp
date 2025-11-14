#include "ChessPiece.h"

#include <D3Dcompiler.h>

#include "ChessObject.h"
#include "ChessBoard.h"
#include "ChessReferee.h"
#include "Game.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/FloatRange.hpp"

extern Game* g_theGame;

ChessPiece::ChessPiece(int ownerID, ChessPieceType type, Vec3 position, EulerAngles orientation)
    : m_definition(ChessPieceDefinition::GetChessPieceDefinitionByChessPieceType(type))
    , m_ownerKishiID(ownerID)
{
    m_type = type;
    m_lastCoord = IntVec2(RoundDownToInt(position.x), RoundDownToInt(position.y));
    m_currentCoord = IntVec2(RoundDownToInt(position.x), RoundDownToInt(position.y));

    SetMyColor(ownerID);
    m_originalTint = m_tint;
    
    m_position = position;
    m_orientation = orientation;

    //InitializeDebugDraw();
}

ChessPiece::~ChessPiece()
{
}

void ChessPiece::Update(float deltaSeconds)
{
    UpdateMyTint(deltaSeconds);
    
    if (m_lerpT >= 1.0f)
    {
        m_position = g_theGame->m_chessReferee->m_chessBoard->GetCenterPosition(m_currentCoord); // 确保最终对齐
        return;
    }
    m_lerpT += m_lerpSpeed * deltaSeconds;
    if (m_lerpT > 1.0f)
    {
        m_lerpT = 1.0f;
    }
    
    if (m_lastCoord != m_currentCoord)
    {
        Vec3 lastPos = g_theGame->m_chessReferee->m_chessBoard->GetCenterPosition(m_lastCoord);
        Vec3 currentPos = g_theGame->m_chessReferee->m_chessBoard->GetCenterPosition(m_currentCoord);

        float t = SmoothStop3(m_lerpT);
        m_position.x = Interpolate(lastPos.x, currentPos.x, t);
        m_position.y = Interpolate(lastPos.y, currentPos.y, t);
        
        if (m_type == ChessPieceType::Knight)
        {
            m_position.z = 1 - SmoothStop3(m_lerpT);
        }
        else
        {
            m_position.z = 0.f;
        }
    }
}

void ChessPiece::UpdateMyTint(float deltaSeconds)
{
    m_varyTime += deltaSeconds;
    if (m_varyTime > 360.f)
    {
        m_varyTime = 0.f;
    }
    
    if (m_isImpacted==true && !m_isGrabbed)
    {
        if (m_ownerKishiID == 0)
            m_tint = Rgba8::WHITE;
            //m_tint = Rgba8::TEAL;
        else
        {
            m_tint = Rgba8::WHITE;
            //m_tint = Rgba8::PEACH;
        }
        //m_tint.r += 180;
        //m_tint.g += 180;
        //m_tint.b += 180;
        //GetClamped(m_tint.r, 0, 255);
        //GetClamped(m_tint.g, 0, 255);
        //GetClamped(m_tint.b, 0, 255);
    }
    if (m_isGrabbed)
    {
        float pulse = 0.5f + 0.5f * sinf(m_varyTime * 3.0f);
        m_tint.r = m_originalTint.r;
        m_tint.g = m_originalTint.g;
        m_tint.b = m_originalTint.b;
        m_tint.a = (unsigned char)(Interpolate(150.0f, 255.0f, pulse));
    }
    if (!m_isImpacted && !m_isGrabbed)
    {
        m_tint = m_originalTint;
    }
}

void ChessPiece::SetMyColor(int ownerID)
{
    if (ownerID==0)
    {
        m_tint = Rgba8::WHITE;
        //m_tint = Rgba8::BLUE;
    }
    else
    {
        m_tint = Rgba8::WHITE;
        //m_tint = Rgba8::YELLOW;
    }
}

int ChessPiece::GetMyOwnerID() const
{
    return m_ownerKishiID;
}

void ChessPiece::Render() const
{
    // g_theRenderer->BindShader(m_definition.m_shader);
    // g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
    // g_theRenderer->BindTexture(m_definition.m_diffuseTextures[m_ownerKishiID],0);
    // g_theRenderer->BindTexture(m_definition.m_normalTextures[m_ownerKishiID],1);
    // g_theRenderer->BindTexture(m_definition.m_specGlossEmitTextures[m_ownerKishiID],2);
    //
    // g_theRenderer->SetModelConstants(GetModelToWorldTransform(), m_tint);
    // g_theRenderer->DrawIndexBuffer(m_definition.m_vertexBuffers[m_ownerKishiID],
    //     m_definition.m_indexBuffers[m_ownerKishiID],
    //     (unsigned int)m_definition.m_indexes[m_ownerKishiID].size());
    int set = g_theGame->m_setSelected;
    g_theRenderer->BindShader(m_definition.m_shader);
    g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
    g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
    g_theRenderer->BindTexture(m_definition.m_sets[set][m_ownerKishiID]->m_diffuseTexture,0);
    g_theRenderer->BindTexture(m_definition.m_sets[set][m_ownerKishiID]->m_normalTexture,1);
    g_theRenderer->BindTexture(m_definition.m_sets[set][m_ownerKishiID]->m_specularTexture,2);

    //GetModelToWorldTransform().Append();
    //m_definition.m_sets[set][m_ownerKishiID]->m_transform.Append(GetModelToWorldTransform());
    Mat44 mat = GetModelToWorldTransform();
    g_theRenderer->SetModelConstants(mat, m_tint);
    g_theRenderer->DrawIndexBuffer(m_definition.m_sets[set][m_ownerKishiID]->m_vertexBuffer,
        m_definition.m_sets[set][m_ownerKishiID]->m_indexBuffer,
        (unsigned int)m_definition.m_sets[set][m_ownerKishiID]->m_indices.size());

    // if (m_isImpacted == true)
    // {
    //     g_theRenderer->BindShader(nullptr);
    //     g_theRenderer->BindTexture(nullptr);
    //     g_theRenderer->SetModelConstants(GetModelToWorldTransform());
    //     g_theRenderer->DrawVertexArray(m_debugVertices);
    // }
}

ChessKishi* ChessPiece::GetMyKishi() const
{
    return g_theGame->m_chessKishi[m_ownerKishiID];
}

ChessKishi* ChessPiece::GetAnotherKishi() const
{
    return g_theGame->m_chessKishi[1-m_ownerKishiID];
}

Mat44 ChessPiece::GetModelToWorldTransform() const
{
    Mat44 rotate;
    rotate = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
    Mat44 translate;
    translate = Mat44::MakeTranslation3D(m_position);

    translate.Append(rotate);
    translate.Append( m_definition.m_sets[g_theGame->m_setSelected][m_ownerKishiID]->m_transform);
    return translate;
}

void ChessPiece::OnMove(IntVec2 from, IntVec2 to, ChessPieceType promoteType) //call时已确定to为空或对方棋子
{
    //m_lastCoord = from;
    //ChessPiece* fromPiece = g_theGame->m_chessReferee->m_chessBoard->GetPiece(from);
    ChessPiece* fromPiece = this;
    ChessPiece* toPiece = g_theGame->m_chessReferee->m_chessBoard->GetPiece(to);
    
    if (g_theGame->m_chessReferee->m_chessBoard->IsDiagonal(from, to))
    {
        if (g_theGame->m_chessReferee->m_chessBoard->HasBlockedOnDiagonal(from, to))
        {
            m_currentCoord = from;
            ChessMoveResult result;
            result = ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
            g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
            return;
        }
        
        if (m_type == ChessPieceType::Bishop)
        {
            m_lastCoord = from;
            g_theDevConsole->AddLine(Rgba8::MISTBLUE,
        "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
            +GetMyKishi()->m_colorName+
            ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
            + " to " + std::to_string(to.x) + std::to_string(to.y));
            if (toPiece)
            {
                ChessMoveResult result;
                result = ChessMoveResult::VALID_CAPTURE_NORMAL;
                g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));

                g_theDevConsole->AddLine(Rgba8::PEACH,
                "Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                    + GetMyKishi()->m_colorName +") captured Player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
                    + " ("+ GetAnotherKishi()->m_colorName +")'s " + toPiece->m_definition.m_name + " at " +
                    std::to_string(to.x) + std::to_string(to.y));
            }
            else
            {
                ChessMoveResult result;
                result = ChessMoveResult::VALID_MOVE_NORMAL;
                g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
            }
            fromPiece->m_lerpT = 0.f;
            g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
            m_currentCoord = to;
            m_hasMoved = true;
            GetMyKishi()->m_lastMovedPiece = this;

            g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
            return;
        }
        if (m_type == ChessPieceType::Queen)
        {
            m_lastCoord = from;
            g_theDevConsole->AddLine(Rgba8::MISTBLUE,
        "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
            +GetMyKishi()->m_colorName+
            ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
            + " to " + std::to_string(to.x) + std::to_string(to.y));
            if (toPiece)
            {
                ChessMoveResult result;
                result = ChessMoveResult::VALID_CAPTURE_NORMAL;
                g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                
                g_theDevConsole->AddLine(Rgba8::PEACH,
                "Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                    + GetMyKishi()->m_colorName +") captured Player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
                    + " ("+ GetAnotherKishi()->m_colorName +")'s " + toPiece->m_definition.m_name + " at " +
                    std::to_string(to.x) + std::to_string(to.y));
            }
            else
            {
                ChessMoveResult result;
                result = ChessMoveResult::VALID_MOVE_NORMAL;
                g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
            }
            fromPiece->m_lerpT = 0.f;
            g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
            m_currentCoord = to;
            m_hasMoved = true;
            GetMyKishi()->m_lastMovedPiece = this;

            g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
            return;
        }
        if (m_type == ChessPieceType::King)
        {
            if (abs(from.x - to.x) >1 || abs(from.y == to.y)>1) //斜着走不可能王车易位
            {
                m_currentCoord = from;
                ChessMoveResult result;
                result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                return;
            }
            std::vector<ChessPiece*> piecesAround= GetChessPiecesAroundAxially(to);
            bool isThereKing = false;
            for (ChessPiece* piece : piecesAround)
            {
                if (piece != this && piece->m_type == ChessPieceType::King)
                {
                    isThereKing = true;
                }
            }
            if (isThereKing)
            {
                m_currentCoord = from;
                ChessMoveResult result;
                result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                return;
            }
            else
            {
                m_lastCoord = from;
                g_theDevConsole->AddLine(Rgba8::MISTBLUE,
        "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
            +GetMyKishi()->m_colorName+
            ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
            + " to " + std::to_string(to.x) + std::to_string(to.y));
                if (toPiece)
                {
                    ChessMoveResult result;
                    result = ChessMoveResult::VALID_CAPTURE_NORMAL;
                    g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                    
                    g_theDevConsole->AddLine(Rgba8::PEACH,
                    "Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                        + GetMyKishi()->m_colorName +") captured Player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
                        + " ("+ GetAnotherKishi()->m_colorName +")'s " + toPiece->m_definition.m_name + " at " +
                        std::to_string(to.x) + std::to_string(to.y));
                }
                else
                {
                    ChessMoveResult result;
                    result = ChessMoveResult::VALID_MOVE_NORMAL;
                    g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                }
                fromPiece->m_lerpT = 0.f;
                g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
                m_currentCoord = to;
                m_hasMoved = true;
                GetMyKishi()->m_lastMovedPiece = this;

                g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
                return;
            }
        }
        if (m_type == ChessPieceType::Pawn) //En passant/吃子 斜着移动
        {
            if (abs(from.x - to.x) ==1 && abs(from.y - to.y)==1)
            {
                if (!toPiece)
                {
                    if (m_ownerKishiID==0) //black
                    {
                        if (from.y > to.y) //moving forward le
                        {
                            ChessPiece* bePassantedP = nullptr;
                            if (from.x > to.x)
                            {
                                bePassantedP = g_theGame->m_chessReferee->m_chessBoard->GetPiece(IntVec2(from.x-1, from.y));
                            }
                            if (from.x < to.x)
                            {
                                bePassantedP = g_theGame->m_chessReferee->m_chessBoard->GetPiece(IntVec2(from.x + 1, from.y));
                            }
                            if (bePassantedP && bePassantedP->m_ownerKishiID != m_ownerKishiID
                                && bePassantedP->m_currentCoord.y==m_currentCoord.y)
                            {
                                if (!bePassantedP->m_hasMoved2Squares)
                                {
                                    m_currentCoord = from;
                                    ChessMoveResult result;
                                    result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                                    g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                    return;
                                }
                                if (bePassantedP->m_hasMoved2Squares && !(bePassantedP->GetMyKishi()->m_lastMovedPiece == bePassantedP))
                                {
                                    m_currentCoord = from;
                                    ChessMoveResult result;
                                    result = ChessMoveResult::INVALID_ENPASSANT_STALE;
                                    g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                    return;
                                }
                                if (bePassantedP->m_hasMoved2Squares &&
                                    (bePassantedP->GetMyKishi()->m_lastMovedPiece == bePassantedP) )
                                    //En passant
                                {
                                    if (!(abs(bePassantedP->m_currentCoord.y-bePassantedP->m_lastCoord.y)==2))
                                    {
                                        m_currentCoord = from;
                                        ChessMoveResult result;
                                        result = ChessMoveResult::INVALID_ENPASSANT_STALE;
                                        g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                        return;
                                    }

									ChessMoveResult result;
									result = ChessMoveResult::VALID_CAPTURE_ENPASSANT;
									g_theDevConsole->AddLine(Rgba8::BLUE, GetMoveResultString(result));
                                    
                                    g_theDevConsole->AddLine(Rgba8::MISTBLUE,
                                "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                                    +GetMyKishi()->m_colorName+
                                    ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
                                    + " to " + std::to_string(to.x) + std::to_string(to.y));

                                    g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(bePassantedP->m_currentCoord);
                                    
                                    ResetMyCoords(from, to);
                                    m_hasMoved = true;
                                    GetMyKishi()->m_lastMovedPiece = this;

                                    g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
                                    //fromPiece->m_lerpT = 0.f;
                                    //g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
                                    //m_lastCoord = from;
                                    //m_currentCoord = to;
                                    return;
                                }
                            }
                            else
                            {
                                m_currentCoord = from;
                                ChessMoveResult result;
                                result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                                g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                return;
                            }
                        }
                    }
                    if (m_ownerKishiID==1) //white
                    {
                        if (from.y < to.y) //moving forward le
                        {
							ChessPiece* bePassantedP = nullptr;
							if (from.x > to.x)
							{
								bePassantedP = g_theGame->m_chessReferee->m_chessBoard->GetPiece(IntVec2(from.x - 1, from.y));
							}
							if (from.x < to.x)
							{
								bePassantedP = g_theGame->m_chessReferee->m_chessBoard->GetPiece(IntVec2(from.x + 1, from.y));
							}
                            if (bePassantedP && bePassantedP->m_ownerKishiID != m_ownerKishiID
                                && bePassantedP->m_currentCoord.y==m_currentCoord.y)
                            {
                                if (!bePassantedP->m_hasMoved2Squares)
                                {
                                    m_currentCoord = from;
                                    ChessMoveResult result;
                                    result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                                    g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                    return;
                                }
                                if (bePassantedP->m_hasMoved2Squares && !(bePassantedP->GetMyKishi()->m_lastMovedPiece == bePassantedP))
                                {
                                    m_currentCoord = from;
                                    ChessMoveResult result;
                                    result = ChessMoveResult::INVALID_ENPASSANT_STALE;
                                    g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                    return;
                                }
                                if (bePassantedP->m_hasMoved2Squares &&
                                    (bePassantedP->GetMyKishi()->m_lastMovedPiece == bePassantedP) )
                                    //En passant
                                {
                                    if (!(abs(bePassantedP->m_currentCoord.y-bePassantedP->m_lastCoord.y)==2))
                                    {
                                        m_currentCoord = from;
                                        ChessMoveResult result;
                                        result = ChessMoveResult::INVALID_ENPASSANT_STALE;
                                        g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                        return;
                                    }
                                    
									ChessMoveResult result;
									result = ChessMoveResult::VALID_CAPTURE_ENPASSANT;
									g_theDevConsole->AddLine(Rgba8::BLUE, GetMoveResultString(result));

                                    g_theDevConsole->AddLine(Rgba8::MISTBLUE,
                                "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                                    +GetMyKishi()->m_colorName+
                                    ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
                                    + " to " + std::to_string(to.x) + std::to_string(to.y));

                                    g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(bePassantedP->m_currentCoord);
                                    
                                    ResetMyCoords(from, to);
                                    m_hasMoved = true;
                                    GetMyKishi()->m_lastMovedPiece = this;

                                    g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
                                    //fromPiece->m_lerpT = 0.f;
                                    //g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
                                    //m_lastCoord = from;
                                    //m_currentCoord = to;
                                    return;
                                }
                            }
                            else
                            {
                                m_currentCoord = from;
                                ChessMoveResult result;
                                result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                                g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                return;
                            }
                        }
                    }
                }
                else
                {
                    if (m_ownerKishiID==0) //black
                    {
                        if (from.y > to.y) //moving forward le
                        {
                            if (toPiece) //要吃子
                            {
                                if (to.y == 0 || to.y == 7) //其实黑方这里只能to==0
                                {
                                    if (promoteType == ChessPieceType::Count)
                                    {
                                        m_currentCoord = from;
                                        ChessMoveResult result;
                                        result = ChessMoveResult::INVALID_MOVE_NO_PROMOTION;
                                        g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                        return;
                                    }
                                    else
                                    {
                                        if (promoteType == ChessPieceType::Queen ||
                                            promoteType == ChessPieceType::Rook ||
                                            promoteType == ChessPieceType::Bishop ||
                                            promoteType == ChessPieceType::Knight)
                                        {
                                            PromoteTo(promoteType);
                                        }
                                        else
                                        {
                                            m_currentCoord = from;
                                            ChessMoveResult result;
                                            result = ChessMoveResult::INVALID_MOVE_NO_PROMOTION;
                                            g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                            return;
                                        }
                                    }
                                }
                                
                                g_theDevConsole->AddLine(Rgba8::MISTBLUE,
        "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
            +GetMyKishi()->m_colorName+
            ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
            + " to " + std::to_string(to.x) + std::to_string(to.y));
                                
                                ChessMoveResult result;
                                result = ChessMoveResult::VALID_CAPTURE_NORMAL;
                                g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                
                                g_theDevConsole->AddLine(Rgba8::PEACH,
                                "Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                                    + GetMyKishi()->m_colorName +") captured Player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
                                    + " ("+ GetAnotherKishi()->m_colorName +")'s " + toPiece->m_definition.m_name + " at " +
                                    std::to_string(to.x) + std::to_string(to.y));
                                
                                fromPiece->m_lerpT = 0.f;
                                g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
                                m_lastCoord = from;
                                m_currentCoord = to;
                                m_hasMoved = true;
                                GetMyKishi()->m_lastMovedPiece = this;

                                g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
                                return;
                            }
                            else
                            {
                                m_currentCoord = from;
                                ChessMoveResult result;
                                result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                                g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                return;
                            }
                        }
                        else
                        {
                            m_currentCoord = from;
                            ChessMoveResult result;
                            result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                            g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                            return;
                        }
                    }
                    if (m_ownerKishiID==1)
                    {
                    if (from.y < to.y) //moving forward le
                        {
                            if (toPiece) //要吃子
                            {
                                if (to.y == 0 || to.y == 7) //其实黑方这里只能to==0
                                {
                                    if (promoteType == ChessPieceType::Count)
                                    {
                                        m_currentCoord = from;
                                        ChessMoveResult result;
                                        result = ChessMoveResult::INVALID_MOVE_NO_PROMOTION;
                                        g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                        return;
                                    }
                                    else
                                    {
                                        if (promoteType == ChessPieceType::Queen ||
                                            promoteType == ChessPieceType::Rook ||
                                            promoteType == ChessPieceType::Bishop ||
                                            promoteType == ChessPieceType::Knight)
                                        {
                                            PromoteTo(promoteType);
                                        }
                                        else
                                        {
                                            m_currentCoord = from;
                                            ChessMoveResult result;
                                            result = ChessMoveResult::INVALID_MOVE_NO_PROMOTION;
                                            g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                            return;
                                        }
                                    }
                                }
                                
                                g_theDevConsole->AddLine(Rgba8::MISTBLUE,
        "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
            +GetMyKishi()->m_colorName+
            ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
            + " to " + std::to_string(to.x) + std::to_string(to.y));
                                
                                ChessMoveResult result;
                                result = ChessMoveResult::VALID_CAPTURE_NORMAL;
                                g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                
                                g_theDevConsole->AddLine(Rgba8::PEACH,
                                "Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                                    + GetMyKishi()->m_colorName +") captured Player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
                                    + " ("+ GetAnotherKishi()->m_colorName +")'s " + toPiece->m_definition.m_name + " at " +
                                    std::to_string(to.x) + std::to_string(to.y));
                                
                                fromPiece->m_lerpT = 0.f;
                                g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
                                m_lastCoord = from;
                                m_currentCoord = to;
                                m_hasMoved = true;
                                GetMyKishi()->m_lastMovedPiece = this;

                                g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
                                return;
                            }
                            else
                            {
                                m_currentCoord = from;
                                ChessMoveResult result;
                                result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                                g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                return;
                            }
                        }
                        else
                        {
                            m_currentCoord = from;
                            ChessMoveResult result;
                            result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                            g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                            return;
                        }
                    }
                }
            }
            else
            {
                m_currentCoord = from;
                ChessMoveResult result;
                result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                return;
            }
        }
        else
        {
            m_currentCoord = from;
            ChessMoveResult result;
            result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
            g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
            return;
        }
    }

    if (g_theGame->m_chessReferee->m_chessBoard->IsAxial(from, to))
    {
        // if (g_theGame->m_chessReferee->m_chessBoard->HasBlockedOnAxial(from, to)) //TODO: 处理王车易位
        // {
        //     m_currentCoord = from;
        //     ChessMoveResult result;
        //     result = ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
        //     g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
        //     return;
        // }
        if (m_type == ChessPieceType::Queen)
        {
            if (g_theGame->m_chessReferee->m_chessBoard->HasBlockedOnAxial(from, to)) //TODO: 处理王车易位
            {
                m_currentCoord = from;
                ChessMoveResult result;
                result = ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
                g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                return;
            }
            g_theDevConsole->AddLine(Rgba8::MISTBLUE,
        "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
            +GetMyKishi()->m_colorName+
            ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
            + " to " + std::to_string(to.x) + std::to_string(to.y));
            if (toPiece)
            {
                ChessMoveResult result;
                result = ChessMoveResult::VALID_CAPTURE_NORMAL;
                g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                
                g_theDevConsole->AddLine(Rgba8::PEACH,
                "Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                    + GetMyKishi()->m_colorName +") captured Player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
                    + " ("+ GetAnotherKishi()->m_colorName +")'s " + toPiece->m_definition.m_name + " at " +
                    std::to_string(to.x) + std::to_string(to.y));
            }
            else
            {
                ChessMoveResult result;
                result = ChessMoveResult::VALID_MOVE_NORMAL;
                g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
            }
            fromPiece->m_lerpT = 0.f;
            g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
            m_lastCoord = from;
            m_currentCoord = to;
            m_hasMoved = true;
            GetMyKishi()->m_lastMovedPiece = this;

            g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
            return;
        }
        if (m_type == ChessPieceType::King)
        {
            if (abs(from.y - to.y)>1) //竖着走了两步 不行
            {
                if (g_theGame->m_chessReferee->m_chessBoard->HasBlockedOnAxial(from, to)) //TODO: 处理王车易位
                {
                    m_currentCoord = from;
                    ChessMoveResult result;
                    result = ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
                    g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                    return;
                }
                m_currentCoord = from;
                ChessMoveResult result;
                result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                return;
            }
            else
            {
                std::vector<ChessPiece*> piecesAround= GetChessPiecesAroundAxially(to);
                bool isThereKing = false;
                for (ChessPiece* piece : piecesAround)
                {
                    if (piece != this && piece->m_type == ChessPieceType::King)
                    {
                        isThereKing = true;
                    }
                }
                if (isThereKing)
                {
                    m_currentCoord = from;
                    ChessMoveResult result;
                    result = ChessMoveResult::INVALID_MOVE_KING_TOGETHER;
                    g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                    return;
                }
                
                // if (!m_hasMoved) //王车易位
                // {
                    if (abs(from.x - to.x) ==2) //横着走了两步
                    {
                        //ChessPiece* myRooks[2]; //可以升变
                        std::vector<ChessPiece*> myRooks;
                        for (ChessPiece* piece : g_theGame->m_chessReferee->m_chessBoard->m_chessPieces)
                        {
                            if (piece->m_type==ChessPieceType::Rook && piece->m_ownerKishiID == m_ownerKishiID)
                            {
                                myRooks.push_back(piece);
                            }
                        }
                        ChessPiece* targetRook = nullptr;
                        int minDist = 999;
                        for (ChessPiece* rook : myRooks)
                        {
                            // if (!rook->m_hasMoved) //为了细分move result，把这个移走
                            // {
                                if (abs(rook->m_currentCoord.x - m_currentCoord.x) >2) //&&
                                //g_theGame->m_chessReferee->m_chessBoard->HasBlockedOnAxial(m_currentCoord, rook->m_currentCoord))
                                {
                                    int ji = (to.x - from.x) * (rook->m_currentCoord.x - m_currentCoord.x);
                                    if (ji >0 && abs(rook->m_currentCoord.x - m_currentCoord.x)<minDist) //它们在同一方向
                                    {
                                        minDist = abs(rook->m_currentCoord.x - m_currentCoord.x);
                                        targetRook = rook;
                                    }
                                }
                            //}
                        }
                        if (!targetRook)
                        {
                            //在这儿还没return的话就是单纯走了2步，不行,王不能这么走所以是wrong shape
                            //二编：如果没找到这个target的话就说明没有易位的需求，不易位时不能走2步
                            m_currentCoord = from;
                            ChessMoveResult result;
                            result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                            g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                            return;
                        }
                        if (m_hasMoved && targetRook)
                        {
                            m_currentCoord = from;
                            ChessMoveResult result;
                            result = ChessMoveResult::INVALID_CASTLE_KING_HAS_MOVED;
                            g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                            return;
                        }
                        if (targetRook && targetRook->m_hasMoved)
                        {
                            m_currentCoord = from;
                            ChessMoveResult result;
                            result = ChessMoveResult::INVALID_CASTLE_ROOK_HAS_MOVED;
                            g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                            return;
                        }
                        if (targetRook && g_theGame->m_chessReferee->m_chessBoard->HasBlockedOnAxial(m_currentCoord, targetRook->m_currentCoord))
                        {
                            m_currentCoord = from;
                            ChessMoveResult result;  
                            result = ChessMoveResult::INVALID_CASTLE_PATH_BLOCKED;
                            g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                            return;
                        }
                        
                        if (from.x-to.x >0) //queenisde
                        {
                            m_hasMoved = true;
                            GetMyKishi()->m_lastMovedPiece = this;

                            ChessMoveResult result;
                            result = ChessMoveResult::VALID_CASTLE_QUEENSIDE;
                            g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                            g_theDevConsole->AddLine(Rgba8::MISTBLUE,
        "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
            +GetMyKishi()->m_colorName+
            ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
            + " to " + std::to_string(to.x) + std::to_string(to.y));
                            // if (toPiece)
                            // {
                            //     g_theDevConsole->AddLine(Rgba8::PEACH,
                            //     "Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                            //         + GetMyKishi()->m_colorName +") captured Player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
                            //         + " ("+ GetAnotherKishi()->m_colorName +")'s " + toPiece->m_definition.m_name + " at " +
                            //         std::to_string(to.x) + std::to_string(to.y));
                            // }
                            fromPiece->m_lerpT = 0.f;
                            g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
                            m_lastCoord = from;
                            m_currentCoord = to;
                            
                            targetRook->m_hasMoved = true;
                            g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(targetRook->m_currentCoord,  IntVec2(from.x -1, from.y));
                            // targetRook->m_lastCoord = targetRook->m_currentCoord;
                            // targetRook->m_currentCoord = IntVec2(from.x -1, from.y);
                            // targetRook->m_lerpT = 0.f;
                            targetRook->ResetMyCoords(targetRook->m_currentCoord, IntVec2(from.x-1, from.y));

                            g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
                            return;
                        }
                        if (from.x-to.x <0) //kingside
                        {
                            m_hasMoved = true;
                            GetMyKishi()->m_lastMovedPiece = this;

                            ChessMoveResult result;
                            result = ChessMoveResult::VALID_CASTLE_KINGSIDE;
                            g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                            g_theDevConsole->AddLine(Rgba8::MISTBLUE,
        "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
            +GetMyKishi()->m_colorName+
            ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
            + " to " + std::to_string(to.x) + std::to_string(to.y));
                            
                            fromPiece->m_lerpT = 0.f;
                            g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
                            m_lastCoord = from;
                            m_currentCoord = to;
                            
                            targetRook->m_hasMoved = true;
                            g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(targetRook->m_currentCoord,  IntVec2(from.x +1, from.y));
                            targetRook->ResetMyCoords(targetRook->m_currentCoord, IntVec2(from.x +1, from.y));
                            //targetRook->m_lastCoord = targetRook->m_currentCoord;
                            //targetRook->m_currentCoord = IntVec2(from.x +1, from.y);
                            //targetRook->m_lerpT = 0.f;

                            g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
                            return;
                        }
                    }
                
                // else
                // {
                    // if (abs(from.x - to.x)>1) //不王车易位但是横着走了两步
                    // {
                    //     m_currentCoord = from;
                    //     ChessMoveResult result;
                    //     result = ChessMoveResult::INVALID_CASTLE_KING_HAS_MOVED; //TODO:?这要区分king has moved还是王车易位失败？
                    //     g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                    //     return;
                    // }
                    else if (abs(from.x - to.x)==1 || abs(from.y - to.y)==1) //不王车易位,横着/竖着走了1步
                    {
                        m_lastCoord = from;
                        g_theDevConsole->AddLine(Rgba8::MISTBLUE,
        "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
            +GetMyKishi()->m_colorName+
            ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
            + " to " + std::to_string(to.x) + std::to_string(to.y));
                        if (toPiece)
                        {
                            ChessMoveResult result;
                            result = ChessMoveResult::VALID_CAPTURE_NORMAL;
                            g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                            
                            g_theDevConsole->AddLine(Rgba8::PEACH,
                            "Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                                + GetMyKishi()->m_colorName +") captured Player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
                                + " ("+ GetAnotherKishi()->m_colorName +")'s " + toPiece->m_definition.m_name + " at " +
                                std::to_string(to.x) + std::to_string(to.y));
                        }
                        else
                        {
                            ChessMoveResult result;
                            result = ChessMoveResult::VALID_MOVE_NORMAL;
                            g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                        }
                        fromPiece->m_lerpT = 0.f;
                        g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
                        m_currentCoord = to;
                        m_hasMoved = true;
                        GetMyKishi()->m_lastMovedPiece = this;

                        g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
                        return;
                    }
                    else //其余移动均为非法
                    {
                        m_currentCoord = from;
                        ChessMoveResult result;
                        result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                        g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                        return;
                    }
                }
            }
        if (m_type == ChessPieceType::Pawn)
        {
            if (abs(from.y - to.y)>2) 
            {
                m_currentCoord = from;
                ChessMoveResult result;
                result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                return;
            }
            if (g_theGame->m_chessReferee->m_chessBoard->HasBlockedOnAxial(from, to))
            {
                m_currentCoord = from;
                ChessMoveResult result;
                result = ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
                g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                return;
            }
            
            if (from.x != to.x) //mei向前 
            {
                m_currentCoord = from;
                ChessMoveResult result;
                result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                g_theDevConsole->AddLine(Rgba8::MISTBLUE, GetMoveResultString(result));
                return;
            }
            if (m_ownerKishiID == 0 && from.y < to.y)
            {
				m_currentCoord = from;
				ChessMoveResult result;
				result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
				g_theDevConsole->AddLine(Rgba8::MISTBLUE, GetMoveResultString(result));
				return;
            }
            if (m_ownerKishiID == 1 && from.y > to.y)
			{
				m_currentCoord = from;
				ChessMoveResult result;
				result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
				g_theDevConsole->AddLine(Rgba8::MISTBLUE, GetMoveResultString(result));
				return;
			}
            else  
            {
                if (g_theGame->m_chessReferee->m_chessBoard->HasBlockedOnAxial(from, to)) //TODO: 处理王车易位
                {
                    m_currentCoord = from;
                    ChessMoveResult result;
                    result = ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
                    g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                    return;
                }
                if (m_hasMoved == false)
                {
                    if (toPiece)
                    {
                        ChessMoveResult result;
                        result = ChessMoveResult::INVALID_MOVE_PAWN_BLOCKED;
                        g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                        return;
                    }
                    
                    if (abs(from.y - to.y) == 2) //初次移动+移动2步
                    {
                        ChessMoveResult result;
                        result = ChessMoveResult::VALID_MOVE_NORMAL;
                        g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                    
                        g_theDevConsole->AddLine(Rgba8::MISTBLUE,
            "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
            +GetMyKishi()->m_colorName+
            ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
            + " to " + std::to_string(to.x) + std::to_string(to.y));

                        m_lerpT = 0.f;
                        m_lastCoord = from;
                        m_currentCoord = to;
                        m_hasMoved = true;
                        GetMyKishi()->m_lastMovedPiece = this;
                        m_hasMoved2Squares = true;

                        g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
                        return;
                    }
                    if (abs(from.y - to.y) == 1) 
                    {
                        if (to.y == 0 || to.y == 7) 
                        {
                            if (promoteType == ChessPieceType::Count)
                            {
                                m_currentCoord = from;
                                ChessMoveResult result;
                                result = ChessMoveResult::INVALID_MOVE_NO_PROMOTION;
                                g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                return;
                            }
                            else
                            {
                                if (promoteType == ChessPieceType::Queen ||
                                    promoteType == ChessPieceType::Rook ||
                                    promoteType == ChessPieceType::Bishop ||
                                    promoteType == ChessPieceType::Knight)
                                {
                                    PromoteTo(promoteType);
                                }
                                else
                                {
                                    m_currentCoord = from;
                                    ChessMoveResult result;
                                    result = ChessMoveResult::INVALID_MOVE_NO_PROMOTION;
                                    g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                    return;
                                }
                            }
                        }
                        
                        ChessMoveResult result;
                        result = ChessMoveResult::VALID_MOVE_NORMAL;
                        g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                    
                        g_theDevConsole->AddLine(Rgba8::MISTBLUE,
            "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
            +GetMyKishi()->m_colorName+
            ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
            + " to " + std::to_string(to.x) + std::to_string(to.y));

                        m_lerpT = 0.f;
                        m_lastCoord = from;
                        m_currentCoord = to;
                        m_hasMoved = true;
                        GetMyKishi()->m_lastMovedPiece = this;
                        m_hasMoved2Squares = false;

                        g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
                        return;
                    }
                    else
                    {
                        m_currentCoord = from;
                        ChessMoveResult result;
                        result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                        g_theDevConsole->AddLine(Rgba8::MISTBLUE, GetMoveResultString(result));
                        return;
                    }
                }
                else
                {
                    if (abs(from.y - to.y) >1)
                    {
                        m_currentCoord = from;
                        ChessMoveResult result;
                        result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                        g_theDevConsole->AddLine(Rgba8::MISTBLUE, GetMoveResultString(result));
                        return;
                    }
                    else if (abs(from.y - to.y) ==1) //正常向前一步
                    {
                        if (toPiece)
                        {
                            ChessMoveResult result;
                            result = ChessMoveResult::INVALID_MOVE_PAWN_BLOCKED;
                            g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                            return;
                        }

                        if (to.y == 0 || to.y == 7) //其实黑方这里只能to==0
                        {
                            if (promoteType == ChessPieceType::Count)
                            {
                                m_currentCoord = from;
                                ChessMoveResult result;
                                result = ChessMoveResult::INVALID_MOVE_NO_PROMOTION;
                                g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                return;
                            }
                            else
                            {
                                if (promoteType == ChessPieceType::Queen ||
                                    promoteType == ChessPieceType::Rook ||
                                    promoteType == ChessPieceType::Bishop ||
                                    promoteType == ChessPieceType::Knight)
                                {
                                    PromoteTo(promoteType);
                                }
                                else
                                {
                                    m_currentCoord = from;
                                    ChessMoveResult result;
                                    result = ChessMoveResult::INVALID_MOVE_NO_PROMOTION;
                                    g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                    return;
                                }
                            }
                        }
                        
                        ChessMoveResult result;
                        result = ChessMoveResult::VALID_MOVE_NORMAL;
                        g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));

                        g_theDevConsole->AddLine(Rgba8::MISTBLUE,
            "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
            +GetMyKishi()->m_colorName+
            ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
            + " to " + std::to_string(to.x) + std::to_string(to.y));
                        // if (toPiece)
                        // {
                        //     ChessMoveResult result;
                        //     result = ChessMoveResult::VALID_CAPTURE_NORMAL;
                        //     g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                        //
                        //     g_theDevConsole->AddLine(Rgba8::PEACH,
                        //     "Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                        //         + GetMyKishi()->m_colorName +") captured Player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
                        //         + " ("+ GetAnotherKishi()->m_colorName +")'s " + toPiece->m_definition.m_name + " at " +
                        //         std::to_string(to.x) + std::to_string(to.y));
                        // }
                        // else
                        // {
                        //     ChessMoveResult result;
                        //     result = ChessMoveResult::VALID_MOVE_NORMAL;
                        //     g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                        // }
                        fromPiece->m_lerpT = 0.f;
                        g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
                        m_lastCoord = from;
                        m_currentCoord = to;
                        m_hasMoved = true;
                        GetMyKishi()->m_lastMovedPiece = this;

                        g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
                        return;
                    }
                }
            }
        }
        if (m_type == ChessPieceType::Rook)
        {
            if (g_theGame->m_chessReferee->m_chessBoard->HasBlockedOnAxial(from, to)) 
            {
                m_currentCoord = from;
                ChessMoveResult result;
                result = ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
                g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                return;
            }
            
            m_lastCoord = from;
            
            g_theDevConsole->AddLine(Rgba8::MISTBLUE,
            "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
            +GetMyKishi()->m_colorName+
            ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
            + " to " + std::to_string(to.x) + std::to_string(to.y));
            if (toPiece)
            {
                ChessMoveResult result;
                result = ChessMoveResult::VALID_CAPTURE_NORMAL;
                g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                
                g_theDevConsole->AddLine(Rgba8::PEACH,
                "Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                    + GetMyKishi()->m_colorName +") captured Player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
                    + " ("+ GetAnotherKishi()->m_colorName +")'s " + toPiece->m_definition.m_name + " at " +
                    std::to_string(to.x) + std::to_string(to.y));
            }
            else
            {
                ChessMoveResult result;
                result = ChessMoveResult::VALID_MOVE_NORMAL;
                g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
            }
            fromPiece->m_lerpT = 0.f;
            g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
            m_currentCoord = to;
            m_hasMoved = true;
            GetMyKishi()->m_lastMovedPiece = this;

            g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
            return;
        }
        else
        {
            m_currentCoord = from;
            ChessMoveResult result;
            result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
            g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
            return;
        }
    }
    if (g_theGame->m_chessReferee->m_chessBoard->IsMovingKnight(from, to))
    {
        if (m_type == ChessPieceType::Knight)
        {
            g_theDevConsole->AddLine(Rgba8::MISTBLUE,
            "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
            +GetMyKishi()->m_colorName+
            ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
            + " to " + std::to_string(to.x) + std::to_string(to.y));
            if (toPiece)
            {
                ChessMoveResult result;
                result = ChessMoveResult::VALID_CAPTURE_NORMAL;
                g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                
                g_theDevConsole->AddLine(Rgba8::PEACH,
                "Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                    + GetMyKishi()->m_colorName +") captured Player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
                    + " ("+ GetAnotherKishi()->m_colorName +")'s " + toPiece->m_definition.m_name + " at " +
                    std::to_string(to.x) + std::to_string(to.y));
            }
            else
            {
                ChessMoveResult result;
                result = ChessMoveResult::VALID_MOVE_NORMAL;
                g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
            }
            fromPiece->m_lerpT = 0.f;
            g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
            m_currentCoord = to;
            m_hasMoved = true;
            GetMyKishi()->m_lastMovedPiece = this;

            g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
            return;
        }
        else
        {
            m_currentCoord = from;
            ChessMoveResult result;
            result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
            g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
            return;
        }
    }
    
    // std::swap(g_theGame->m_chessReferee->m_nextMoveKishiIndex, g_theGame->m_chessReferee->m_currentMoveKishiIndex);
    // g_theGame->m_chessReferee->PrintCurrentPlayerRound();
    // g_theGame->m_chessReferee->PrintBoardStateToDevConsole();
}

void ChessPiece::OnSemiLegalMove(IntVec2 from, IntVec2 to, ChessPieceType newPromoteType)
{
    ResetMyCoords(from, to);

    if (newPromoteType == ChessPieceType::Queen ||
        newPromoteType == ChessPieceType::Bishop ||
        newPromoteType == ChessPieceType::Knight ||
        newPromoteType == ChessPieceType::Rook)
    {
        if (m_type == ChessPieceType::Pawn)
            PromoteTo(newPromoteType);
    }
}

ChessMoveResult ChessPiece::OnRaycastMoveTest(IntVec2 from, IntVec2 to)
{
    ChessPiece* fromPiece = this;
    ChessPiece* toPiece = g_theGame->m_chessReferee->m_chessBoard->GetPiece(to);

    //same
    if (from == to)
    {
        ChessMoveResult result = ChessMoveResult::INVALID_MOVE_ZERO_DISTANCE;
        return result;
    }
    //目标位置棋子是自己的
    if (toPiece &&fromPiece->m_ownerKishiID == toPiece->m_ownerKishiID)
    {
        ChessMoveResult result = ChessMoveResult::INVALID_MOVE_DESTINATION_BLOCKED;
        return result;
    }
    
    if (g_theGame->m_chessReferee->m_chessBoard->IsDiagonal(from, to))
    {
        if (g_theGame->m_chessReferee->m_chessBoard->HasBlockedOnDiagonal(from, to))
        {
            m_currentCoord = from;
            ChessMoveResult result;
            result = ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
            //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
            return result;
        }
        
        if (m_type == ChessPieceType::Bishop)
        {
            m_lastCoord = from;
            g_theDevConsole->AddLine(Rgba8::MISTBLUE,
        "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
            +GetMyKishi()->m_colorName+
            ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
            + " to " + std::to_string(to.x) + std::to_string(to.y));
            if (toPiece)
            {
                ChessMoveResult result;
                result = ChessMoveResult::VALID_CAPTURE_NORMAL;
                //g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));

                // g_theDevConsole->AddLine(Rgba8::PEACH,
                // "Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                //     + GetMyKishi()->m_colorName +") captured Player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
                //     + " ("+ GetAnotherKishi()->m_colorName +")'s " + toPiece->m_definition.m_name + " at " +
                //     std::to_string(to.x) + std::to_string(to.y));
                return result;
                
            }
            else
            {
                ChessMoveResult result;
                result = ChessMoveResult::VALID_MOVE_NORMAL;
                //g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));

                return result;
            }
            // fromPiece->m_lerpT = 0.f;
            // g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
            // m_currentCoord = to;
            // m_hasMoved = true;
            // GetMyKishi()->m_lastMovedPiece = this;
            //
            // g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
            // return;
        }
        if (m_type == ChessPieceType::Queen)
        {
            m_lastCoord = from;
            //     g_theDevConsole->AddLine(Rgba8::MISTBLUE,
            // "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
            //     +GetMyKishi()->m_colorName+
            //     ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
            //     + " to " + std::to_string(to.x) + std::to_string(to.y));
            if (toPiece)
            {
                ChessMoveResult result;
                result = ChessMoveResult::VALID_CAPTURE_NORMAL;
                // g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                //
                // g_theDevConsole->AddLine(Rgba8::PEACH,
                // "Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                //     + GetMyKishi()->m_colorName +") captured Player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
                //     + " ("+ GetAnotherKishi()->m_colorName +")'s " + toPiece->m_definition.m_name + " at " +
                //     std::to_string(to.x) + std::to_string(to.y));
                return result;
            }
            else
            {
                ChessMoveResult result;
                result = ChessMoveResult::VALID_MOVE_NORMAL;
                //g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                
                return result;
            }
            // fromPiece->m_lerpT = 0.f;
            // g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
            // m_currentCoord = to;
            // m_hasMoved = true;
            // GetMyKishi()->m_lastMovedPiece = this;
            //
            // g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
            // return;
        }
        if (m_type == ChessPieceType::King)
        {
            if (abs(from.x - to.x) >1 || abs(from.y == to.y)>1) //斜着走不可能王车易位
            {
                m_currentCoord = from;
                ChessMoveResult result;
                result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                return result;
            }
            std::vector<ChessPiece*> piecesAround= GetChessPiecesAroundAxially(to);
            bool isThereKing = false;
            for (ChessPiece* piece : piecesAround)
            {
                if (piece != this && piece->m_type == ChessPieceType::King)
                {
                    isThereKing = true;
                }
            }
            if (isThereKing)
            {
                m_currentCoord = from;
                ChessMoveResult result;
                result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                return result;
            }
            else
            {
                m_lastCoord = from;
                //         g_theDevConsole->AddLine(Rgba8::MISTBLUE,
                // "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                //     +GetMyKishi()->m_colorName+
                //     ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
                //     + " to " + std::to_string(to.x) + std::to_string(to.y));
                if (toPiece)
                {
                    ChessMoveResult result;
                    result = ChessMoveResult::VALID_CAPTURE_NORMAL;
                    // g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                    //
                    // g_theDevConsole->AddLine(Rgba8::PEACH,
                    // "Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                    //     + GetMyKishi()->m_colorName +") captured Player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
                    //     + " ("+ GetAnotherKishi()->m_colorName +")'s " + toPiece->m_definition.m_name + " at " +
                    //     std::to_string(to.x) + std::to_string(to.y));
                    return result;
                }
                else
                {
                    ChessMoveResult result;
                    result = ChessMoveResult::VALID_MOVE_NORMAL;
                    //g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                    return result;
                }
                // fromPiece->m_lerpT = 0.f;
                // g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
                // m_currentCoord = to;
                // m_hasMoved = true;
                // GetMyKishi()->m_lastMovedPiece = this;
                //
                // g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
                // return;
            }
        }
        if (m_type == ChessPieceType::Pawn) //En passant/吃子 斜着移动
        {
            if (abs(from.x - to.x) ==1 && abs(from.y - to.y)==1)
            {
                if (!toPiece)
                {
                    if (m_ownerKishiID==0) //black
                    {
                        if (from.y > to.y) //moving forward le
                        {
                            ChessPiece* bePassantedP = nullptr;
                            if (from.x > to.x)
                            {
                                bePassantedP = g_theGame->m_chessReferee->m_chessBoard->GetPiece(IntVec2(from.x-1, from.y));
                            }
                            if (from.x < to.x)
                            {
                                bePassantedP = g_theGame->m_chessReferee->m_chessBoard->GetPiece(IntVec2(from.x + 1, from.y));
                            }
                            if (bePassantedP && bePassantedP->m_ownerKishiID != m_ownerKishiID
                                && bePassantedP->m_currentCoord.y==m_currentCoord.y)
                            {
                                if (!bePassantedP->m_hasMoved2Squares)
                                {
                                    m_currentCoord = from;
                                    ChessMoveResult result;
                                    result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                                    //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                    return result;
                                }
                                if (bePassantedP->m_hasMoved2Squares && !(bePassantedP->GetMyKishi()->m_lastMovedPiece == bePassantedP))
                                {
                                    m_currentCoord = from;
                                    ChessMoveResult result;
                                    result = ChessMoveResult::INVALID_ENPASSANT_STALE;
                                    //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                    return result;
                                }
                                if (bePassantedP->m_hasMoved2Squares &&
                                    (bePassantedP->GetMyKishi()->m_lastMovedPiece == bePassantedP) )
                                    //En passant
                                {
                                    if (!(abs(bePassantedP->m_currentCoord.y-bePassantedP->m_lastCoord.y)==2))
                                    {
                                        m_currentCoord = from;
                                        ChessMoveResult result;
                                        result = ChessMoveResult::INVALID_ENPASSANT_STALE;
                                        //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                        return result;
                                    }

                                    ChessMoveResult result;
                                    result = ChessMoveResult::VALID_CAPTURE_ENPASSANT;
                                    // g_theDevConsole->AddLine(Rgba8::BLUE, GetMoveResultString(result));
                                    //                            
                                    //                            g_theDevConsole->AddLine(Rgba8::MISTBLUE,
                                    //                        "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                                    //                            +GetMyKishi()->m_colorName+
                                    //                            ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
                                    //                            + " to " + std::to_string(to.x) + std::to_string(to.y));
                                    //
                                    //                            g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(bePassantedP->m_currentCoord);
                                    //                            
                                    //                            ResetMyCoords(from, to);
                                    //                            m_hasMoved = true;
                                    //                            GetMyKishi()->m_lastMovedPiece = this;
                                    
                                    return result;
                                }
                            }
                            else
                            {
                                m_currentCoord = from;
                                ChessMoveResult result;
                                result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                                //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                return result;
                            }
                        }
                    }
                    if (m_ownerKishiID==1) //white
                    {
                        if (from.y < to.y) //moving forward le
                        {
                            ChessPiece* bePassantedP = nullptr;
                            if (from.x > to.x)
                            {
                                bePassantedP = g_theGame->m_chessReferee->m_chessBoard->GetPiece(IntVec2(from.x - 1, from.y));
                            }
                            if (from.x < to.x)
                            {
                                bePassantedP = g_theGame->m_chessReferee->m_chessBoard->GetPiece(IntVec2(from.x + 1, from.y));
                            }
                            if (bePassantedP && bePassantedP->m_ownerKishiID != m_ownerKishiID
                                && bePassantedP->m_currentCoord.y==m_currentCoord.y)
                            {
                                if (!bePassantedP->m_hasMoved2Squares)
                                {
                                    m_currentCoord = from;
                                    ChessMoveResult result;
                                    result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                                    //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                    return result;
                                }
                                if (bePassantedP->m_hasMoved2Squares && !(bePassantedP->GetMyKishi()->m_lastMovedPiece == bePassantedP))
                                {
                                    m_currentCoord = from;
                                    ChessMoveResult result;
                                    result = ChessMoveResult::INVALID_ENPASSANT_STALE;
                                    //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                    return result;
                                }
                                if (bePassantedP->m_hasMoved2Squares &&
                                    (bePassantedP->GetMyKishi()->m_lastMovedPiece == bePassantedP) )
                                    //En passant
                                {
                                    if (!(abs(bePassantedP->m_currentCoord.y-bePassantedP->m_lastCoord.y)==2))
                                    {
                                        m_currentCoord = from;
                                        ChessMoveResult result;
                                        result = ChessMoveResult::INVALID_ENPASSANT_STALE;
                                        //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                        return result;
                                    }

									ChessMoveResult result;
									result = ChessMoveResult::VALID_CAPTURE_ENPASSANT;
                                    
                                    // ChessMoveResult result;
                                    // result = ChessMoveResult::VALID_CAPTURE_ENPASSANT;
                                    // g_theDevConsole->AddLine(Rgba8::BLUE, GetMoveResultString(result));
                                    //
                                    //                            g_theDevConsole->AddLine(Rgba8::MISTBLUE,
                                    //                        "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                                    //                            +GetMyKishi()->m_colorName+
                                    //                            ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
                                    //                            + " to " + std::to_string(to.x) + std::to_string(to.y));
                                    //
                                    //                            g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(bePassantedP->m_currentCoord);
                                    //                            
                                    //                            ResetMyCoords(from, to);
                                    //                            m_hasMoved = true;
                                    //                            GetMyKishi()->m_lastMovedPiece = this;
                                    //                          return;
                                }
                            }
                            else
                            {
                                m_currentCoord = from;
                                ChessMoveResult result;
                                result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                                //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                return result;
                            }
                        }
                    }
                }
                else
                {
                    if (m_ownerKishiID==0) //black
                    {
                        if (from.y > to.y) //moving forward le
                        {
                            if (toPiece) //要吃子
                            {
                                if (to.y == 0 || to.y == 7) //其实黑方这里只能to==0
                                {
                                    // if (promoteType == ChessPieceType::Count)
                                    // {
                                    //     m_currentCoord = from;
                                    //     ChessMoveResult result;
                                    //     result = ChessMoveResult::INVALID_MOVE_NO_PROMOTION;
                                    //     g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                    //     return;
                                    // }
                                    // else
                                    // {
                                    //     if (promoteType == ChessPieceType::Queen ||
                                    //         promoteType == ChessPieceType::Rook ||
                                    //         promoteType == ChessPieceType::Bishop ||
                                    //         promoteType == ChessPieceType::Knight)
                                    //     {
                                    //         PromoteTo(promoteType);
                                    //     }
                                    //     else
                                    //     {
                                    //         m_currentCoord = from;
                                    //         ChessMoveResult result;
                                    //         result = ChessMoveResult::INVALID_MOVE_NO_PROMOTION;
                                    //         g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                    //         return;
                                    //     }
                                }
                            }
                                
                            //                         g_theDevConsole->AddLine(Rgba8::MISTBLUE,
                            // "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                            //     +GetMyKishi()->m_colorName+
                            //     ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
                            //     + " to " + std::to_string(to.x) + std::to_string(to.y));
                                
                            ChessMoveResult result;
                            result = ChessMoveResult::VALID_CAPTURE_NORMAL;
                            // g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                            //
                            // g_theDevConsole->AddLine(Rgba8::PEACH,
                            // "Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                            //     + GetMyKishi()->m_colorName +") captured Player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
                            //     + " ("+ GetAnotherKishi()->m_colorName +")'s " + toPiece->m_definition.m_name + " at " +
                            //     std::to_string(to.x) + std::to_string(to.y));
                            return result;
                                
                            // fromPiece->m_lerpT = 0.f;
                            // g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
                            // m_lastCoord = from;
                            // m_currentCoord = to;
                            // m_hasMoved = true;
                            // GetMyKishi()->m_lastMovedPiece = this;
                            //
                            // g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
                            // return;
                        }
                        else
                        {
                            m_currentCoord = from;
                            ChessMoveResult result;
                            result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                            //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                            return result;
                        }
                    }

                    if (m_ownerKishiID==1)
                    {
                        if (from.y < to.y) //moving forward le
                    {
                        if (toPiece) //要吃子
                        {
                            if (to.y == 0 || to.y == 7) //其实黑方这里只能to==0
                            {
                                // if (promoteType == ChessPieceType::Count)
                                // {
                                //     m_currentCoord = from;
                                //     ChessMoveResult result;
                                //     result = ChessMoveResult::INVALID_MOVE_NO_PROMOTION;
                                //     g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                //     return;
                                // }
                                // else
                                // {
                                //     if (promoteType == ChessPieceType::Queen ||
                                //         promoteType == ChessPieceType::Rook ||
                                //         promoteType == ChessPieceType::Bishop ||
                                //         promoteType == ChessPieceType::Knight)
                                //     {
                                //         PromoteTo(promoteType);
                                //     }
                                //     else
                                //     {
                                //         m_currentCoord = from;
                                //         ChessMoveResult result;
                                //         result = ChessMoveResult::INVALID_MOVE_NO_PROMOTION;
                                //         g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                                //         return;
                                //     }
                                // }
                            }
                                
                            //                         g_theDevConsole->AddLine(Rgba8::MISTBLUE,
                            // "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                            //     +GetMyKishi()->m_colorName+
                            //     ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
                            //     + " to " + std::to_string(to.x) + std::to_string(to.y));
                                
                            ChessMoveResult result;
                            result = ChessMoveResult::VALID_CAPTURE_NORMAL;
                                
                            return result;
                                
                            // g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                            //
                            // g_theDevConsole->AddLine(Rgba8::PEACH,
                            // "Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                            //     + GetMyKishi()->m_colorName +") captured Player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
                            //     + " ("+ GetAnotherKishi()->m_colorName +")'s " + toPiece->m_definition.m_name + " at " +
                            //     std::to_string(to.x) + std::to_string(to.y));
                                
                            // fromPiece->m_lerpT = 0.f;
                            // g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
                            // m_lastCoord = from;
                            // m_currentCoord = to;
                            // m_hasMoved = true;
                            // GetMyKishi()->m_lastMovedPiece = this;
                            //
                            // g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
                            // return;
                        }
                        else
                        {
                            m_currentCoord = from;
                            ChessMoveResult result;
                            result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                            //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                            return result;
                        }
                    }
                        else
                        {
                            m_currentCoord = from;
                            ChessMoveResult result;
                            result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                            //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                            return result;
                        }
                    }
                }
            }
        }
        else
        {
            m_currentCoord = from;
            ChessMoveResult result;
            result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
            //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
            return result;
        }
    }

    if (g_theGame->m_chessReferee->m_chessBoard->IsAxial(from, to))
    {
        if (m_type == ChessPieceType::Queen)
        {
            if (g_theGame->m_chessReferee->m_chessBoard->HasBlockedOnAxial(from, to)) //TODO: 处理王车易位
            {
                m_currentCoord = from;
                ChessMoveResult result;
                result = ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
                //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                return result;
            }
        //     g_theDevConsole->AddLine(Rgba8::MISTBLUE,
        // "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
        //     +GetMyKishi()->m_colorName+
        //     ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
        //     + " to " + std::to_string(to.x) + std::to_string(to.y));
            if (toPiece)
            {
                ChessMoveResult result;
                result = ChessMoveResult::VALID_CAPTURE_NORMAL;
                // g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                //
                // g_theDevConsole->AddLine(Rgba8::PEACH,
                // "Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                //     + GetMyKishi()->m_colorName +") captured Player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
                //     + " ("+ GetAnotherKishi()->m_colorName +")'s " + toPiece->m_definition.m_name + " at " +
                //     std::to_string(to.x) + std::to_string(to.y));
                return result;
            }
            else
            {
                ChessMoveResult result;
                result = ChessMoveResult::VALID_MOVE_NORMAL;
                //g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                return result;
            }
            // fromPiece->m_lerpT = 0.f;
            // g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
            // m_lastCoord = from;
            // m_currentCoord = to;
            // m_hasMoved = true;
            // GetMyKishi()->m_lastMovedPiece = this;
            //
            // g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
            // return;
        }
        if (m_type == ChessPieceType::King)
        {
            if (abs(from.y - to.y)>1) //竖着走了两步 不行
            {
                if (g_theGame->m_chessReferee->m_chessBoard->HasBlockedOnAxial(from, to)) //TODO: 处理王车易位
                {
                    m_currentCoord = from;
                    ChessMoveResult result;
                    result = ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
                    //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                    return result;
                }
                m_currentCoord = from;
                ChessMoveResult result;
                result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                return result;
            }
            else
            {
                std::vector<ChessPiece*> piecesAround= GetChessPiecesAroundAxially(to);
                bool isThereKing = false;
                for (ChessPiece* piece : piecesAround)
                {
                    if (piece!=this && piece->m_type == ChessPieceType::King)
                    {
                        isThereKing = true;
                    }
                }
                if (isThereKing)
                {
                    m_currentCoord = from;
                    ChessMoveResult result;
                    result = ChessMoveResult::INVALID_MOVE_KING_TOGETHER;
                    //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                    return result;
                }
                
                // if (!m_hasMoved) //王车易位
                // {
                    if (abs(from.x - to.x) ==2) //横着走了两步
                    {
                        //ChessPiece* myRooks[2]; //可以升变
                        std::vector<ChessPiece*> myRooks;
                        for (ChessPiece* piece : g_theGame->m_chessReferee->m_chessBoard->m_chessPieces)
                        {
                            if (piece->m_type==ChessPieceType::Rook && piece->m_ownerKishiID == m_ownerKishiID)
                            {
                                myRooks.push_back(piece);
                            }
                        }
                        ChessPiece* targetRook = nullptr;
                        int minDist = 999;
                        for (ChessPiece* rook : myRooks)
                        {
                            // if (!rook->m_hasMoved) //为了细分move result，把这个移走
                            // {
                                if (abs(rook->m_currentCoord.x - m_currentCoord.x) >2) //&&
                                //g_theGame->m_chessReferee->m_chessBoard->HasBlockedOnAxial(m_currentCoord, rook->m_currentCoord))
                                {
                                    int ji = (to.x - from.x) * (rook->m_currentCoord.x - m_currentCoord.x);
                                    if (ji >0 && abs(rook->m_currentCoord.x - m_currentCoord.x)<minDist) //它们在同一方向
                                    {
                                        minDist = abs(rook->m_currentCoord.x - m_currentCoord.x);
                                        targetRook = rook;
                                    }
                                }
                            //}
                        }
                        if (!targetRook)
                        {
                            //在这儿还没return的话就是单纯走了2步，不行,王不能这么走所以是wrong shape
                            //二编：如果没找到这个target的话就说明没有易位的需求，不易位时不能走2步
                            m_currentCoord = from;
                            ChessMoveResult result;
                            result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                            //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                            return result;
                        }
                        if (m_hasMoved && targetRook)
                        {
                            m_currentCoord = from;
                            ChessMoveResult result;
                            result = ChessMoveResult::INVALID_CASTLE_KING_HAS_MOVED;
                            //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                            return result;
                        }
                        if (targetRook && targetRook->m_hasMoved)
                        {
                            m_currentCoord = from;
                            ChessMoveResult result;
                            result = ChessMoveResult::INVALID_CASTLE_ROOK_HAS_MOVED;
                            //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                            return result;
                        }
                        if (targetRook && g_theGame->m_chessReferee->m_chessBoard->HasBlockedOnAxial(m_currentCoord, targetRook->m_currentCoord))
                        {
                            m_currentCoord = from;
                            ChessMoveResult result;  
                            result = ChessMoveResult::INVALID_CASTLE_PATH_BLOCKED;
                            //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                            return result;
                        }
                        
                        if (from.x-to.x >0) //queenisde
                        {
                            ChessMoveResult result;
                            result = ChessMoveResult::VALID_CASTLE_QUEENSIDE;

        //                     m_hasMoved = true;
        //                     GetMyKishi()->m_lastMovedPiece = this;
        //                     g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
        //                     g_theDevConsole->AddLine(Rgba8::MISTBLUE,
        // "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
        //     +GetMyKishi()->m_colorName+
        //     ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
        //     + " to " + std::to_string(to.x) + std::to_string(to.y));
        //                     
        //                     fromPiece->m_lerpT = 0.f;
        //                     g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
        //                     m_lastCoord = from;
        //                     m_currentCoord = to;
        //                     
        //                     targetRook->m_hasMoved = true;
        //                     g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(targetRook->m_currentCoord,  IntVec2(from.x -1, from.y));
        //                     
        //                     targetRook->ResetMyCoords(targetRook->m_currentCoord, IntVec2(from.x-1, from.y));
        //
        //                     g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
                            return result;
                        }
                        if (from.x-to.x <0) //kingside
                        {
                            ChessMoveResult result;
                            result = ChessMoveResult::VALID_CASTLE_KINGSIDE;
                            
        //                     m_hasMoved = true;
        //                     GetMyKishi()->m_lastMovedPiece = this;
        //                     g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
        //                     g_theDevConsole->AddLine(Rgba8::MISTBLUE,
        // "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
        //     +GetMyKishi()->m_colorName+
        //     ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
        //     + " to " + std::to_string(to.x) + std::to_string(to.y));
        //                     
        //                     fromPiece->m_lerpT = 0.f;
        //                     g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
        //                     m_lastCoord = from;
        //                     m_currentCoord = to;
        //                     
        //                     targetRook->m_hasMoved = true;
        //                     g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(targetRook->m_currentCoord,  IntVec2(from.x +1, from.y));
        //                     targetRook->ResetMyCoords(targetRook->m_currentCoord, IntVec2(from.x +1, from.y));
        //
        //                     g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
                            return result;
                        }
                    }
                
                    else if (abs(from.x - to.x)==1 || abs(from.y - to.y)==1) //不王车易位,横着/竖着走了1步
                    {
        //                 m_lastCoord = from;
        //                 g_theDevConsole->AddLine(Rgba8::MISTBLUE,
        // "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
        //     +GetMyKishi()->m_colorName+
        //     ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
        //     + " to " + std::to_string(to.x) + std::to_string(to.y));
                        if (toPiece)
                        {
                            ChessMoveResult result;
                            result = ChessMoveResult::VALID_CAPTURE_NORMAL;
                            // g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                            //
                            // g_theDevConsole->AddLine(Rgba8::PEACH,
                            // "Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                            //     + GetMyKishi()->m_colorName +") captured Player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
                            //     + " ("+ GetAnotherKishi()->m_colorName +")'s " + toPiece->m_definition.m_name + " at " +
                            //     std::to_string(to.x) + std::to_string(to.y));
                            return result;
                        }
                        else
                        {
                            ChessMoveResult result;
                            result = ChessMoveResult::VALID_MOVE_NORMAL;
                            //g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                            return result;
                        }
                        // fromPiece->m_lerpT = 0.f;
                        // g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
                        // m_currentCoord = to;
                        // m_hasMoved = true;
                        // GetMyKishi()->m_lastMovedPiece = this;
                        //
                        // g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
                        // return;
                    }
                    else //其余移动均为非法
                    {
                        m_currentCoord = from;
                        ChessMoveResult result;
                        result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                        //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                        return result;
                    }
                }
            }
        if (m_type == ChessPieceType::Pawn)
        {
            if (abs(from.y - to.y)>2) 
            {
                m_currentCoord = from;
                ChessMoveResult result;
                result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                return result;
            }
            if (g_theGame->m_chessReferee->m_chessBoard->HasBlockedOnAxial(from, to))
            {
                m_currentCoord = from;
                ChessMoveResult result;
                result = ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
                //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                return result;
            }
            
            if (from.x != to.x) //mei向前 
            {
                m_currentCoord = from;
                ChessMoveResult result;
                result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                //g_theDevConsole->AddLine(Rgba8::MISTBLUE, GetMoveResultString(result));
                return result;
            }
			if (m_ownerKishiID == 0 && from.y < to.y)
			{
				m_currentCoord = from;
				ChessMoveResult result;
				result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
				g_theDevConsole->AddLine(Rgba8::MISTBLUE, GetMoveResultString(result));
				return result;
			}
			if (m_ownerKishiID == 1 && from.y > to.y)
			{
				m_currentCoord = from;
				ChessMoveResult result;
				result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
				g_theDevConsole->AddLine(Rgba8::MISTBLUE, GetMoveResultString(result));
				return result;
			}
            else  
            {
				if (to.y == 7 || to.y == 0 && !toPiece)
				{
                    if(g_theGame->m_chessReferee->m_currentMoveKishiIndex == 1 && to.y>from.y)
					    return ChessMoveResult::VALID_MOVE_PROMOTION;
                    if (g_theGame->m_chessReferee->m_currentMoveKishiIndex == 0 && to.y < from.y)
                        return ChessMoveResult::VALID_MOVE_PROMOTION;
                    else
                        return ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
				}

                if (g_theGame->m_chessReferee->m_chessBoard->HasBlockedOnAxial(from, to))
                {
                    m_currentCoord = from;
                    ChessMoveResult result;
                    result = ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
                    //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                    return result;
                }
                if (m_hasMoved == false)
                {
                    if (toPiece)
                    {
                        ChessMoveResult result;
                        result = ChessMoveResult::INVALID_MOVE_PAWN_BLOCKED;
                        //g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                        return result;
                    }
                    
                    if (abs(from.y - to.y) == 2) //初次移动+移动2步
                    {
                        ChessMoveResult result;
                        result = ChessMoveResult::VALID_MOVE_PAWN_2SQUARE;
                        return result;
            //             g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
            //         
            //             g_theDevConsole->AddLine(Rgba8::MISTBLUE,
            // "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
            // +GetMyKishi()->m_colorName+
            // ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
            // + " to " + std::to_string(to.x) + std::to_string(to.y));
            //
            //             m_lerpT = 0.f;
            //             m_lastCoord = from;
            //             m_currentCoord = to;
            //             m_hasMoved = true;
            //             GetMyKishi()->m_lastMovedPiece = this;
            //             m_hasMoved2Squares = true;
            //
            //             g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
            //             return;
                    }
                    if (abs(from.y - to.y) == 1) 
                    {
                        if (to.y == 0 || to.y == 7) 
                        {
                            // if (promoteType == ChessPieceType::Count)
                            // {
                            //     m_currentCoord = from;
                            //     ChessMoveResult result;
                            //     result = ChessMoveResult::INVALID_MOVE_NO_PROMOTION;
                            //     g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                            //     return;
                            // }
                            // else
                            // {
                            //     if (promoteType == ChessPieceType::Queen ||
                            //         promoteType == ChessPieceType::Rook ||
                            //         promoteType == ChessPieceType::Bishop ||
                            //         promoteType == ChessPieceType::Knight)
                            //     {
                            //         PromoteTo(promoteType);
                            //     }
                            //     else
                            //     {
                            //         m_currentCoord = from;
                            //         ChessMoveResult result;
                            //         result = ChessMoveResult::INVALID_MOVE_NO_PROMOTION;
                            //         g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                            //         return;
                            //     }
                            // }
                        }
                        
                        ChessMoveResult result;
                        result = ChessMoveResult::VALID_MOVE_NORMAL;
                        return result;
            //             g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
            //         
            //             g_theDevConsole->AddLine(Rgba8::MISTBLUE,
            // "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
            // +GetMyKishi()->m_colorName+
            // ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
            // + " to " + std::to_string(to.x) + std::to_string(to.y));
            //
            //             m_lerpT = 0.f;
            //             m_lastCoord = from;
            //             m_currentCoord = to;
            //             m_hasMoved = true;
            //             GetMyKishi()->m_lastMovedPiece = this;
            //             m_hasMoved2Squares = false;
            //
            //             g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
            //             return;
                    }
                    else
                    {
                        m_currentCoord = from;
                        ChessMoveResult result;
                        result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                        //g_theDevConsole->AddLine(Rgba8::MISTBLUE, GetMoveResultString(result));
                        return result;
                    }
                }
                else
                {
                    if (abs(from.y - to.y) >1)
                    {
                        m_currentCoord = from;
                        ChessMoveResult result;
                        result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
                        //g_theDevConsole->AddLine(Rgba8::MISTBLUE, GetMoveResultString(result));
                        return result;
                    }
                    else if (abs(from.y - to.y) ==1) //正常向前一步
                    {
                        if (toPiece)
                        {
                            ChessMoveResult result;
                            result = ChessMoveResult::INVALID_MOVE_PAWN_BLOCKED;
                            //g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                            return result;
                        }

                        if (to.y == 0 || to.y == 7) //其实黑方这里只能to==0
                        {
                            // if (promoteType == ChessPieceType::Count)
                            // {
                            //     m_currentCoord = from;
                            //     ChessMoveResult result;
                            //     result = ChessMoveResult::INVALID_MOVE_NO_PROMOTION;
                            //     g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                            //     return;
                            // }
                            // else
                            // {
                            //     if (promoteType == ChessPieceType::Queen ||
                            //         promoteType == ChessPieceType::Rook ||
                            //         promoteType == ChessPieceType::Bishop ||
                            //         promoteType == ChessPieceType::Knight)
                            //     {
                            //         PromoteTo(promoteType);
                            //     }
                            //     else
                            //     {
                            //         m_currentCoord = from;
                            //         ChessMoveResult result;
                            //         result = ChessMoveResult::INVALID_MOVE_NO_PROMOTION;
                            //         g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                            //         return;
                            //     }
                            // }
                        }
                        
                        ChessMoveResult result;
                        result = ChessMoveResult::VALID_MOVE_NORMAL;
                        //g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                        return result;
                        
            //             g_theDevConsole->AddLine(Rgba8::MISTBLUE,
            // "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
            // +GetMyKishi()->m_colorName+
            // ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
            // + " to " + std::to_string(to.x) + std::to_string(to.y));
            //             
            //             fromPiece->m_lerpT = 0.f;
            //             g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
            //             m_lastCoord = from;
            //             m_currentCoord = to;
            //             m_hasMoved = true;
            //             GetMyKishi()->m_lastMovedPiece = this;
            //
            //             g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
            //             return;
                    }
                }
            }
        }
        if (m_type == ChessPieceType::Rook)
        {
            if (g_theGame->m_chessReferee->m_chessBoard->HasBlockedOnAxial(from, to)) 
            {
                m_currentCoord = from;
                ChessMoveResult result;
                result = ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
                //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
                return result;
            }
            
            // m_lastCoord = from;
            //
            // g_theDevConsole->AddLine(Rgba8::MISTBLUE,
            // "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
            // +GetMyKishi()->m_colorName+
            // ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
            // + " to " + std::to_string(to.x) + std::to_string(to.y));
            if (toPiece)
            {
                ChessMoveResult result;
                result = ChessMoveResult::VALID_CAPTURE_NORMAL;
                // g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                //
                // g_theDevConsole->AddLine(Rgba8::PEACH,
                // "Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                //     + GetMyKishi()->m_colorName +") captured Player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
                //     + " ("+ GetAnotherKishi()->m_colorName +")'s " + toPiece->m_definition.m_name + " at " +
                //     std::to_string(to.x) + std::to_string(to.y));

                return result;
            }
            else
            {
                ChessMoveResult result;
                result = ChessMoveResult::VALID_MOVE_NORMAL;
                //g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                return result;
            }
            // fromPiece->m_lerpT = 0.f;
            // g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
            // m_currentCoord = to;
            // m_hasMoved = true;
            // GetMyKishi()->m_lastMovedPiece = this;
            //
            // g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
            // return;
        }
        else
        {
            m_currentCoord = from;
            ChessMoveResult result;
            result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
            //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
            return result;
        }
    }
    if (g_theGame->m_chessReferee->m_chessBoard->IsMovingKnight(from, to))
    {
        if (m_type == ChessPieceType::Knight)
        {
            // g_theDevConsole->AddLine(Rgba8::MISTBLUE,
            // "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
            // +GetMyKishi()->m_colorName+
            // ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
            // + " to " + std::to_string(to.x) + std::to_string(to.y));
            if (toPiece)
            {
                ChessMoveResult result;
                result = ChessMoveResult::VALID_CAPTURE_NORMAL;
                // g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                //
                // g_theDevConsole->AddLine(Rgba8::PEACH,
                // "Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                //     + GetMyKishi()->m_colorName +") captured Player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
                //     + " ("+ GetAnotherKishi()->m_colorName +")'s " + toPiece->m_definition.m_name + " at " +
                //     std::to_string(to.x) + std::to_string(to.y));

                return result;
            }
            else
            {
                ChessMoveResult result;
                result = ChessMoveResult::VALID_MOVE_NORMAL;
                //g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
                return result;
            }
            // fromPiece->m_lerpT = 0.f;
            // g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
            // m_currentCoord = to;
            // m_hasMoved = true;
            // GetMyKishi()->m_lastMovedPiece = this;
            //
            // g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
            // return;
        }
        else
        {
            m_currentCoord = from;
            ChessMoveResult result;
            result = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
            //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
            return result;
        }
    }

    return ChessMoveResult::UNKNOWN;
}

ChessMoveResult ChessPiece::OnRaycastSemiMoveTest(IntVec2 from, IntVec2 to)
{
    ChessPiece* toPiece = g_theGame->m_chessReferee->m_chessBoard->GetPiece(to);
    if (toPiece)
    {
        return ChessMoveResult::VALID_CAPTURE_NORMAL;
    }
    return ChessMoveResult::VALID_MOVE_NORMAL;
}

void ChessPiece::OnRaycastValidMove(IntVec2 from, IntVec2 to, ChessMoveResult result)
{
    g_theDevConsole->AddLine(Rgba8::MINTGREEN, GetMoveResultString(result));
    g_theGame->m_chessReferee->m_currentSetFromCoord = from;
    g_theGame->m_chessReferee->m_currentSetToCoord = to;

    if (result != ChessMoveResult::VALID_MOVE_PROMOTION)
    {
        EventArgs args;
        args.SetValue("from", std::string{ static_cast<char>('a' + from.x), static_cast<char>('1' + from.y) });
        args.SetValue("to",   std::string{ static_cast<char>('a' + to.x),   static_cast<char>('1' + to.y) });

        g_theEventSystem->FireEvent("chessmove", args);
    }
    else
    {
        g_theGame->m_promoteWidget->SetEnabled(true);
        g_theGame->m_gameClock->TogglePause();
    }
    
 //    ChessPiece* fromPiece = g_theGame->m_chessReferee->m_chessBoard->GetPiece(from);
 //    ChessPiece* toPiece = g_theGame->m_chessReferee->m_chessBoard->GetPiece(to);
 //    
 //    if (result == ChessMoveResult::VALID_MOVE_NORMAL || result == ChessMoveResult::VALID_CAPTURE_NORMAL)
 //    {
 //        if (result == ChessMoveResult::VALID_MOVE_NORMAL)
 //        {
 //            g_theDevConsole->AddLine(Rgba8::MISTBLUE,
 //            "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
 //            +GetMyKishi()->m_colorName+
 //            ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
 //            + " to " + std::to_string(to.x) + std::to_string(to.y));
 //        }
 //        if (result == ChessMoveResult::VALID_CAPTURE_NORMAL)
 //        {
 //            g_theDevConsole->AddLine(Rgba8::PEACH,
 //            "Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
 //            + GetMyKishi()->m_colorName +") captured Player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
 //            + " ("+ GetAnotherKishi()->m_colorName +")'s " + toPiece->m_definition.m_name + " at " +
 //            std::to_string(to.x) + std::to_string(to.y));
 //        }
 //
	// 	fromPiece->m_lerpT = 0.f;
	// 	g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
 //        m_lastCoord = from;
	// 	m_currentCoord = to;
	// 	m_hasMoved = true;
	// 	GetMyKishi()->m_lastMovedPiece = this;
 //
	// 	g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
 //        return;
 //    }
 //    if (result == ChessMoveResult::VALID_MOVE_PAWN_2SQUARE)
 //    {
 //        m_hasMoved2Squares = true;
 //    }
 //    if (result == ChessMoveResult::VALID_CASTLE_KINGSIDE)
 //    {
 //        ChessPiece* targetRook;
 //        if (m_ownerKishiID == 0) //hei
 //        {
 //            targetRook = g_theGame->m_chessReferee->m_chessBoard->GetPiece(IntVec2(7,7));
 //            targetRook->m_hasMoved = true;
 //            targetRook->ResetMyCoords(from, IntVec2(5,7));
 //        }
 //        else
 //        {
 //            targetRook = g_theGame->m_chessReferee->m_chessBoard->GetPiece(IntVec2(7,0));
 //            targetRook->m_hasMoved = true;
 //            targetRook->ResetMyCoords(from, IntVec2(5,0));
 //        }
 //    }
 //    if (result == ChessMoveResult::VALID_CASTLE_QUEENSIDE)
 //    {
 //        ChessPiece* targetRook;
 //        if (m_ownerKishiID == 0) //hei
 //        {
 //            targetRook = g_theGame->m_chessReferee->m_chessBoard->GetPiece(IntVec2(0,7));
 //            targetRook->m_hasMoved = true;
 //            targetRook->ResetMyCoords(from, IntVec2(3,7));
 //        }
 //        else
 //        {
 //            targetRook = g_theGame->m_chessReferee->m_chessBoard->GetPiece(IntVec2(0,0));
 //            targetRook->m_hasMoved = true;
 //            targetRook->ResetMyCoords(from, IntVec2(3,0));
 //        }
 //    }
 //    if (result == ChessMoveResult::VALID_CAPTURE_ENPASSANT)
 //    {
 //        if (g_theGame->m_chessReferee->m_currentMoveKishiIndex == 0)
 //        {
 //            g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, IntVec2(to.x, to.y + 1));
 //        }
	// 	if (g_theGame->m_chessReferee->m_currentMoveKishiIndex == 1)
	// 	{
	// 		g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, IntVec2(to.x, to.y - 1));
	// 	}
 //    }
 //
	// fromPiece->m_lerpT = 0.f;
 //    m_lastCoord = from;
	// m_currentCoord = to;
	// m_hasMoved = true;
	// GetMyKishi()->m_lastMovedPiece = this;
	// g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);

    // g_theDevConsole->AddLine(Rgba8::MISTBLUE,
    //         "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
    //         +GetMyKishi()->m_colorName+
    //         ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
    //         + " to " + std::to_string(to.x) + std::to_string(to.y));
    //
    // g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
    return;
}

void ChessPiece::OnRaycastSemiMove(IntVec2 from, IntVec2 to, ChessMoveResult result)
{
    ChessPiece* fromPiece = g_theGame->m_chessReferee->m_chessBoard->GetPiece(from);
    ChessPiece* toPiece = g_theGame->m_chessReferee->m_chessBoard->GetPiece(to);

    if (result == ChessMoveResult::VALID_MOVE_NORMAL)
    {
        g_theDevConsole->AddLine(Rgba8::MISTBLUE,
        "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
        +GetMyKishi()->m_colorName+
        ")'s " + fromPiece->m_definition.m_name + " from " + std::to_string(from.x) + std::to_string(from.y)
        + " to " + std::to_string(to.x) + std::to_string(to.y));
    }
    if (result == ChessMoveResult::VALID_CAPTURE_NORMAL)
    {
        g_theDevConsole->AddLine(Rgba8::PEACH,
        "Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
        + GetMyKishi()->m_colorName +") captured Player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
        + " ("+ GetAnotherKishi()->m_colorName +")'s " + toPiece->m_definition.m_name + " at " +
        std::to_string(to.x) + std::to_string(to.y));
    }
	fromPiece->m_lerpT = 0.f;
	g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(from, to);
    m_lastCoord = from;
	m_currentCoord = to;
	m_hasMoved = true;
	GetMyKishi()->m_lastMovedPiece = this;

    g_theGame->m_chessReferee->SwapAndPrintBoardStatesAndRound();
}

std::vector<ChessPiece*> ChessPiece::GetChessPiecesAroundAxially(IntVec2 pos)
{
    std::vector<ChessPiece*> result;
    ChessBoard* board = g_theGame->m_chessReferee->m_chessBoard;

    IntVec2 directions[4] = {
        IntVec2(0, 1),   
        IntVec2(0, -1),  
        IntVec2(-1, 0),  
        IntVec2(1, 0)    
    };

    for (const IntVec2& dir : directions)
    {
        IntVec2 neighbor = pos + dir;
        ChessPiece* piece = board->GetPiece(neighbor);
        if (piece != nullptr)
        {
            result.push_back(piece);
        }
    }
    return result;
}

void ChessPiece::ResetMyCoords(IntVec2 last, IntVec2 current)
{
    m_lastCoord = last;
    m_currentCoord = current;
    m_lerpT = 0.f;
}

void ChessPiece::InitializeDebugDraw()
{
    AddVertsForCylinderZWireframe3D(m_debugVertices, Vec2(),
        FloatRange(0.f, 1.0f), 0.35f, 8, 0.01f);
}

bool ChessPiece::PromoteTo(ChessPieceType type)
{
    m_type = type;
    m_definition = ChessPieceDefinition::GetChessPieceDefinitionByChessPieceType(type);
    return true;
}
