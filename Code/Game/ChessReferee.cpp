#include "ChessReferee.h"

#include <complex>

#include "Game.hpp"
#include "App.hpp"
#include "Player.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/FloatRange.hpp"

ChessReferee::ChessReferee(ChessKishi* chessKishi[2])
    : m_chessKishi(chessKishi)
{
    if (!g_theGame->m_isRemote)
    {
        m_myKishi = chessKishi[0];
        m_currentState = MatchState::PLAYING;
        m_status = ChessStatus::KISHI;
        g_theDevConsole->AddLine(Rgba8::PEACH, "The match state is: Playing. The match has started.");
    }
    else
    {
        m_currentState = MatchState::CONNECTING;
        g_theDevConsole->AddLine(Rgba8::PEACH, "The match state is: Connecting. Try to connect to a server/client to play.");
    }
    
    RegisterForEvents();
    InitializeTheBoard();
    InitializeLights();
    
    /*PrintCurrentPlayerRound();
    PrintBoardStateToDevConsole();*/

    m_ghostPiece = new ChessPiece(0, ChessPieceType::Pawn, Vec3(), EulerAngles());
}

ChessReferee::~ChessReferee()
{
    delete m_chessBoard;
    m_chessBoard = nullptr;
}

void ChessReferee::InitializeTheBoard()
{
    m_chessBoard = new ChessBoard(this);
}

void ChessReferee::InitializeLights()
{
    //Add a point light
    Rgba8 plColor = Rgba8(255,255,255,20);
    m_lightColors.push_back(plColor);
    Vec3 plOriginalPosition = Vec3(4.f, 4.f, 5.f);
    m_worldPositions.push_back(plOriginalPosition);
    m_spotForwards.push_back(Vec3());
    m_ambiences.push_back(0.5f);
    float plInnerR = 4.5f;
    m_innerRadii.push_back(plInnerR);
    float plOuterR = 15.f;
    m_outerRadii.push_back(plOuterR);
    m_innerDotThresholds.push_back(-1.f);
    m_outerDotThresholds.push_back(-1.f);

    //Add a spot light
    Rgba8 slColor = Rgba8::GREY;
    m_lightColors.push_back(slColor);
    Vec3 slOriginalPosition = Vec3(4.f, 4.f, 7.f);
    m_worldPositions.push_back(slOriginalPosition);
    m_spotForwards.push_back(Vec3(0.f,0.f,-1.f));
    m_ambiences.push_back(0.2f);
    float slInnerR = 2.f;
    m_innerRadii.push_back(slInnerR);
    float slOuterR = 15.f;
    m_outerRadii.push_back(slOuterR);
    m_innerDotThresholds.push_back(CosDegrees(15.f));
    m_outerDotThresholds.push_back(SinDegrees(45.f));

    //Add a spot light for show
    Rgba8 slColor1 = Rgba8(255,255,0,0);
    m_lightColors.push_back(slColor1);
    Vec3 slOriginalPosition1 = Vec3(4.f, 4.f, 3.f);
    m_worldPositions.push_back(slOriginalPosition1);
    m_spotForwards.push_back(Vec3(0.f,0.f,-1.f));
    m_ambiences.push_back(0.2f);
    float slInnerR1 = 0.f;
    m_innerRadii.push_back(slInnerR1);
    float slOuterR1 = 4.f;
    m_outerRadii.push_back(slOuterR1);
    m_innerDotThresholds.push_back(CosDegrees(15.f));
    m_outerDotThresholds.push_back(SinDegrees(45.f));

    //Add a spot light
    Rgba8 slColor2 = Rgba8(0,0,0,0);
    m_lightColors.push_back(slColor2);
    Vec3 slOriginalPosition2 = Vec3(4.f, 10.f, 5.f);
    m_worldPositions.push_back(slOriginalPosition2);
    m_spotForwards.push_back(Vec3(0.f,-1.f,-1.f).GetNormalized());
    m_ambiences.push_back(0.3f);
    float slInnerR2 = 5.f;
    m_innerRadii.push_back(slInnerR2);
    float slOuterR2 = 30.f;
    m_outerRadii.push_back(slOuterR2);
    m_innerDotThresholds.push_back(CosDegrees(15.f));
    m_outerDotThresholds.push_back(SinDegrees(45.f));
    
    //Add a spot light
    Rgba8 slColor3 = Rgba8(0,0,0,0);
    m_lightColors.push_back(slColor3);
    Vec3 slOriginalPosition3 = Vec3(4.f, -2.f, 5.f);
    m_worldPositions.push_back(slOriginalPosition3);
    m_spotForwards.push_back(Vec3(0.f,1.f,1.f).GetNormalized());
    m_ambiences.push_back(0.3f);
    float slInnerR3 = 5.f;
    m_innerRadii.push_back(slInnerR3);
    float slOuterR3 = 30.f;
    m_outerRadii.push_back(slOuterR3);
    m_innerDotThresholds.push_back(CosDegrees(15.f));
    m_outerDotThresholds.push_back(SinDegrees(45.f));
    
    m_numLights = 5;
}

void ChessReferee::Update(float deltaSeconds)
{
    m_chessBoard->Update(deltaSeconds);
    UpdateLights(deltaSeconds);

    if (m_myKishi)
	{
		if (!g_theGame->m_isRemote || (m_currentState == MatchState::PLAYING &&
			m_currentMoveKishiIndex == m_myKishi->GetPlayerID()))
		{
			m_chessRaycastResult = UpdateChessRaycast();
			UpdateGrabAndUngrab();
		}

    }
}

void ChessReferee::UpdateLights(float deltaSeconds)
{
    //patrol
    static const Vec2 corners[4] = {
        Vec2(1.f, 1.f), 
        Vec2(6.f, 1.f), 
        Vec2(6.f, 6.f), 
        Vec2(1.f, 6.f)  
    };

    Vec2 from = corners[m_lightPathIndex];
    Vec2 to   = corners[(m_lightPathIndex + 1) % 4];

    float edgeLength = (to - from).GetLength();
    m_lightTAlongEdge += (m_lightSpeed * deltaSeconds) / edgeLength;

    if (m_lightTAlongEdge >= 1.f)
    {
        m_lightTAlongEdge = 0.f;
        m_lightPathIndex = (m_lightPathIndex + 1) % 4;
        from = corners[m_lightPathIndex];
        to   = corners[(m_lightPathIndex + 1) % 4];
    }

    Vec2 pos;
    pos.x = Interpolate(from.x, to.x, m_lightTAlongEdge);
    pos.y = Interpolate(from.y, to.y, m_lightTAlongEdge);
    m_worldPositions[1].x = pos.x;
    m_worldPositions[1].y = pos.y;

    //highlight
    if (m_chessRaycastResult.m_raycast.m_didImpact == true &&
        m_chessRaycastResult.m_impactedObject)
    {
        m_worldPositions[2].x = m_chessRaycastResult.m_impactedObject->m_position.x;
        m_worldPositions[2].y = m_chessRaycastResult.m_impactedObject->m_position.y;
        m_lightColors[2].a = 255;
    }
    else
    {
        m_lightColors[2].a = 0;
    }

    //light current player
    if (m_currentMoveKishiIndex==0)
    {
        m_lightColors[3] = Rgba8::CYAN;
        m_lightColors[3].a = 100;

        m_lightColors[4] = Rgba8(0,0,0,0);
    }
    if (m_currentMoveKishiIndex==1)
    {
        m_lightColors[3] = Rgba8::YELLOW;
        m_lightColors[3].a = 80;
        
        m_lightColors[3] = Rgba8(0,0,0,0);
    }
    if (g_theGame->m_hasWon)
    {
        m_lightColors[3] = Rgba8(0,0,0,0);
        m_lightColors[4] = Rgba8(0,0,0,0);
    }
}

