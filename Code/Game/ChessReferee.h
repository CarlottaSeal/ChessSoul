#pragma once
#include "ChessBoard.h"

enum class MatchState
{
    UNKNOWN,
    CONNECTING,
    SELECTING,
    PLAYING,
    OVER
};

enum class ChessStatus
{
    KISHI,
    SPECTATOR,
    UNKNOWN
};

struct ChessRaycastResult
{
    ChessObject* m_impactedObject = nullptr;

    RaycastResult3D m_raycast;
};

class ChessReferee //Match
{
public:
    ChessReferee(ChessKishi* chessKishi[2]);
    ~ChessReferee();

    void InitializeTheBoard();
    void InitializeLights();
    
    void Update(float deltaSeconds);

    void UpdateLights(float deltaSeconds);

    //Chess move
    void Reset();
    void RegisterForEvents();
    void PrintBoardStateToDevConsole();
    std::string GetBoardStateAsString();
    void PrintCurrentPlayerRound();
    void SwapAndPrintBoardStatesAndRound() const;

    void Render() const;
    void RenderGhostPiece() const;

    ChessRaycastResult UpdateChessRaycast();
    
    void UpdateGrabAndUngrab();
    void SetGhostPieceState(IntVec2 pos);
    void ResetChessMoveRayCast();
    IntVec2 GetCurrentRaycastCoordExceptGrabbedPiece() const;
    ChessMoveResult OnRaycastMoveTest(IntVec2 from, IntVec2 to);
    ChessMoveResult OnRaycastSemiMoveTest(IntVec2 from, IntVec2 to);
    void OnRaycastValidMove(ChessMoveResult result, IntVec2 from, IntVec2 to);
    void OnRaycastSemiMove(ChessMoveResult result, IntVec2 from, IntVec2 to);

    //event
    static bool OnChessMove(EventArgs& args);
    static bool OnChessServerInfo(EventArgs& args);
    static bool OnChessListen(EventArgs& args);
    static bool OnChessConnect(EventArgs& args);
    static bool OnChessDisconnect(EventArgs& args);
    static bool OnRemoteCmd(EventArgs& args);
    
    static bool OnChessPlayerInfo(EventArgs& args);
    static bool OnChessBegin(EventArgs& args);
    static bool OnChessValidate(EventArgs& args);
    static bool OnChessResign(EventArgs& args);
    static bool OnChessOfferDraw(EventArgs& args);
    static bool OnChessAcceptDraw(EventArgs& args);
    static bool OnChessRejectDraw(EventArgs& args);
    static bool OnPromoteClicked(EventArgs& args);
    static bool OnChessState(EventArgs& args);
    static bool OnMatchStateChange(EventArgs& args);
    static bool OnConnectSucceed(EventArgs& args);
    static bool OnJoinGame(EventArgs& args);
    static bool OnReset(EventArgs& args);

public:
    ChessBoard* m_chessBoard;
    ChessKishi** m_chessKishi;

    //game state
    MatchState m_currentState = MatchState::UNKNOWN;
    int m_nextMoveKishiIndex = 0;  //1先移动
    int m_currentMoveKishiIndex = 1;
    ChessKishi* m_myKishi = nullptr;  //for remote
    std::string m_myName = "";
    std::string m_opponentName = "";
    std::string m_promoteType = "pawn";
    ChessStatus m_status = ChessStatus::SPECTATOR;
    int m_connectedKishi = 0;

	IntVec2 m_currentSetFromCoord = IntVec2::NEGATIVEONE; //move condition
	IntVec2 m_currentSetToCoord = IntVec2::NEGATIVEONE;

    ChessRaycastResult m_chessRaycastResult;
    bool m_hasGrabbedPiece = false;
    bool m_hasFoundLegalMovePos = false;
    ChessPiece* m_ghostPiece = nullptr;

    float m_updateRateTimer = 0.f;
    float c_updateRate = 0.005f;

    //Light
    Vec3 m_sunDirection = Vec3(3.f, 1.f, -2.f);
    Rgba8 m_sunColor = Rgba8(90,90,90,255);
    int m_numLights = 0;
    std::vector<Rgba8> m_lightColors;
    std::vector<Vec3> m_worldPositions;
    std::vector<Vec3> m_spotForwards;
    std::vector<float> m_ambiences;
    std::vector<float> m_innerRadii;
    std::vector<float> m_outerRadii;
    std::vector<float> m_innerDotThresholds;
    std::vector<float> m_outerDotThresholds;
    int m_lightPathIndex = 0;      
    float m_lightTAlongEdge = 0.f; 
    float m_lightSpeed = 4.f;      
};

bool IsValid(ChessMoveResult result);
char const* GetMoveResultString(ChessMoveResult result);