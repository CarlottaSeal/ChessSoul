#pragma once
#include "Game/Gamecommon.hpp"
#include "Game/App.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Audio/AudioSystem.hpp"

#include <vector>

#include "ChessKishi.h"

class ChessReferee;
class ChessObject;
class Player;
class Clock;

enum class GameState
{
	ATTRACT,
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
	void EnterAttractState();
	void EnterPlayingState();
	void ExitPlayingState();
	void ExitLobbyState();

	void UpdateFirstPersonRaycast();

public:
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

	bool m_hasWon = false;

	//debug rendering
	float m_debugTime = 0.f;
	int m_debugInt = 0;
	float m_debugFloat = 0.f;

private:
	void AttractStateUpdate();
	void PlayingStateUpdate();
	void AttractModeRender() const;
	void PlayingModeRender() const;
	//void UpdateCamera();  //move into m_player
	void PrintGameControlToDevConsole();
	void DrawSquareXYGrid(int unit = 100);
	void DebugRenderSystemInputUpdate();
	void DebugAddWorldAxisText(Mat44 worldMat);

	void UpdateDebugInt();
	std::string GetDebugRenderPrintByInt() const;

private:
	bool m_isSlowMo;
	bool m_isUsingUserTimeScale;

	float m_userTimeScale;

	bool m_hasPlayedAttractSound = false;
	SoundPlaybackID m_attractSoundID = MISSING_SOUND_ID;

	float m_varyTime = 0.f;

	std::vector<Vertex_PCU> m_gridVertexes;
};