void ChessReferee::Reset()
{
	if (!g_theGame->m_isRemote)
	{
		m_currentState = MatchState::PLAYING;
		g_theDevConsole->AddLine(Rgba8::PEACH, "The match state is: Playing. The match has started.");
	}
	else
	{
		m_currentState = MatchState::CONNECTING;
		g_theDevConsole->AddLine(Rgba8::PEACH, "The match state is: Connecting. Try to connect to a server/client to play.");
	}
    delete m_chessBoard;
    m_chessBoard = nullptr;
    m_chessBoard = new ChessBoard(this);

	m_nextMoveKishiIndex = 0;  //1先移动
	m_currentMoveKishiIndex = 1;
	m_myKishi = nullptr;  //for remote
	m_myName = "";
	m_opponentName = "";
	m_promoteType = "pawn";
	m_status = ChessStatus::SPECTATOR;
	m_connectedKishi = 0;
	m_currentSetFromCoord = IntVec2::NEGATIVEONE;
	m_currentSetToCoord = IntVec2::NEGATIVEONE;
}

void ChessReferee::RegisterForEvents()
{
    g_theEventSystem->SubscribeEventCallBackFunction("chessmove", OnChessMove);

    //under remote mode...
    g_theEventSystem->SubscribeEventCallBackFunction("chessserverinfo", OnChessServerInfo); //local
    g_theEventSystem->SubscribeEventCallBackFunction("chesslisten", OnChessListen); 
    g_theEventSystem->SubscribeEventCallBackFunction("chessconnect", OnChessConnect);
    g_theEventSystem->SubscribeEventCallBackFunction("chessdisconnect", OnChessDisconnect);
    g_theEventSystem->SubscribeEventCallBackFunction("remotecmd", OnRemoteCmd);
    
    g_theEventSystem->SubscribeEventCallBackFunction("chessplayerinfo", OnChessPlayerInfo); //remote
    g_theEventSystem->SubscribeEventCallBackFunction("chessbegin", OnChessBegin);
    g_theEventSystem->SubscribeEventCallBackFunction("chessvalidate", OnChessValidate);
    g_theEventSystem->SubscribeEventCallBackFunction("chessresign", OnChessResign);
    g_theEventSystem->SubscribeEventCallBackFunction("chessofferdraw", OnChessOfferDraw);
    g_theEventSystem->SubscribeEventCallBackFunction("chessacceptdraw", OnChessAcceptDraw);
    g_theEventSystem->SubscribeEventCallBackFunction("chessrejectdraw", OnChessRejectDraw);
    g_theEventSystem->SubscribeEventCallBackFunction("chessstate", OnChessState);

	g_theEventSystem->SubscribeEventCallBackFunction("queen", OnPromoteClicked);
	g_theEventSystem->SubscribeEventCallBackFunction("bishop", OnPromoteClicked);
	g_theEventSystem->SubscribeEventCallBackFunction("knight", OnPromoteClicked);
	g_theEventSystem->SubscribeEventCallBackFunction("rook", OnPromoteClicked);
    g_theEventSystem->SubscribeEventCallBackFunction("changestate", OnMatchStateChange);
    g_theEventSystem->SubscribeEventCallBackFunction("connectsucceed", OnConnectSucceed);
    g_theEventSystem->SubscribeEventCallBackFunction("joingame", OnJoinGame);
}

void ChessReferee::PrintBoardStateToDevConsole()
{
    //print那个map 、/二编：不用map了
    Rgba8 cyan = Rgba8::CYAN;

    g_theDevConsole->AddLine(cyan, "  ABCDEFGH");

    g_theDevConsole->AddLine(cyan, " +--------+");

    // 行 8 到 1（y=7 到 y=0）
    for (int y = 7; y >= 0; --y)
    {
        std::string line = std::to_string(y + 1) + "|";

        for (int x = 0; x < 8; ++x)
        {
            IntVec2 coord(x, y);
            ChessPiece* piece = m_chessBoard->GetPiece(coord);
            if (piece != nullptr)
            {
                char symbol = piece->m_definition.m_glyph[piece->m_ownerKishiID];
                line += symbol;
            }
            // auto iter = m_chessBoard->m_chessPieces.find(coord);
            // if (iter != m_chessBoard->m_chessPieces.end() && iter->second != nullptr)
            // {
            //     const ChessPiece* piece = iter->second;
            //     char symbol = piece->m_definition.m_glyph[piece->m_ownerKishiID];
            //     line += symbol;
            // }
            else
            {
                line += '.';
            }
        }

        line += "|" + std::to_string(y + 1);
        g_theDevConsole->AddLine(cyan, line);
    }

    g_theDevConsole->AddLine(cyan, " +--------+");

    g_theDevConsole->AddLine(cyan, "  ABCDEFGH");
}

std::string ChessReferee::GetBoardStateAsString()
{
    std::string boardString;
    for (int y = 7; y >= 0; --y)
    {
        for (int x = 0; x < 8; ++x)
        {
            IntVec2 coord(x, y);
            ChessPiece* piece = m_chessBoard->GetPiece(coord);
            if (piece != nullptr)
            {
                char symbol = piece->m_definition.m_glyph[piece->m_ownerKishiID];
                boardString += symbol;
            }
            else
            {
                boardString += '.';
            }
        }

        boardString += '\n'; 
    }
    return boardString;
}

void ChessReferee::PrintCurrentPlayerRound()
{
    std::string currentColor; //TODO: let game config decides the color
    if (m_currentMoveKishiIndex ==0) //blue
    {
        currentColor = "Blue";
        //nextColor = "Yellow";
    }
    else
    {
        currentColor = "Yellow";
    }
    
    g_theDevConsole->AddLine(Rgba8::LAVENDER, "Player #" + std::to_string(m_currentMoveKishiIndex)+
        " (" + currentColor + ") -- it's your move!");
}

void ChessReferee::SwapAndPrintBoardStatesAndRound() const
{
    std::swap(g_theGame->m_chessReferee->m_nextMoveKishiIndex, g_theGame->m_chessReferee->m_currentMoveKishiIndex);
    g_theGame->m_chessReferee->PrintCurrentPlayerRound();
    g_theGame->m_chessReferee->PrintBoardStateToDevConsole();
}

void ChessReferee::Render() const
{
    g_theRenderer->SetGeneralLightConstants(m_sunColor, m_sunDirection.GetNormalized(), m_numLights,
        m_lightColors, m_worldPositions, m_spotForwards, m_ambiences, m_innerRadii, m_outerRadii,
        m_innerDotThresholds, m_outerDotThresholds);
    
    g_theRenderer->SetPerFrameConstants(g_theGame->m_debugTime, g_theGame->m_debugInt, g_theGame->m_debugFloat);
    g_theRenderer->SetBlendMode(BlendMode::ALPHA);
    g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
    g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);

	// g_theRenderer->BindTexture(g_theGame->m_testMesh->m_diffuseTexture, 0);
	// g_theRenderer->BindTexture(g_theGame->m_testMesh->m_normalTexture, 1);
	// g_theRenderer->BindTexture(g_theGame->m_testMesh->m_specularTexture, 2);
	// g_theRenderer->BindShader(g_theGame->m_testMesh->m_shader);
	// g_theRenderer->SetModelConstants(g_theGame->m_testMesh->m_transform);
 //    g_theRenderer->DrawIndexBuffer(g_theGame->m_testMesh->m_vertexBuffer,
 //        g_theGame->m_testMesh->m_indexBuffer, (unsigned int)g_theGame->m_testMesh->m_indices.size());
 //    //g_theRenderer->DrawVertexArray(g_theGame->m_testMesh->m_verts);

    if(m_chessBoard)
        m_chessBoard->Render();
    RenderGhostPiece();
}

void ChessReferee::RenderGhostPiece() const
{
    if (m_hasFoundLegalMovePos)
    {
        m_ghostPiece->Render();
    }
}

