#pragma once
#include "Game/Gamecommon.hpp"
#include "Game/App.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/UI/Widget.h"

#include <vector>

#include "ChessKishi.h"
#include "Engine/Core/StaticMesh.h"

class ChessReferee;
class ChessObject;
class Player;
class Clock;

enum class GameState
{
	ATTRACT,
	LOBBY,
	PLAYING,
	COUNT
};

class Game 
{
public:
	Game();
	~Game();

	void Startup();
	void Update();
	void Render() const;

	void AdjustForPauseAndTimeDistortion();

	//State machine functions
	void ChangeStateIfChange();
	void ChangeState(GameState newState);
	void EnterState(GameState state);
	void ExitState(GameState state);
	
	void UpdateFirstPersonRaycast();

public:
	bool m_isRemote = true;
	bool m_openDevConsole = false;
	bool m_isInAttractMode;
	Clock* m_gameClock;
	//Camera m_gameCamera;
	Camera m_screenCamera;

	//Player and chess entity
	Player* m_player; //For camera
	RaycastResult3D m_cameraRay;
	
	ChessReferee* m_chessReferee;
	ChessKishi* m_chessKishi[NUM_KISHI];

	//state
	GameState m_currentState = GameState::COUNT;
	GameState m_nextState = GameState::ATTRACT;
	Texture* m_attractCover = nullptr;

	bool m_hasWon = false;

	//debug rendering
	float m_debugTime = 0.f;
	int m_debugInt = 0;
	float m_debugFloat = 0.f;

	StaticMesh* m_testMesh = nullptr;

	void LoadAnObj();

private:
	void AttractStateUpdate();
	void PlayingStateUpdate();
	void LobbyStateUpdate();
	void AttractModeRender() const;
	void PlayingModeRender() const;
	void LobbyModeRender() const;
	void EnterAttractState();
	void ExitAttractState();
	void EnterPlayingState();
	void ExitPlayingState();
	void ExitLobbyState();
	void EnterLobbyState();
	void InitializeWidgetsForAttract();
	void InitializeWidgetsForLobby();
	void RegisterAllWidgetEvents();
	
	void PrintGameControlToDevConsole();
	void DrawSquareXYGrid(int unit = 100);
	void DebugRenderSystemInputUpdate();
	void DebugAddWorldAxisText(Mat44 worldMat);

	void UpdateDebugInt();
	std::string GetDebugRenderPrintByInt() const;

	//event
	static bool OnRemoteModeSelection(EventArgs& args);
	static bool OnLocalModeSelection(EventArgs& args);
	static bool OnModelSet1Selection(EventArgs& args);
	static bool OnModelSet2Selection(EventArgs& args);
	static bool OnModelSet3Selection(EventArgs& args);
	static bool OnServerModeSelection(EventArgs& args);
	static bool OnClientModeSelection(EventArgs& args);


public:
	int m_setSelected = 0;

	//widgets
	Widget* m_attractWidget = nullptr;
	Widget* m_lobbyWidget[2];
	Widget* m_promoteWidget = nullptr;
	
private:
	bool m_isSlowMo;
	bool m_isUsingUserTimeScale;

	float m_userTimeScale;

	bool m_hasPlayedAttractSound = false;
	SoundPlaybackID m_attractSoundID = MISSING_SOUND_ID;

	float m_varyTime = 0.f;

	std::vector<Vertex_PCU> m_gridVertexes;
};





