#pragma once
#include "ChessBoard.h"

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
    void InitializeWidgets();

    void Update(float deltaSeconds);

    void UpdateLights(float deltaSeconds);

    //Chess move
    void RegisterForEvents();
    void PrintBoardStateToDevConsole();
    void PrintCurrentPlayerRound();
    void SwapAndPrintBoardStatesAndRound() const;

    void Render() const;
    void RenderGhostPiece() const;

    static bool OnChessMove(EventArgs& args);

    ChessRaycastResult UpdateChessRaycast();
    
    void UpdateGrabAndUngrab();
    void SetGhostPieceState(IntVec2 pos);
    void ResetChessMoveRayCast();
    IntVec2 GetCurrentRaycastCoordExceptGrabbedPiece() const;
    ChessMoveResult OnRaycastMoveTest(IntVec2 from, IntVec2 to);
    ChessMoveResult OnRaycastSemiMoveTest(IntVec2 from, IntVec2 to);
    void OnRaycastValidMove(ChessMoveResult result, IntVec2 from, IntVec2 to);
    void OnRaycastSemiMove(ChessMoveResult result, IntVec2 from, IntVec2 to);

public:
    ChessBoard* m_chessBoard;
    ChessKishi** m_chessKishi;

    int m_nextMoveKishiIndex = 1;  //0先移动
    int m_currentMoveKishiIndex = 0;

    ChessRaycastResult m_chessRaycastResult;
    bool m_hasGrabbedPiece = false;
    bool m_hasFoundLegalMovePos = false;
    ChessPiece* m_ghostPiece = nullptr;

    float m_updateRateTimer = 0.f;
    float c_updateRate = 0.005f;

    //Light
    Vec3 m_sunDirection = Vec3(3.f, 1.f, -2.f);
    Rgba8 m_sunColor = Rgba8::DARKGREY;
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