bool ChessReferee::OnChessMove(EventArgs& args)
{
    if (g_theGame->m_hasWon)
        return true;
    
    if (g_theGame->m_isRemote)
    {
        if (g_theGame->m_chessReferee->m_status != ChessStatus::SPECTATOR)
		{
			if (g_theGame->m_chessReferee->m_chessKishi[g_theGame->m_chessReferee->m_currentMoveKishiIndex] !=
				g_theGame->m_chessReferee->m_myKishi && !args.GetValue("remote", false))
			{
				g_theDevConsole->AddLine(Rgba8::RED, "It's not your turn!");
				return false;
			}
        }
        else
        {
            if (!args.GetValue("remote", false))
            {
				g_theDevConsole->AddLine(Rgba8::RED, "Specatator cannot move!");
				return false;
            }
        }
    }
    
    
    std::string from = args.GetValue("from", "Unknown coordinate");
    std::string to = args.GetValue("to", "Unknown coordinate");
    std::string teleportResult = args.GetValue("teleport", "false");
    std::string promoteTo = args.GetValue("promoteTo", "");

    IntVec2 fromCoord = g_theGame->m_chessReferee->m_chessBoard->ParseCoordinate(from);
    IntVec2 toCoord = g_theGame->m_chessReferee->m_chessBoard->ParseCoordinate(to);
    bool tResult;
    if (teleportResult == "true")
    {
        tResult = true;
    }
    else
    {
        tResult = false;
    }
    // if (teleportResult == "false")
    // {
    //     tResult = false;
    // }
    // if (teleportResult == "")
    // {
    //     tResult = false;
    // }
    // else //Parse失败
    // {
    //     g_theDevConsole->AddLine(Rgba8::RED, "Illegal chess move; must have from= and to= arguments!");
    //     return true;
    // }

    //Parse失败
    if (fromCoord == IntVec2::NEGATIVEONE || toCoord == IntVec2::NEGATIVEONE)
    {
        g_theDevConsole->AddLine(Rgba8::RED, "Illegal chess move; must have from= and to= arguments!");
        g_theDevConsole->AddLine(Rgba8::AQUA, "  ChessMove from=e2 to=e4");
        return true;
    }
    //出界
    if (fromCoord == IntVec2(100,100))
    {
        ChessMoveResult result = ChessMoveResult::INVALID_MOVE_BAD_LOCATION;
        //g_theDevConsole->AddLine(Rgba8::RED, "Illegal 'from=' square " + from +
        //    "; must be a two-letter [Column][Rank]");
        g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
        g_theDevConsole->AddLine(Rgba8::AQUA, "  Examples: E2, E4; A1 is bottom left and H8 is top-right");
        return true;
    }
    if (toCoord == IntVec2(100,100))
    {
        ChessMoveResult result = ChessMoveResult::INVALID_MOVE_BAD_LOCATION;
        //g_theDevConsole->AddLine(Rgba8::RED, "Illegal 'to=' square " + to +
        //   "; must be a two-letter [Column][Rank]");
        g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
        g_theDevConsole->AddLine(Rgba8::AQUA, "  Examples: E2, E4; A1 is bottom left and H8 is top-right");
        return true;
    }
    //same
    if (fromCoord == toCoord)
    {
        //g_theDevConsole->AddLine(Rgba8::RED, "The from=" + from + " and t0=" + to + " coordinates cannot be the same square!");
        ChessMoveResult result = ChessMoveResult::INVALID_MOVE_ZERO_DISTANCE;
        g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
        return true;
    }
    //位置上没棋子
    if (!g_theGame->m_chessReferee->m_chessBoard->IsTherePiece(fromCoord))
    {
        ChessMoveResult result = ChessMoveResult::INVALID_MOVE_NO_PIECE;
        g_theDevConsole->AddLine(Rgba8::RED, "There's no chess at " + from);
        g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
        return true;
    }
    // if (!g_theGame->m_chessReferee->m_chessBoard->IsTherePiece(toCoord))
    // {
    //     g_theDevConsole->AddLine(Rgba8::RED, "There's no chess at " + to);
    //     return false;
    // }
    std::string currentColor; //TODO: let game config decides the color
    std::string nextColor;
    if (g_theGame->m_chessReferee->m_currentMoveKishiIndex ==0) //blue
    {
        currentColor = "Blue";
        nextColor = "Yellow";
    }
    else
    {
		nextColor = "Blue";
		currentColor = "Yellow";
    }
    //位置上的棋子不是自己的
    ChessPiece* fromPiece = g_theGame->m_chessReferee->m_chessBoard->GetPiece(fromCoord);
    if (fromPiece->m_ownerKishiID != g_theGame->m_chessReferee->m_currentMoveKishiIndex)
    {
        g_theDevConsole->AddLine(Rgba8::RED, "The " + fromPiece->m_definition.m_name +
            " at " + from + " belongs to player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
            + " ("+nextColor+"); it is currently player #"+
            std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex)+ " (" + currentColor+")'s turn.");
        ChessMoveResult result = ChessMoveResult::INVALID_MOVE_NOT_YOUR_PIECE;
        g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
        return true;
    }
    //目标格子上的棋子是自己的
    ChessPiece* toPiece = g_theGame->m_chessReferee->m_chessBoard->GetPiece(toCoord);
    if (toPiece &&fromPiece->m_ownerKishiID == toPiece->m_ownerKishiID)
    {
        g_theDevConsole->AddLine(Rgba8::RED, "Cannot move to" + to +
            ", since it is occupied by your own "+toPiece->m_definition.m_name);
        ChessMoveResult result = ChessMoveResult::INVALID_MOVE_DESTINATION_BLOCKED;
        g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
        return true;
    }

    ChessPieceType promoteType = ChessPieceType::Count;
    if (promoteTo == "queen")
    {
        promoteType = ChessPieceType::Queen;
    }
    else if (promoteTo == "bishop")
    {
        promoteType = ChessPieceType::Bishop;
    }
    else if (promoteTo == "knight")
    {
        promoteType = ChessPieceType::Knight;
    }
    else if (promoteTo == "rook")
    {
        promoteType = ChessPieceType::Rook;
    }

    if (tResult == false)
    {
        //capture! //二编: move!!
        fromPiece->OnMove(fromCoord, toCoord, promoteType);

        if (!args.GetValue("remote", false))
        {
			std::string anotherCmd = args.AppendToString();
			anotherCmd = "RemoteCmd cmd=chessmove " + anotherCmd + " remote=true";
			g_theNetworkSystem->SendStringToRemote(anotherCmd);
        }

        return true;
    }
    else
    {
        g_theDevConsole->AddLine(Rgba8::MISTBLUE,
        "Moved Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
            +currentColor+")'s " + fromPiece->m_definition.m_name + " from " + from + " to " + to);
        if (toPiece)
        {
            g_theDevConsole->AddLine(Rgba8::PEACH,
            "Player #" + std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                + currentColor +") captured Player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
                + " ("+ nextColor +")'s " + toPiece->m_definition.m_name + " at " + to);
        }
        g_theGame->m_chessReferee->m_chessBoard->CaptureAnotherPiece(fromCoord, toCoord);
        
        fromPiece->OnSemiLegalMove(fromCoord, toCoord, promoteType);
    
        if (g_theGame->m_hasWon == true)
        {
            g_theDevConsole->AddLine(Rgba8::GREY, "#########################################");
            g_theDevConsole->AddLine(Rgba8::YELLOW, "Player #"+std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex) + " ("
                + currentColor +") has won the match!");
            g_theDevConsole->AddLine(Rgba8::GREY, "#########################################");
            return true;
        }
    
        std::swap(g_theGame->m_chessReferee->m_nextMoveKishiIndex, g_theGame->m_chessReferee->m_currentMoveKishiIndex);
        g_theGame->m_chessReferee->PrintCurrentPlayerRound();
        g_theGame->m_chessReferee->PrintBoardStateToDevConsole();

		std::string anotherCmd = args.AppendToString();
		anotherCmd = "RemoteCmd cmd=chessmove " + anotherCmd + " remote=true";
		g_theNetworkSystem->SendStringToRemote(anotherCmd);

        return true;
    }
    
    return false;
}

bool ChessReferee::OnChessServerInfo(EventArgs& args)
{
    if (g_theNetworkSystem->HasConnected())
    {
        if (!args.IsEmpty())
        {
            g_theDevConsole->AddLine(Rgba8::YELLOW, "WARNING: Cannot change IP/Port while connected.");
            return false;
        }
    }
    else
    {
        if (!args.IsEmpty())
            g_theDevConsole->AddLine(Rgba8::YELLOW, "Set Server Info");
        {
            if (args.Has("ip") && g_theNetworkSystem->GetNetworkMode() == NetworkMode::CLIENT)
                g_theNetworkSystem->SetServerAddress(args.GetValue("ip", ""));
            else if(args.Has("ip") && g_theNetworkSystem->GetNetworkMode() == NetworkMode::CLIENT)
                g_theDevConsole->AddLine(Rgba8::YELLOW, "Cannot set ip when in SERVER mode");
        }
        if (args.Has("port"))
            g_theNetworkSystem->SetServerPort((uint16_t)atoi((args.GetValue("port", "").c_str())));
    }
    std::string mode;
    std::string ip;
    std::string port;
    if (g_theNetworkSystem->GetNetworkMode() == NetworkMode::CLIENT)
    {
        mode = "Net mode: Client";
        ip = "The ip to connect: "+g_theNetworkSystem->GetServerAddress();
        port = std::to_string(g_theNetworkSystem->GetServerPort());
    }
    if (g_theNetworkSystem->GetNetworkMode() == NetworkMode::SERVER)
    {
        mode = "Net mode: Server";
        ip = "The server local ip: " + g_theNetworkSystem->GetMyLocalIPAddress();
        port = "The server port: " + std::to_string(g_theNetworkSystem->GetServerPort());
    }
    g_theDevConsole->AddLine(Rgba8::CYAN, mode);
    g_theDevConsole->AddLine(Rgba8::YELLOW, ip);
    g_theDevConsole->AddLine(Rgba8::GREEN, port);
    return true;
}

bool ChessReferee::OnChessListen(EventArgs& args)
{
    if (g_theGame->m_chessReferee->m_currentState != MatchState::CONNECTING)
    {
        g_theDevConsole->AddLine(Rgba8::RED, "ERROR: Cannot Listen while game has connected.");
        return false;
    }
    if (g_theNetworkSystem->GetNetworkMode() == NetworkMode::CLIENT)
    {
        g_theDevConsole->AddLine(Rgba8::RED, "ERROR: Cannot Listen while connected as a client.");
        return false;
    }
    else if (!g_theNetworkSystem->HasConnected() && g_theNetworkSystem->GetNetworkMode()==NetworkMode::CLIENT)
    {
        g_theNetworkSystem->DisconnectClient(g_theNetworkSystem->GetMyClientConnection().m_socket);
        g_theDevConsole->AddLine(Rgba8::MINTGREEN, "Transform from a client to be a server");
    }

    if (args.Has("port"))
    {
        g_theNetworkSystem->SetServerConfig((uint16_t)atoi((args.GetValue("port", "")).c_str()));
    }
    g_theNetworkSystem->StartServerOrClient();
    g_theDevConsole->AddLine(Rgba8::YELLOW, "Now Start Listen:");
    return true;
}

bool ChessReferee::OnChessConnect(EventArgs& args)
{
    if (g_theGame->m_chessReferee->m_currentState != MatchState::CONNECTING)
    {
		g_theDevConsole->AddLine(Rgba8::RED, "ERROR: Cannot connect when it's now in connecting mode.");
        return false;
    }
	if (g_theNetworkSystem->GetNetworkMode() == NetworkMode::SERVER)
	{
		g_theDevConsole->AddLine(Rgba8::RED, "ERROR: Cannot connect as a server.");
		return false;
	}
	if (g_theNetworkSystem->HasConnected())
	{
		g_theDevConsole->AddLine(Rgba8::RED, "ERROR: Cannot connect when being connected.");
		return false;
	}
	if (!args.IsEmpty())
	{
		if (args.Has("ip"))
		{
			g_theNetworkSystem->SetServerAddress(args.GetValue("ip", ""));
		}
		if (args.Has("port"))
		{
			g_theNetworkSystem->SetServerPort((uint16_t)atoi(args.GetValue("port", "").c_str()));
		}
	}
    g_theNetworkSystem->StartServerOrClient();
    return true;
}

bool ChessReferee::OnChessDisconnect(EventArgs& args)
{
    if (!g_theGame->m_isRemote)
        return false;
	if (g_theGame->m_chessReferee->m_currentState == MatchState::CONNECTING)
	{
		g_theDevConsole->AddLine(Rgba8::RED, "ERROR: Cannot disconnect when trying connecting.");
		return false;
	}
    if (!g_theNetworkSystem->HasConnected())
        return false;

	if (g_theNetworkSystem->GetNetworkMode() == NetworkMode::SERVER && args.GetValue("remote", false))
	{
		//g_theNetworkSystem->Disconnect((uint16_t)atoi(args.GetValue("socket", "INVALID_SOCKET").c_str()));
	}
    if (g_theNetworkSystem->GetNetworkMode() == NetworkMode::SERVER && !args.GetValue("remote", false))
	{
		//g_theNetworkSystem->StopServer();

	}
    EventArgs stateArgs;
    stateArgs.SetValue("state", "connecting");
    g_theEventSystem->FireEvent("changestate", stateArgs);

    if (!args.GetValue("remote", false)) //false,本地发出指令->disconnect的同时发送一个remote的cmd
    {
        std::string myStatus;
        if (g_theGame->m_chessReferee->m_status == ChessStatus::KISHI)
        {
            myStatus = "player";
        }
		if (g_theGame->m_chessReferee->m_status == ChessStatus::SPECTATOR)
		{
			myStatus = "spectator";
		}
        std::string newCmd = args.AppendToString();
        newCmd = "RemoteCmd cmd=chessdisconnect " + newCmd + " status=" + myStatus;
		/*if (g_theNetworkSystem->GetNetworkMode() == NetworkMode::CLIENT)
		{
			newCmd += (" socket=" + g_theNetworkSystem->GetMyClientConnection().m_socket);
		}*/
        newCmd += " remote=true";
        g_theNetworkSystem->SendStringToRemote(newCmd);
		if (args.Has("reason"))
		{
			g_theDevConsole->AddLine(Rgba8::RED, "You disconnect because " + args.GetValue("reason", ""));
		}

		if (g_theNetworkSystem->GetNetworkMode() == NetworkMode::CLIENT)
		{
			//g_theNetworkSystem->StopClient();
            g_theNetworkSystem->m_pendingDisconnect = true;
		}
        return true;
    }
    if(args.GetValue("remote", false))
    {
		if (g_theNetworkSystem->GetNetworkMode() == NetworkMode::CLIENT)
		{
			//g_theNetworkSystem->Disconnect();
			g_theNetworkSystem->StopClient();
		}
	    g_theDevConsole->AddLine(Rgba8::YELLOW, "Another device has disconnected because " + args.GetValue("reason", ""));
    }

	if (args.GetValue("status", "spectator") == "player")
	{
		g_theGame->m_chessReferee->Reset();
		g_theDevConsole->AddLine(Rgba8::AQUA, "One of the players left, the game now resets and waiting for new players to join.");
		g_theDevConsole->AddLine(Rgba8::AQUA, "Your status is now a Spectator.");
	}
	if (args.GetValue("status", "spectator") == "spectator")
	{
		g_theGame->m_chessReferee->Reset();
		g_theDevConsole->AddLine(Rgba8::AQUA, "One of the spectators left.");
	}
	
    return true;
}

bool ChessReferee::OnRemoteCmd(EventArgs& args)
{
    std::string command = args.GetValue("cmd", "");
    EventArgs newArgs;
    const auto& pairs = args.GetAllKeyValuePairs(); 
    auto it = pairs.begin();
    if (it != pairs.end())
    {
        ++it;
    }
    for (; it != pairs.end(); ++it)
    {
        newArgs.SetValue(it->first, it->second);
    }

    newArgs.SetValue("remote", "true");

    g_theEventSystem->FireEvent(command, newArgs);
    return true;
}

bool ChessReferee::OnChessPlayerInfo(EventArgs& args)
{
	if (g_theGame->m_chessReferee->m_status == ChessStatus::SPECTATOR && !args.GetValue("remote", false))
	{
		g_theDevConsole->AddLine(Rgba8::YELLOW, "WARNING: Spectators cannot set the playerinfo.");
        return false;
	}
    if (args.Has("name"))
    {
        std::string name = args.GetValue("name", "Squirrel");
        if (!args.GetValue("remote", false))
        {
            g_theGame->m_chessReferee->m_myName = name;
            g_theDevConsole->AddLine(Rgba8::MAGENTA,"You have set your name as " + name);

			std::string nameCmd = args.AppendToString();
			nameCmd = "RemoteCmd cmd=chessplayerinfo " + nameCmd + " remote=true";
			g_theNetworkSystem->SendStringToRemote(nameCmd);

            return true;
        }
        if (args.GetValue("remote", false))
        {
            //std::string nameCmd = args.AppendToString();
            //nameCmd = "RemoteCmd cmd=chessplayerinfo " + nameCmd;
            //g_theNetworkSystem->SendStringToRemote(nameCmd);
            g_theDevConsole->AddLine(Rgba8::MAGENTA,"A player has set their name as " + name);
            g_theGame->m_chessReferee->m_opponentName = name;
            return true;
        }
        return false;
    }
    g_theDevConsole->AddLine(Rgba8::RED, "Add a name to set the player info!");
    return false;
}

bool ChessReferee::OnChessBegin(EventArgs& args)
{
    if (!g_theGame->m_isRemote)
    {
        return false;
    }
	if (g_theGame->m_chessReferee->m_status == ChessStatus::SPECTATOR && !args.GetValue("remote", false))
	{
		g_theDevConsole->AddLine(Rgba8::YELLOW, "WARNING: Spectators cannot begin the game.");
		return false;
	}
    if (g_theGame->m_chessReferee->m_status == ChessStatus::SPECTATOR && args.GetValue("remote", false))
    {
		EventArgs specArgs;
        specArgs.SetValue("state", "playing");
		g_theEventSystem->FireEvent("changestate", specArgs);
        return true;
    }

    if (!g_theGame->m_isRemote || g_theGame->m_chessReferee->m_currentState==MatchState::PLAYING)
    {
        g_theDevConsole->AddLine(Rgba8::BLUE, "Game has already begun!");
        return false;
    }
    if (g_theGame->m_isRemote && g_theGame->m_chessReferee->m_currentState == MatchState::CONNECTING)
    {
		g_theDevConsole->AddLine(Rgba8::RED, "Cannot begin the game because it's still in connecting state!");
		return false;
    }
    std::string myName = g_theGame->m_chessReferee->m_myName;
    std::string firstPlayer = args.GetValue("firstplayer", "");
    if (!args.Has("firstplayer"))
    {
        firstPlayer = g_theGame->m_chessReferee->m_myName;
    }
    
    bool isRemote = args.GetValue("remote", false);

    if (isRemote)
    {
        g_theDevConsole->AddLine(Rgba8::BLUE, firstPlayer + " has been set as the first player");

        // 如果对方先手（不是我），我就执黑；否则我执白
        if (firstPlayer == myName)
        {
            g_theDevConsole->AddLine(Rgba8::GREEN, "You are the first player.[player[1]]");
            g_theGame->m_chessReferee->m_myKishi = g_theGame->m_chessReferee->m_chessKishi[1]; // 白
            g_theGame->m_player->m_autoPos = Vec3(4.f, -2.f, 5.f);
            g_theGame->m_player->m_autoOrientation = EulerAngles(90.f, 45.f, 0.f);
        }
        else
        {
            g_theDevConsole->AddLine(Rgba8::GREEN, "You are the second player.[player[0]]");
            g_theGame->m_chessReferee->m_myKishi = g_theGame->m_chessReferee->m_chessKishi[0]; // 黑
			g_theGame->m_player->m_autoPos = Vec3(4.f, 10.f, 5.f);
			g_theGame->m_player->m_autoOrientation = EulerAngles(-90.f, 45.f, 0.f);
        }
    }
    else // 我本地发起的 ChessBegin
    {
        if (args.GetValue("firstplayer", "") == g_theGame->m_chessReferee->m_myName || 
            !args.Has("firstplayer"))
        {
			g_theDevConsole->AddLine(Rgba8::BLUE, "You have started a new game as the first player.[player[1]]");
			g_theGame->m_chessReferee->m_myKishi = g_theGame->m_chessReferee->m_chessKishi[1]; // 白方
			g_theGame->m_player->m_autoPos = Vec3(4.f, -2.f, 5.f);
			g_theGame->m_player->m_autoOrientation = EulerAngles(90.f, 45.f, 0.f);
        }
        else
        {
			g_theDevConsole->AddLine(Rgba8::BLUE, "You have started a new game as the second player.[player[0]]");
			g_theGame->m_chessReferee->m_myKishi = g_theGame->m_chessReferee->m_chessKishi[0]; // hei方
			g_theGame->m_player->m_autoPos = Vec3(4.f, 10.f, 5.f);
			g_theGame->m_player->m_autoOrientation = EulerAngles(-90.f, 45.f, 0.f);
        }

        std::string newCmd = args.AppendToString();
        if (!args.Has("firstplayer"))
        {
            newCmd = "firstplayer=" + myName + newCmd;
        }
        newCmd = "RemoteCmd cmd=chessbegin " + newCmd + " remote=true";
        g_theNetworkSystem->SendStringToRemote(newCmd);
    }
	g_theGame->m_player->m_mode = CameraMode::AUTO;
    //g_theGame->m_chessReferee->m_currentState = MatchState::PLAYING;
    EventArgs stateArgs;
    stateArgs.SetValue("state", "playing");
    g_theEventSystem->FireEvent("changestate", stateArgs);

    return true;
}

bool ChessReferee::OnChessValidate(EventArgs& args)
{
	if (g_theGame->m_chessReferee->m_status == ChessStatus::SPECTATOR)
	{
		g_theDevConsole->AddLine(Rgba8::YELLOW, "WARNING: Spectators cannot check if it's validate.");
		return false;
	}
    if (args.GetValue("remote", false)) //远端发来了东西 打印到devConsole
    {
        std::string longInfo = args.AppendToString();
        Strings infos = SplitStringOnDelimiter(longInfo, ' ');
        for (std::string line : infos)
        {
            Strings subLines = SplitStringOnDelimiter(line, '\n');
            for (std::string subLine : subLines)
            {
                g_theDevConsole->AddLine(Rgba8::MINTGREEN, subLine);
            }
        }

        std::string remoteState = args.GetValue("state", "");
        int remoteMoveIndex = args.GetValue("currentmovingplayer", -1);
        std::string myState = g_theGame->m_chessReferee->GetBoardStateAsString();
        int myMoveIndex = g_theGame->m_chessReferee->m_currentMoveKishiIndex;
        
        bool mismatch = false;
        if (remoteState != myState)
        {
            mismatch = true;
            g_theDevConsole->AddLine(Rgba8::RED, "Validation FAILED: board mismatch");
        }
        if (remoteMoveIndex != myMoveIndex)
        {
            mismatch = true;
            g_theDevConsole->AddLine(Rgba8::RED, "Validation FAILED: current moving player mismatch");
        }
        if (mismatch)
        {
            g_theDevConsole->AddLine(Rgba8::RED, "Validation failed, disconnecting...");

            EventArgs disconnectArgs;
            disconnectArgs.SetValue("reason", "VALIDATION FAILED");
            g_theEventSystem->FireEvent("chessdisconnect", disconnectArgs);
        }
        else
        {
            g_theDevConsole->AddLine(Rgba8::GREEN, "Validation PASSED.");
        }
    }
    else
    {
        EventArgs remoteArgs;
        remoteArgs.SetValue("state", g_theGame->m_chessReferee->GetBoardStateAsString());
        remoteArgs.SetValue("currentmovingplayer", std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex));
        remoteArgs.SetValue("remote", "true");
        //g_theEventSystem->FireEvent("chessvalidate", remoteArgs); 
        std::string newCmd = args.AppendToString();
        newCmd = "RemoteCmd cmd=chessvalidate " + newCmd + " remote=true";
        g_theNetworkSystem->SendStringToRemote(newCmd);
    }
    return true;
}

bool ChessReferee::OnChessResign(EventArgs& args)
{
    UNUSED(args);
    return true;
}

bool ChessReferee::OnChessOfferDraw(EventArgs& args)
{
    UNUSED(args);
    return true;
}

bool ChessReferee::OnChessAcceptDraw(EventArgs& args)
{
    UNUSED(args);
    return true;
}

bool ChessReferee::OnChessRejectDraw(EventArgs& args)
{
    UNUSED(args);
    return true;
}

bool ChessReferee::OnPromoteClicked(EventArgs& args)
{
    std::string promoteType = args.GetValue("value", "");
    EventArgs newArgs;
    IntVec2 from = g_theGame->m_chessReferee->m_currentSetFromCoord;
    IntVec2 to = g_theGame->m_chessReferee->m_currentSetToCoord;
    newArgs.SetValue("from", std::string{ static_cast<char>('a' + from.x), static_cast<char>('1' + from.y) });
    newArgs.SetValue("to", std::string{ static_cast<char>('a' + to.x),   static_cast<char>('1' + to.y) });
    newArgs.SetValue("promoteTo", promoteType);
	g_theEventSystem->FireEvent("chessmove", newArgs);
    g_theGame->m_promoteWidget->SetEnabled(false);
    g_theGame->m_gameClock->TogglePause();
    return false;
}

bool ChessReferee::OnChessState(EventArgs& args)
{
    UNUSED(args);
    std::string state;
    if (g_theGame->m_chessReferee->m_currentState == MatchState::CONNECTING)
        state = "Connecting";
	if (g_theGame->m_chessReferee->m_currentState == MatchState::SELECTING)
		state = "Selecting";
	if (g_theGame->m_chessReferee->m_currentState == MatchState::PLAYING)
		state = "Playing";


    g_theDevConsole->AddLine(Rgba8::MINTGREEN, "The current match state is "
            + state);
    return false;
}

bool ChessReferee::OnMatchStateChange(EventArgs& args)
{
    std::string state = args.GetValue("state", "Unknown state");
    MatchState receivedState = MatchState::UNKNOWN;
    if (state == "connecting")
        receivedState = MatchState::CONNECTING;
    if (state == "playing")
    {
        receivedState = MatchState::PLAYING;
    }
	if (state == "selecting")
        receivedState = MatchState::SELECTING;

    if (receivedState != g_theGame->m_chessReferee->m_currentState)
    {
        g_theGame->m_chessReferee->m_currentState = receivedState;
		g_theDevConsole->AddLine(Rgba8::MINTGREEN, "The game is now in " + state);
        if (receivedState == MatchState::SELECTING)
        {
		    g_theDevConsole->AddLine(Rgba8::MINTGREEN, "You've joined as a Spectator.");
        }
        if (receivedState == MatchState::PLAYING)
        {
			g_theGame->m_chessReferee->PrintCurrentPlayerRound();
			g_theGame->m_chessReferee->PrintBoardStateToDevConsole();
        }
    }
    
    return false;
}

bool ChessReferee::OnConnectSucceed(EventArgs& args)
{
    UNUSED(args);
    if (!g_theGame->m_isRemote)
    {
        g_theDevConsole->AddLine(Rgba8::RED, "Cannot succeed connection when it's not in remote mode!");
        return false;
    }
	/*if (g_theGame->m_chessReferee->m_currentState != MatchState::CONNECTING)
	{
		ERROR_AND_DIE("Network system reconnects to somewhere when it's connecting!")
	}*/
    if (g_theGame->m_chessReferee->m_currentState == MatchState::PLAYING)
    {
        g_theDevConsole->AddLine(Rgba8::RED, "Cannot succeed connection when it's playing!");
        return false;
    }
    EventArgs newArgs;
    newArgs.SetValue("state", "selecting");
    g_theEventSystem->FireEvent("changestate", newArgs);
    return false;
}

bool ChessReferee::OnJoinGame(EventArgs& args)
{
    UNUSED(args);
    if (g_theGame->m_chessReferee->m_currentState != MatchState::SELECTING)
    {
        g_theDevConsole->AddLine(Rgba8::RED, "You can only join the game as a player when it's connected and in Selecting mode.");
        return false;
    }
    if (!args.GetValue("remote", false))
    {
        if (g_theGame->m_chessReferee->m_connectedKishi >= 2)
        {
            g_theDevConsole->AddLine(Rgba8::RED, "The game has had 2 players. You cannot join the game.");
            return false;
        }
        g_theGame->m_chessReferee->m_status = ChessStatus::KISHI;
        g_theDevConsole->AddLine(Rgba8::AQUA, "Now u join the game as a player.");
        g_theDevConsole->AddLine(Rgba8::AQUA, "Your chess status is Player now.");
        g_theGame->m_chessReferee->m_connectedKishi++;

        std::string newCmd = "RemoteCmd cmd=joingame remote=true";
        g_theNetworkSystem->SendStringToRemote(newCmd);
        return true;
    }
    else
    {
        g_theDevConsole->AddLine(Rgba8::AQUA, "A player has joined the game.");
        g_theGame->m_chessReferee->m_connectedKishi++;
        return true;
    }
}

bool ChessReferee::OnReset(EventArgs& args)
{
    UNUSED(args);
    if (!g_theGame->m_isRemote)
    {
        g_theGame->m_chessReferee->Reset();
		g_theDevConsole->AddLine(Rgba8::GREEN, "The game has been reset.");
        return true;
    }
    else
    {
		g_theDevConsole->AddLine(Rgba8::RED, "Cannot directly to use reset in remote mode.");
        return true;
    }
}

ChessRaycastResult ChessReferee::UpdateChessRaycast()
{
    if (m_hasGrabbedPiece)
        return m_chessRaycastResult;
    
    ChessRaycastResult voidResult;
    
    if (m_chessRaycastResult.m_raycast.m_didImpact == false)
    {
        float impactDist = 100;
        ChessRaycastResult closestResult;
        for (ChessPiece* piece : m_chessBoard->m_chessPieces)
        {
            if (piece->m_ownerKishiID == m_currentMoveKishiIndex)
            {
                RaycastResult3D result;
                result = RaycastVsCylinderZ3D(g_theGame->m_cameraRay.m_rayStartPos, g_theGame->m_cameraRay.m_rayFwdNormal,
                    g_theGame->m_cameraRay.m_rayMaxLength, Vec2(piece->m_position.x, piece->m_position.y),
                    FloatRange(0.f, 1.f), 0.25f);
                if (result.m_didImpact == true)
                {
                    if (result.m_impactDist < impactDist)
                    {
                        impactDist = result.m_impactDist;
                        
                        closestResult.m_raycast = result;
                        closestResult.m_impactedObject = piece;
                    }
                }
            }
        }
        if (closestResult.m_raycast.m_didImpact == true)
        {
            closestResult.m_impactedObject->OnImpacted();
            for (ChessPiece* piece : m_chessBoard->m_chessPieces)
            {
                if (piece!=closestResult.m_impactedObject)
                {
                    piece->OnUnImpacted();
                }
            }
            return closestResult;
        }
        else
        {
            RaycastResult3D result;
            result = RaycastVsAABB3D(g_theGame->m_cameraRay.m_rayStartPos,g_theGame->m_cameraRay.m_rayFwdNormal,
                g_theGame->m_cameraRay.m_rayMaxLength, m_chessBoard->GetAABB());
            if (result.m_didImpact == true)
            {
                Vec3 impactPos = result.m_impactPos;
                ChessPiece* thisPiece = m_chessBoard->GetPiece(IntVec2(RoundDownToInt(impactPos.x), RoundDownToInt(impactPos.y)));
                if (thisPiece != nullptr && thisPiece->m_ownerKishiID == m_currentMoveKishiIndex)
                {
                    ChessRaycastResult chessResult;
                    chessResult.m_raycast = result;
                    chessResult.m_impactedObject = thisPiece;
                    thisPiece->OnImpacted();
                    
                    return chessResult;
                }
                else
                {
                    for (ChessPiece* piece : m_chessBoard->m_chessPieces)
                    {
                        piece->OnUnImpacted();
                    }
                    return voidResult;
                }
            }
        }
    }
    for (ChessPiece* piece : m_chessBoard->m_chessPieces)
    {
        if (m_chessRaycastResult.m_impactedObject!=nullptr && m_chessRaycastResult.m_impactedObject != piece)
            piece->OnUnImpacted();
    }
    return voidResult;
}

void ChessReferee::SetGhostPieceState(IntVec2 pos)
{
    if (m_chessRaycastResult.m_impactedObject)
    {
        m_ghostPiece->PromoteTo(m_chessRaycastResult.m_impactedObject->m_type);

        m_ghostPiece->m_ownerKishiID = m_currentMoveKishiIndex;
        m_ghostPiece->SetMyColor(m_currentMoveKishiIndex);
        if (m_currentMoveKishiIndex ==1)
        {
            m_ghostPiece->m_orientation = EulerAngles(90.f, 0.f, 0.f);
        }
        if (m_currentMoveKishiIndex ==0)
        {
            m_ghostPiece->m_orientation = EulerAngles(-90.f, 0.f, 0.f);
        }
        m_ghostPiece->m_position = m_chessBoard->GetCenterPosition(pos);
        m_ghostPiece->m_tint.a = 120;
    }
}

void ChessReferee::UpdateGrabAndUngrab()
{
    if (!m_hasGrabbedPiece && m_chessRaycastResult.m_impactedObject)
    {
        m_hasFoundLegalMovePos = false;
        if (g_theApp->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
        {
            m_hasGrabbedPiece=true;
            m_chessRaycastResult.m_impactedObject->OnGrabbed();
            return;
        }
    }
    if (m_hasGrabbedPiece)
    {
        if (g_theApp->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE))
        {
            m_hasGrabbedPiece = false;
            ResetChessMoveRayCast();
            return;
        }
        
        IntVec2 to = GetCurrentRaycastCoordExceptGrabbedPiece();
        SetGhostPieceState(to);

        //legal
        if (!g_theApp->IsKeyDown(KEYCODE_LEFTCONTROL))
        {
            ChessMoveResult legalMove;
            legalMove = OnRaycastMoveTest(m_chessRaycastResult.m_impactedObject->m_currentCoord, to);
            if (IsValid(legalMove))
            {
                m_hasFoundLegalMovePos = true;
            }
            else
            {
                m_hasFoundLegalMovePos = false;
            }

            if (g_theApp->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
            {
                if (IsValid(legalMove))
                {
                    OnRaycastValidMove(legalMove, m_chessRaycastResult.m_impactedObject->m_currentCoord, to);
                }
                else
                {
                    g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(legalMove));
                }

                //m_chessRaycastResult.m_impactedObject->OnUnGrabbed();
                m_hasGrabbedPiece = false;
                m_hasFoundLegalMovePos = false;
                ResetChessMoveRayCast();
                return;
            }
        }

        if (g_theApp->IsKeyDown(KEYCODE_LEFTCONTROL))
        {
            ChessMoveResult semiLegalMove;
            semiLegalMove = OnRaycastSemiMoveTest(m_chessRaycastResult.m_impactedObject->m_currentCoord, to);
            if (IsValid(semiLegalMove))
            {
                m_hasFoundLegalMovePos = true;
            }
            else
            {
                m_hasFoundLegalMovePos = false;
            }
            if (g_theApp->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
            {
                if (IsValid(semiLegalMove))
                {
                    OnRaycastSemiMove(semiLegalMove, m_chessRaycastResult.m_impactedObject->m_currentCoord, to);
                }

                m_hasGrabbedPiece = false;
                m_hasFoundLegalMovePos = false;
                ResetChessMoveRayCast();
            }
            return;
        }
    }
}

void ChessReferee::ResetChessMoveRayCast()
{
    m_chessRaycastResult.m_impactedObject->OnUnImpacted();
    m_chessRaycastResult.m_impactedObject->OnUnGrabbed();
    m_chessRaycastResult.m_impactedObject = nullptr;

    m_chessRaycastResult.m_raycast.m_didImpact = false;
}

IntVec2 ChessReferee::GetCurrentRaycastCoordExceptGrabbedPiece() const
{
    ChessPiece* currentImpactedP = (ChessPiece*)m_chessRaycastResult.m_impactedObject;

    RaycastResult3D voidResult;
    
        float impactDist = 100;
        ChessRaycastResult closestResult;
        for (ChessPiece* piece : m_chessBoard->m_chessPieces)
        {
            if (piece != currentImpactedP)
            {
                RaycastResult3D result;
                result = RaycastVsCylinderZ3D(g_theGame->m_cameraRay.m_rayStartPos, g_theGame->m_cameraRay.m_rayFwdNormal,
                    g_theGame->m_cameraRay.m_rayMaxLength, Vec2(piece->m_position.x, piece->m_position.y),
                    FloatRange(0.f, 1.f), 0.25f);
                if (result.m_didImpact == true)
                {
                    if (result.m_impactDist < impactDist)
                    {
                        impactDist = result.m_impactDist;
                        
                        closestResult.m_raycast = result;
                        closestResult.m_impactedObject = piece;
                    }
                }
            }
        }
        if (closestResult.m_raycast.m_didImpact == true)
        {
            for (ChessPiece* piece : m_chessBoard->m_chessPieces)
            {
                if (piece!=closestResult.m_impactedObject)
                {
                    piece->OnUnGrabbed();
                }
            }
            return closestResult.m_impactedObject->m_currentCoord;
        }
        else
        {
            RaycastResult3D result;
            result = RaycastVsAABB3D(g_theGame->m_cameraRay.m_rayStartPos,g_theGame->m_cameraRay.m_rayFwdNormal,
                g_theGame->m_cameraRay.m_rayMaxLength, m_chessBoard->GetAABB());
            if (result.m_didImpact == true)
            {
                Vec3 impactPos = result.m_impactPos;
                return IntVec2(RoundDownToInt(impactPos.x), RoundDownToInt(impactPos.y));
                
                //ChessPiece* thisPiece = m_chessBoard->GetPiece(IntVec2(RoundDownToInt(impactPos.x), RoundDownToInt(impactPos.y)));
                //if (thisPiece != nullptr)
                // {
                //     ChessRaycastResult chessResult;
                //     chessResult.m_raycast = result;
                //     chessResult.m_impactedObject = thisPiece;
                //     thisPiece->OnImpacted();
                //     
                //     return chessResult;
                // }
                // else
                // {
                //     for (ChessPiece* piece : m_chessBoard->m_chessPieces)
                //     {
                //         piece->OnUnImpacted();
                //     }
                //     return voidResult;
                // }
            //}
        }
    }

    for (ChessPiece* piece : m_chessBoard->m_chessPieces)
    {
        if (piece!=closestResult.m_impactedObject)
            piece->OnUnGrabbed();
    }
    return IntVec2::NEGATIVEONE;
}

ChessMoveResult ChessReferee::OnRaycastMoveTest(IntVec2 from, IntVec2 to)
{
    if (g_theGame->m_hasWon)
        return ChessMoveResult::INVALID_MOVE_NO_PIECE;

    IntVec2 fromCoord = from;
    IntVec2 toCoord = to;
    
    // //出界 不可能出界
    
    //same
    if (fromCoord == toCoord)
    {
        //g_theDevConsole->AddLine(Rgba8::RED, "The from=" + from + " and t0=" + to + " coordinates cannot be the same square!");
        ChessMoveResult result = ChessMoveResult::INVALID_MOVE_ZERO_DISTANCE;
        //g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
        return result;
    }

    std::string currentColor; //TODO: let game config decides the color
    std::string nextColor;
    if (g_theGame->m_chessReferee->m_currentMoveKishiIndex ==0) //blue
    {
        currentColor = "Blue";
        nextColor = "Yellow";
    }
    else
    {
		nextColor = "Blue";
		currentColor = "Yellow";
    }
    // //位置上的棋子不是自己的 bkn
     ChessPiece* fromPiece = g_theGame->m_chessReferee->m_chessBoard->GetPiece(fromCoord);
    // if (fromPiece->m_ownerKishiID != g_theGame->m_chessReferee->m_currentMoveKishiIndex)
    // {
    //     g_theDevConsole->AddLine(Rgba8::RED, "The " + fromPiece->m_definition.m_name +
    //         " at " + from + " belongs to player #" + std::to_string(g_theGame->m_chessReferee->m_nextMoveKishiIndex)
    //         + " ("+nextColor+"); it is currently player #"+
    //         std::to_string(g_theGame->m_chessReferee->m_currentMoveKishiIndex)+ " (" + currentColor+")'s turn.");
    //     ChessMoveResult result = ChessMoveResult::INVALID_MOVE_NOT_YOUR_PIECE;
    //     g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
    //     return true;
    // }
    //目标格子上的棋子是自己的
    // ChessPiece* toPiece = g_theGame->m_chessReferee->m_chessBoard->GetPiece(toCoord);
    // if (toPiece &&fromPiece->m_ownerKishiID == toPiece->m_ownerKishiID)
    // {
    //     g_theDevConsole->AddLine(Rgba8::RED, "Cannot move to" + to +
    //         ", since it is occupied by your own "+toPiece->m_definition.m_name);
    //     ChessMoveResult result = ChessMoveResult::INVALID_MOVE_DESTINATION_BLOCKED;
    //     g_theDevConsole->AddLine(Rgba8::RED, GetMoveResultString(result));
    //     return true;
    // }

    // ChessPieceType promoteType = ChessPieceType::Count;
    // if (promoteTo == "queen")
    // {
    //     promoteType = ChessPieceType::Queen;
    // }
    // else if (promoteTo == "bishop")
    // {
    //     promoteType = ChessPieceType::Bishop;
    // }
    // else if (promoteTo == "knight")
    // {
    //     promoteType = ChessPieceType::Knight;
    // }
    // else if (promoteTo == "rook")
    // {
    //     promoteType = ChessPieceType::Rook;
    // }

    // if (tResult == false)
    // {
    //     //capture! //二编: move!!
         
         return fromPiece->OnRaycastMoveTest(fromCoord, toCoord);
}

ChessMoveResult ChessReferee::OnRaycastSemiMoveTest(IntVec2 from, IntVec2 to)
{
    if (from == to)
    {
        ChessMoveResult result = ChessMoveResult::INVALID_MOVE_ZERO_DISTANCE;
        return result;
    }

    ChessPiece* fromPiece = g_theGame->m_chessReferee->m_chessBoard->GetPiece(from);
    if (fromPiece->m_ownerKishiID != g_theGame->m_chessReferee->m_currentMoveKishiIndex)
    {
        ChessMoveResult result = ChessMoveResult::INVALID_MOVE_NOT_YOUR_PIECE;
        return result;
    }
    ChessPiece* toPiece = g_theGame->m_chessReferee->m_chessBoard->GetPiece(to);
    if (toPiece &&fromPiece->m_ownerKishiID == toPiece->m_ownerKishiID)
    {
        ChessMoveResult result = ChessMoveResult::INVALID_MOVE_DESTINATION_BLOCKED;
        return result;
    }
    
    return fromPiece->OnRaycastSemiMoveTest(from, to);
}

void ChessReferee::OnRaycastValidMove(ChessMoveResult result, IntVec2 from, IntVec2 to)
{
    ChessPiece* fromPiece = g_theGame->m_chessReferee->m_chessBoard->GetPiece(from);
    fromPiece->OnRaycastValidMove(from, to, result);
    return;
}

void ChessReferee::OnRaycastSemiMove(ChessMoveResult result, IntVec2 from, IntVec2 to)
{
    ChessPiece* fromPiece = g_theGame->m_chessReferee->m_chessBoard->GetPiece(from);
    fromPiece->OnRaycastSemiMove(from, to, result);
    return;
}

bool IsValid(ChessMoveResult result)
{
    switch (result)
    {
    case ChessMoveResult::VALID_MOVE_NORMAL:
    case ChessMoveResult::VALID_MOVE_PROMOTION:
    case ChessMoveResult::VALID_CASTLE_KINGSIDE:
    case ChessMoveResult::VALID_CASTLE_QUEENSIDE:
    case ChessMoveResult::VALID_CAPTURE_NORMAL:
    case ChessMoveResult::VALID_CAPTURE_ENPASSANT:
    case ChessMoveResult::VALID_MOVE_PAWN_2SQUARE:
        return true;

    case ChessMoveResult::INVALID_MOVE_BAD_LOCATION:
    case ChessMoveResult::INVALID_MOVE_NO_PIECE:
    case ChessMoveResult::INVALID_MOVE_NOT_YOUR_PIECE:
    case ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE:
    case ChessMoveResult::INVALID_MOVE_ZERO_DISTANCE:
    case ChessMoveResult::INVALID_MOVE_DESTINATION_BLOCKED:
    case ChessMoveResult::INVALID_MOVE_PATH_BLOCKED:
    case ChessMoveResult::INVALID_MOVE_ENDS_IN_CHECK:
    case ChessMoveResult::INVALID_ENPASSANT_STALE:
    case ChessMoveResult::INVALID_MOVE_PAWN_BLOCKED:
    case ChessMoveResult::INVALID_MOVE_KING_TOGETHER:
    case ChessMoveResult::INVALID_MOVE_NO_PROMOTION:
    case ChessMoveResult::INVALID_CASTLE_KING_HAS_MOVED:
    case ChessMoveResult::INVALID_CASTLE_ROOK_HAS_MOVED:
    case ChessMoveResult::INVALID_CASTLE_PATH_BLOCKED:
    case ChessMoveResult::INVALID_CASTLE_THROUGH_CHECK:
    case ChessMoveResult::INVALID_CASTLE_OUT_OF_CHECK:
    case ChessMoveResult::UNKNOWN:
        return false;

    default: ERROR_AND_DIE(Stringf("Unhandled ChessMoveResult enum value #%d", (int)result))
    }
}

char const* GetMoveResultString(ChessMoveResult result)
{
    switch (result)
    {
    case ChessMoveResult::UNKNOWN:                          return "Unknown ChessMoveResult!";  
    case ChessMoveResult::VALID_MOVE_NORMAL:                return "Valid move";
    case ChessMoveResult::VALID_MOVE_PROMOTION:             return "Valid move, resulting in pawn promotion";
    case ChessMoveResult::VALID_MOVE_PAWN_2SQUARE:          return "Valid move, resulting in pawn moving 2 squares";
    case ChessMoveResult::VALID_CASTLE_KINGSIDE:            return "Valid move, castling kingisde";
    case ChessMoveResult::VALID_CASTLE_QUEENSIDE:           return "Valid move, castling queenside";
    case ChessMoveResult::VALID_CAPTURE_NORMAL:             return "Valid move, capturing enemy piece";
    case ChessMoveResult::VALID_CAPTURE_ENPASSANT:          return "Valid move, capturing enemy pawn en passant";
            
    case ChessMoveResult::INVALID_MOVE_BAD_LOCATION:        return "Invalid move; invalid board location given";
    case ChessMoveResult::INVALID_MOVE_NO_PIECE:            return "Invalid move; no piece at location given";
    case ChessMoveResult::INVALID_MOVE_NOT_YOUR_PIECE:      return "Invalid move; can't move opponent's piece";
    case ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE:    return "Invalid move; this piece can't go this shape";
    case ChessMoveResult::INVALID_MOVE_ZERO_DISTANCE:       return "Invalid move; didn't go anywhere";
    case ChessMoveResult::INVALID_MOVE_DESTINATION_BLOCKED: return "Invalid move; destination is blocked by your piece";
    case ChessMoveResult::INVALID_MOVE_PATH_BLOCKED:        return "Invalid move; path is blocked by your piece";
    case ChessMoveResult::INVALID_MOVE_ENDS_IN_CHECK:       return "Invalid move; can't leave yourself in check";
    case ChessMoveResult::INVALID_MOVE_PAWN_BLOCKED:       return "Invalid move; can't move pawn forward when there is a piece in front of it";
    case ChessMoveResult::INVALID_MOVE_KING_TOGETHER:       return "Invalid move; can't move 1 king next to another king";
    case ChessMoveResult::INVALID_MOVE_NO_PROMOTION:       return "Invalid move; must promote this pawn to queen/bishop/knight/rook";
    case ChessMoveResult::INVALID_ENPASSANT_STALE:          return "Invalid move; en passant must immediately follow a pawn double-move";
    case ChessMoveResult::INVALID_CASTLE_KING_HAS_MOVED:    return "Invalid castle; king has moved previously";
    case ChessMoveResult::INVALID_CASTLE_ROOK_HAS_MOVED:    return "Invalid castle; that rook has moved previously";
    case ChessMoveResult::INVALID_CASTLE_PATH_BLOCKED:      return "Invalid castle; pieces in-between king and rook";
    case ChessMoveResult::INVALID_CASTLE_THROUGH_CHECK:     return "Invalid castle; king can't move through check";
    case ChessMoveResult::INVALID_CASTLE_OUT_OF_CHECK:      return "Invalid castle; king can't castle out of check";

    default: ERROR_AND_DIE(Stringf("Unhandled ChessMoveResult enum value #%d", (int)result))
    }
}
