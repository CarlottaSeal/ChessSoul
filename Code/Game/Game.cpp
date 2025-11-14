#include "Game.hpp"

#include "ChessObject.h"
#include "ChessPiece.h"
#include "ChessPieceDefinition.h"
#include "ChessReferee.h"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/AABB2.hpp"    
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include "Engine/UI/Button.h"

#include "Game/Player.hpp"
#include "Game/Gamecommon.hpp"

RandomNumberGenerator g_RNG;
//extern AudioSystem* g_theAudio;
extern Clock* s_theSystemClock;

Game::Game()
{
	m_isInAttractMode = true;
	m_screenCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(1600.f, 800.f));
	m_player = new Player(this);

	m_player->m_worldCamera.SetCameraMode(Camera::CameraMode::eMode_Perspective);
	m_player->m_worldCamera.SetPerspectiveView(2.f, 60.f, 0.1f, 100.f);
	Mat44 mat;
	mat.SetIJK3D(Vec3(0.f, 0.f,1.f), Vec3(-1.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f));
	((Player*)m_player)->m_worldCamera.SetCameraToRenderTransform(mat);

	//DebugAddWorldBasis(mat, -1.f, DebugRenderMode::USE_DEPTH);

	m_attractCover = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Cover4.png");
	Startup();
}

Game::~Game()
{
	ChessPieceDefinition::ClearDefinitions();

	if (m_attractWidget)
	{
		delete m_attractWidget;
		m_attractWidget = nullptr;
	}
	for (Widget* w : m_lobbyWidget)
	{
		delete w;
		w = nullptr;
	}
 }

void Game::Startup()
{
	PrintGameControlToDevConsole();
	ChessPieceDefinition::InitializeChessPieceDefinitions();

	ChessKishi* kishi1 = new ChessKishi(0);
	ChessKishi* kishi2 = new ChessKishi(1);
	m_chessKishi[0] = kishi1;
	m_chessKishi[1] = kishi2;

	RegisterAllWidgetEvents();
	LoadAnObj();
}

void Game::Update()
{
	if (g_theApp->WasKeyJustPressed(KEYCODE_ESC)
		|| g_theInput->GetController(0).WasButtonJustPressed(XboxButtonID::X))
	{
		if (m_currentState == GameState::ATTRACT)
		{
			g_theApp->g_isQuitting = true;
		}
		if (m_currentState != GameState::ATTRACT)
		{
			ChangeState(GameState::ATTRACT);
		}
	}
	AdjustForPauseAndTimeDistortion();

	if (m_currentState == GameState::ATTRACT)
	{
		AttractStateUpdate();
		if (!m_hasPlayedAttractSound)
		{
			//SoundID Attract = g_theAudio->CreateOrGetSound("Data/Audio/Attract.MP3");
			//m_attractSoundID = g_theAudio->StartSound(Attract, false, 1.0f, 0.5f, 1.0f, false);
			m_hasPlayedAttractSound = true;
		}
	}
	if (m_currentState == GameState::PLAYING)
	{
		PlayingStateUpdate();
	}
	if (m_currentState == GameState::LOBBY)
	{
		LobbyStateUpdate();
	}

	if (g_theDevConsole->GetMode() == OPEN_FULL)
	{
		m_openDevConsole = true;
	}
	if (g_theDevConsole->GetMode() == HIDDEN)
	{
		m_openDevConsole = false;
	}

	ChangeStateIfChange();

	m_varyTime += (float)m_gameClock->GetDeltaSeconds();
	if (m_varyTime > 360.f)
	{
		m_varyTime = 0.f;
	}
}

void Game::Render() const
{
	if (m_currentState == GameState::ATTRACT)
	{
		AttractModeRender();
	}
	if (m_currentState == GameState::PLAYING)
	{
		PlayingModeRender();
	}
	if (m_currentState == GameState::LOBBY)
	{
		LobbyModeRender();
	}
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr,0);
	g_theRenderer->BindTexture(nullptr,1);
	g_theRenderer->BindTexture(nullptr,2);
	g_theDevConsole->Render(AABB2(m_screenCamera.GetOrthographicBottomLeft(), m_screenCamera.GetOrthographicTopRight()), g_theRenderer);
}

void Game::AdjustForPauseAndTimeDistortion()
{
	if (g_theApp->WasKeyJustPressed('T'))
	{
		m_isSlowMo = !m_isSlowMo;
		m_isUsingUserTimeScale = false;
	}
	if (m_isUsingUserTimeScale) // 如果用户通过 set_time_scale 设定了时间缩放，保持该值
	{
		m_gameClock->SetTimeScale(m_userTimeScale);
	}
	else
	{
		m_gameClock->SetTimeScale(m_isSlowMo ? 0.1f : 1.0f);
	}

	if (g_theApp->WasKeyJustPressed('P'))
	{
		m_gameClock->TogglePause();
		if (!m_gameClock->IsPaused())
		{
			m_gameClock->Unpause();
			m_gameClock->Reset();
			//SoundID Pause = g_theAudio->CreateOrGetSound("Data/Audio/Pause.mp3");
			//g_theAudio->StartSound(Pause, false, 0.05f, 0.5f, 1.f, false);
		}
		if (m_gameClock->IsPaused())
		{
			m_gameClock->Reset();
			//SoundID Unpause = g_theAudio->CreateOrGetSound("Data/Audio/Unpause.mp3");
			//g_theAudio->StartSound(Unpause, false, 0.05f, 0.5f, 1.f, false);
		}
	}

	if (g_theApp->WasKeyJustPressed('O'))
	{
		m_gameClock->StepSingleFrame();
	}
}

void Game::ChangeStateIfChange()
{
	if (m_nextState != m_currentState)
	{
		ExitState(m_currentState);
		EnterState(m_nextState);
		m_currentState = m_nextState;
	}
}

void Game::ChangeState(GameState newState)
{
	m_nextState = newState;
}

void Game::EnterState(GameState state)
{
	switch (state)
	{
	case GameState::ATTRACT:
		EnterAttractState();
		break;
	case GameState::LOBBY:
		EnterLobbyState();
		break;
	case GameState::PLAYING:
		EnterPlayingState();
		break;
	}
	m_currentState = state;
}

void Game::ExitState(GameState state)
{
	switch (state)
	{
	case GameState::ATTRACT:
		ExitAttractState();
		break;
	case GameState::LOBBY:
		ExitLobbyState();
		break;
	case GameState::PLAYING:
		ExitPlayingState();
		break;
	}
}

void Game::EnterAttractState()
{
	m_hasWon = false;
	m_isRemote = false;
	InitializeWidgetsForAttract();
	if (m_attractWidget)
		m_attractWidget->Reset();
}

void Game::ExitAttractState()
{
	if (m_attractWidget)
		m_attractWidget->SetEnabled(false);
	
	// delete m_attractWidget;
	// m_attractWidget = nullptr;
}

void Game::EnterPlayingState()
{
	m_chessReferee = new ChessReferee(m_chessKishi);

	m_promoteWidget = new Widget(m_screenCamera, AABB2(Vec2(), Vec2()),
		"", Rgba8(0, 0, 0, 0));

	Button* rook = new Button(m_promoteWidget, AABB2(Vec2(200.f, 300.f), Vec2(400.f, 450.f))
		, Rgba8::MISTBLUE, Rgba8::WHITE, "rook", "Rook");
	Button* queen = new Button(m_promoteWidget, AABB2(Vec2(450.f, 300.f), Vec2(650.f, 450.f))
		, Rgba8::MISTBLUE, Rgba8::WHITE, "queen", "Queen");
	Button* bishop = new Button(m_promoteWidget, AABB2(Vec2(700.f, 300.f), Vec2(900.f, 450.f))
		, Rgba8::MISTBLUE, Rgba8::WHITE, "king", "Bishop");
	Button* knight = new Button(m_promoteWidget, AABB2(Vec2(950.f, 300.f), Vec2(1150.f, 450.f))
		, Rgba8::MISTBLUE, Rgba8::WHITE, "knight", "Knight");
	m_promoteWidget->AddChild(*rook);
	m_promoteWidget->AddChild(*queen);
	m_promoteWidget->AddChild(*bishop);
	m_promoteWidget->AddChild(*knight);
	m_promoteWidget->SetEnabled(false);
}

void Game::ExitPlayingState()
{
	EventArgs args;
	FireEvent("clear", args);

	delete m_promoteWidget;
	m_promoteWidget = nullptr;

	for (ChessKishi* chessKishi : m_chessKishi)
	{
		chessKishi->m_lastMovedPiece = nullptr;
	}
	delete m_chessReferee;
	m_chessReferee = nullptr;
}

void Game::ExitLobbyState()
{
	for (Widget* w : m_lobbyWidget)
	{
		if (w != nullptr)
			w->SetEnabled(false);
	}
}

void Game::EnterLobbyState()
{
	InitializeWidgetsForLobby();
	if (m_lobbyWidget[0] != nullptr)
	{
		m_lobbyWidget[0]->Reset();
		m_lobbyWidget[0]->SetEnabled(true);
	}
	if (m_lobbyWidget[1] != nullptr)
	{
		m_lobbyWidget[1]->Reset();
		m_lobbyWidget[1]->SetEnabled(false);
	}
}

void Game::InitializeWidgetsForAttract()
{
	if (!m_attractWidget)
	{
		m_attractWidget = new Widget(m_screenCamera, AABB2(Vec2() ,Vec2()),
		"", Rgba8(0,0,0,0));
		Button* exerciseButton = new Button(m_attractWidget, AABB2(Vec2(1350.f, 520.f), Vec2(1550.f, 600.f))
						, Rgba8::MISTBLUE, Rgba8::WHITE, "localmodeselection","Exercise");
		Button* remoteButton = new Button(m_attractWidget, AABB2(Vec2(1350.f, 360.f), Vec2(1550.f, 440.f))
						, Rgba8::MISTBLUE, Rgba8::WHITE, "remotemodeselection","Remote");
		Button* exit = new Button(m_attractWidget, AABB2(Vec2(1350.f, 200.f), Vec2(1550.f, 280.f))
						, Rgba8::MISTBLUE, Rgba8::WHITE, "quit","Quit");
		m_attractWidget->AddChild(*exerciseButton);
		m_attractWidget->AddChild(*remoteButton);
		m_attractWidget->AddChild(*exit);
	}
	m_attractWidget->SetEnabled(true);
}

void Game::InitializeWidgetsForLobby()
{
	if (!m_lobbyWidget[0])
	{
		m_lobbyWidget[0] = new Widget(m_screenCamera, AABB2(Vec2(10.f,10.f) ,Vec2(1590.f, 790.f)), "", Rgba8::AQUA);
		Button* set1B = new Button(m_lobbyWidget[0], AABB2(Vec2(200.f, 200.f), Vec2(500.f, 500.f))
					, Rgba8::WHITE, Rgba8::WHITE, "modelset1selection","", Vec2(0.5f, 0.5f), "Data/Images/Set1.png");
		Button* set2B = new Button(m_lobbyWidget[0], AABB2(Vec2(650.f, 200.f), Vec2(950.f, 500.f))
					, Rgba8::WHITE, Rgba8::WHITE, "modelset2selection","", Vec2(0.5f, 0.5f), "Data/Images/Set2.png");
		Button* set3B = new Button(m_lobbyWidget[0], AABB2(Vec2(1100.f, 200.f), Vec2(1400.f, 500.f))
					, Rgba8::WHITE, Rgba8::WHITE, "modelset3selection","", Vec2(0.5f, 0.5f), "Data/Images/Set3.png");
		m_lobbyWidget[0]->AddChild(*set1B);
		m_lobbyWidget[0]->AddChild(*set2B);
		m_lobbyWidget[0]->AddChild(*set3B);
		m_lobbyWidget[0]->SetEnabled(true);
	}
	if (m_isRemote == true)
	{
		if (!m_lobbyWidget[1])
		{
			m_lobbyWidget[1] = new Widget(m_screenCamera, AABB2(Vec2(10.f,10.f) ,Vec2(1590.f, 790.f)), "", Rgba8::MISTBLUE);
			Button* server = new Button(m_lobbyWidget[1], AABB2(Vec2(300.f, 200.f), Vec2(650.f, 500.f))
						, Rgba8::LAVENDER, Rgba8::WHITE, "servermodeselection","Server");
			Button* client = new Button(m_lobbyWidget[1], AABB2(Vec2(950.f, 200.f), Vec2(1300.f, 500.f))
						, Rgba8::LAVENDER, Rgba8::WHITE, "clientmodeselection","Client");
			m_lobbyWidget[1]->AddChild(*server);
			m_lobbyWidget[1]->AddChild(*client);
			m_lobbyWidget[1]->SetEnabled(false);
		}
	}
}

void Game::RegisterAllWidgetEvents()
{
	g_theEventSystem->SubscribeEventCallBackFunction("remotemodeselection", OnRemoteModeSelection);
	g_theEventSystem->SubscribeEventCallBackFunction("localmodeselection", OnLocalModeSelection);
	g_theEventSystem->SubscribeEventCallBackFunction("modelset1selection", OnModelSet1Selection);
	g_theEventSystem->SubscribeEventCallBackFunction("modelset2selection", OnModelSet2Selection);
	g_theEventSystem->SubscribeEventCallBackFunction("modelset3selection", OnModelSet3Selection);
	g_theEventSystem->SubscribeEventCallBackFunction("servermodeselection", OnServerModeSelection);
	g_theEventSystem->SubscribeEventCallBackFunction("clientmodeselection", OnClientModeSelection);
}

void Game::UpdateFirstPersonRaycast()
{
	Vec3 playerI, playerJ, playerK;
	m_player->m_orientation.GetAsVectors_IFwd_JLeft_KUp(playerI, playerJ, playerK);
	m_cameraRay.m_rayFwdNormal = playerI;
	m_cameraRay.m_rayMaxLength = 10.f;
	m_cameraRay.m_rayStartPos = m_player->m_position;
}

void Game::AttractStateUpdate()
{
	// if (g_theApp->IsKeyDown(' ') ||
	// 	g_theApp->IsKeyDown('N') ||
	// 	g_theInput->GetController(0).WasButtonJustPressed(XboxButtonID::START) ||
	// 	g_theInput->GetController(0).WasButtonJustPressed(XboxButtonID::A))
	// {
	// 	//m_isInAttractMode = false;
	// 	ChangeState(GameState::LOBBY);
	// }
	m_attractWidget->Update();
}

void Game::PlayingStateUpdate()
{
	UpdateDebugInt();
	UpdateFirstPersonRaycast();
	m_player->Update((float)s_theSystemClock->GetDeltaSeconds());
	DebugRenderSystemInputUpdate();
	if (m_chessReferee)
	{
		m_chessReferee->Update((float)s_theSystemClock->GetDeltaSeconds());
		m_promoteWidget->Update();
	}

}

void Game::LobbyStateUpdate()
{
	if (!m_isRemote)
	{
		m_lobbyWidget[0]->Update();
	}
	else
	{
		m_lobbyWidget[0]->Update();
		m_lobbyWidget[1]->Update();
	}
}

void Game::UpdateDebugInt()
{
	if (g_theApp->WasKeyJustPressed('1'))
	{
		m_debugInt = 1;
		return;
	}
	else if (g_theApp->WasKeyJustPressed('2'))
	{
		m_debugInt = 2;
		return;
	}
	else if (g_theApp->WasKeyJustPressed('3'))
	{
		m_debugInt = 3;
		return;
	}
	else if (g_theApp->WasKeyJustPressed('4'))
	{
		m_debugInt = 4;
		return;
	}
	else if (g_theApp->WasKeyJustPressed('5'))
	{
		m_debugInt = 5;
		return;
	}
	else if (g_theApp->WasKeyJustPressed('6'))
	{
		m_debugInt = 6;
		return;
	}
	else if (g_theApp->WasKeyJustPressed('7'))
	{
		m_debugInt = 7;
		return;
	}
	else if (g_theApp->WasKeyJustPressed('8'))
	{
		m_debugInt = 8;
		return;
	}
	else if (g_theApp->WasKeyJustPressed('9'))
	{
		m_debugInt = 9;
		return;
	}
	else if (g_theApp->WasKeyJustPressed('0'))
	{
		m_debugInt = 0;
		return;
	}
	else if (g_theApp->WasKeyJustPressed(KEYCODE_RIGHTARROW))
	{
		if (m_debugInt == 19)
		{
			m_debugInt = 0;
			return;
		}
		m_debugInt ++;
		return;
	}
	else if (g_theApp->WasKeyJustPressed(KEYCODE_LEFTARROW))
	{
		if (m_debugInt == 0)
		{
			m_debugInt = 19;
			return;
		}
		m_debugInt --;
		return;
	}
}

std::string Game::GetDebugRenderPrintByInt() const
{
	if (m_debugInt == 0)
	{
		return "DebugInt = 0; render mode is Lit (including normal maps)";
	}
	if (m_debugInt == 1)
	{
		return "DebugInt = 1; render mode is Diffuse Texel only";
	}
	else if (m_debugInt == 2)
	{
		return "DebugInt = 2; render mode is Vertex Color only (C)";
	}
	else if (m_debugInt == 3)
	{
		return "DebugInt = 3; render mode is UV TexCoords only (V)";
	}
	else if (m_debugInt == 4)
	{
		return "DebugInt = 4; render mode is Vertex Tangents: raw, in Model Space (T)";
	}
	else if (m_debugInt == 5)
	{
		return "DebugInt = 5; render mode is Vertex Bitangents: raw, in Model Space (B)";
	}
	else if (m_debugInt == 6)
	{
		return "DebugInt = 6; render mode is Vertex Normals: raw, in Model Space (N)";
	}
	else if (m_debugInt == 7)
	{
		return "DebugInt = 7; render mode is Normal Map texel only";
	}
	else if (m_debugInt == 8)
	{
		return "DebugInt = 8; render mode is Pixel Normal in TBN space (decoded, raw)";
	}
	else if (m_debugInt == 9)
	{
		return "DebugInt = 9; render mode is Pixel Normal in World space (decoded, transformed)";
	}
	else if (m_debugInt == 10)
	{
		return "DebugInt = 10; render mode is Lit, but without normal maps";
	}
	else if (m_debugInt == 11)
	{
		return "DebugInt = 11; render mode is Light strength (vs, pixel normal in world space)";
	}
	else if (m_debugInt == 12)
	{
		return "DebugInt = 12; render mode is Light strength (vs, vertex/surface normals only)";
	}
	else if (m_debugInt == 13)
	{
		return "No render mode for debugInt=13";
	}
	else if (m_debugInt == 14)
	{
		return "DebugInt = 14; render mode is Vertex Tangents: transformed, into World space (T)";
	}
	else if (m_debugInt == 15)
	{
		return "DebugInt = 15; render mode is Vertex Bitangents: transformed, into World space (B)";
	}
	else if (m_debugInt == 16)
	{
		return "DebugInt = 16; render mode is Vertex Normals: transformed, into World space (N)";
	}
	else if (m_debugInt == 17)
	{
		return "DebugInt = 17; render mode is ModelToWorld I (forward) basis vector, in World space (I)";
	}
	else if (m_debugInt == 18)
	{
		return "DebugInt = 18; render mode is ModelToWorld J (left) basis vector, in World space (J)";
	}
	else if (m_debugInt == 19)
	{
		return "DebugInt = 19; render mode is ModelToWorld K (up) basis vector, in World space (K)";
	}

	return "UNKNOWN";
}

bool Game::OnRemoteModeSelection(EventArgs& args)
{
	UNUSED(args)
	g_theGame->m_isRemote = true;
	g_theGame->ChangeState(GameState::LOBBY);
	return true;
}

bool Game::OnLocalModeSelection(EventArgs& args)
{
	UNUSED(args)
	g_theGame->m_isRemote = false;
	g_theGame->ChangeState(GameState::LOBBY);
	return true;
}

bool Game::OnModelSet1Selection(EventArgs& args)
{
	UNUSED(args)
	if (!g_theGame->m_isRemote)
		g_theGame->ChangeState(GameState::PLAYING);
	else
	{
		g_theGame->m_lobbyWidget[0]->SetEnabled(false);
		g_theGame->m_lobbyWidget[1]->SetEnabled(true);
	}
	g_theGame->m_setSelected = 0;
	return true;
}

bool Game::OnModelSet2Selection(EventArgs& args)
{
	UNUSED(args)
	if (!g_theGame->m_isRemote)
		g_theGame->ChangeState(GameState::PLAYING);
	else
	{
		g_theGame->m_lobbyWidget[0]->SetEnabled(false);
		g_theGame->m_lobbyWidget[1]->SetEnabled(true);
	}
	g_theGame->m_setSelected = 1;
	return true;
}

bool Game::OnModelSet3Selection(EventArgs& args)
{
	UNUSED(args)
	if (!g_theGame->m_isRemote)
		g_theGame->ChangeState(GameState::PLAYING);
	else
	{
		g_theGame->m_lobbyWidget[0]->SetEnabled(false);
		g_theGame->m_lobbyWidget[1]->SetEnabled(true);
	}
	g_theGame->m_setSelected = 2;
	return true;
}

bool Game::OnServerModeSelection(EventArgs& args)
{
	UNUSED(args)
	g_theNetworkSystem->SetMode(NetworkMode::SERVER);
	g_theGame->ChangeState(GameState::PLAYING);
	g_theDevConsole->AddLine(Rgba8::YELLOW, "Enter the game as a server. Use cmd to connect to client!");
	return true;
}

bool Game::OnClientModeSelection(EventArgs& args)
{
	UNUSED(args)
	g_theNetworkSystem->SetMode(NetworkMode::CLIENT);
	g_theDevConsole->AddLine(Rgba8::YELLOW, "Enter the game as a client. Use cmd to connect to server!");
	g_theGame->ChangeState(GameState::PLAYING);
	return true;
}

void Game::LoadAnObj()
{
	//m_testMesh = new StaticMesh(g_theRenderer, "Data/Models/Woman");

	// Mat44 mat;
	// mat = mat.MakeUniformScale3D(0.02f);
	// m_testMesh->m_transform.Append(mat);
}

void Game::AttractModeRender() const
{
	std::vector<Vertex_PCU> verts;
	g_theRenderer->ClearScreen();
	g_theRenderer->BeginCamera(m_screenCamera);
	g_theRenderer->BindTexture(m_attractCover);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	//DebugDrawRing(Vec2(800.f, 400.f), 100.f, 10.f + sinf(m_varyTime) * 10.f, Rgba8::YELLOW);
	g_theRenderer->DrawAABB2(m_screenCamera.GetOrthographicBounds(), Rgba8::WHITE);
	AddVertsForTextTriangles2D(verts, "Chess Soul", Vec2(650.f, 600.f), 65.f, Rgba8::WHITE);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(verts);
	m_attractWidget->Render();
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::PlayingModeRender() const
{
	g_theRenderer->ClearScreen(Rgba8(0, 0, 0));
	//g_theRenderer->ClearScreen();
	g_theRenderer->BeginCamera(((Player*)m_player)->m_worldCamera);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
	//g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);

	m_chessReferee->Render();
	
	//g_theRenderer->DrawVertexIndexArray(m_testMesh->m_verts, m_testMesh->m_indices);

	//g_theRenderer->BindTexture(nullptr);
	//g_theRenderer->SetModelConstants();
	//g_theRenderer->DrawVertexArray(m_gridVertexes);
	g_theRenderer->EndCamera(((Player*)m_player)->m_worldCamera);

	
	g_theRenderer->BeginCamera(m_screenCamera);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	DebugDrawRing(Vec2(800.f, 400.f), 5.f, 1.f, Rgba8::LAVENDER);
	m_promoteWidget->Render();
	g_theRenderer->EndCamera(m_screenCamera);
	

	if (m_hasWon)
	{
		std::vector<Vertex_PCU> verts;
		AddVertsForAABB2D(verts, m_screenCamera.GetOrthographicBounds(), Rgba8(120,120,120,120));
		AddVertsForTextTriangles2D(verts, "Win!", Vec2(720.f, 385.f),
				40.f, Rgba8::YELLOW);
		g_theRenderer->BeginCamera(m_screenCamera);
		//g_theRenderer->BindTexture(nullptr);
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
		g_theRenderer->DrawVertexArray(verts);
		g_theRenderer->EndCamera(m_screenCamera);
	}
	else
	{
		std::vector<Vertex_PCU> verts;
		// AddVertsForTextTriangles2D(verts, "Use the DevConsole (~) to enter commands", Vec2(3.f, 785.f),
		// 		12.f, Rgba8::YELLOW);
		AddVertsForTextTriangles2D(verts, GetDebugRenderPrintByInt(), Vec2(3.f, 785.f),
				12.f, Rgba8::YELLOW);
		if (m_player->m_mode == CameraMode::FREE)
		{
			AddVertsForTextTriangles2D(verts, "CameraMode (F4) Free | GameState: First Player's Turn", Vec2(3.f, 771.f),
				12.f, Rgba8::MINTGREEN);
		}
		if (m_player->m_mode == CameraMode::AUTO)
		{
			AddVertsForTextTriangles2D(verts, "CameraMode (F4) Auto | GameState: First Player's Turn", Vec2(3.f, 771.f),
				12.f, Rgba8::MINTGREEN);
		}
		g_theRenderer->BeginCamera(m_screenCamera);
		//g_theRenderer->BindTexture(nullptr);
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindTexture(nullptr,1);
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
		g_theRenderer->DrawVertexArray(verts);
		g_theRenderer->EndCamera(m_screenCamera);
	}

	DebugRenderWorld(((Player*)m_player)->m_worldCamera);
	DebugRenderScreen(m_screenCamera);
}

void Game::LobbyModeRender() const
{
	g_theRenderer->ClearScreen(Rgba8(0,0,0));
	g_theRenderer->BeginCamera(m_screenCamera);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	for (Widget* w : m_lobbyWidget)
	{
		if (w)
			w->Render();
	}
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::PrintGameControlToDevConsole()
{
	//g_theDevConsole->AddLine(Rgba8::BLUE, "Type help for a list of commands");
	//g_theDevConsole->AddLine(Rgba8::GREEN, std::string("This is a client"));
}

void Game::DrawSquareXYGrid(int unit /*= 100*/)
{
	m_gridVertexes.clear();

	const int GRID_SIZE = unit * unit;
	const float LINE_THICKNESS = 0.03f;
	const float BOLD_LINE_THICKNESS = 0.05f;
	const float ORIGIN_LINE_THICKNESS = 0.1f;

	for (int x = -GRID_SIZE / 2; x <= GRID_SIZE / 2; ++x)
	{
		float thickness = LINE_THICKNESS;
		Rgba8 color = Rgba8::GREY; 

		if (x % 5 == 0)
		{
			thickness = BOLD_LINE_THICKNESS;
			color = Rgba8::RED; 
		}
		if (x == 0)
		{
			thickness = ORIGIN_LINE_THICKNESS;
			color = Rgba8(255, 50, 50, 255);
		}

		AABB3 bounds(Vec3(-GRID_SIZE / 2.f, (float)x - thickness / 2.f, -thickness / 2.f),
			Vec3(GRID_SIZE / 2.f, (float)x + thickness / 2.f, thickness / 2.f));

		AddVertsForAABB3D(m_gridVertexes, bounds, color, AABB2::ZERO_TO_ONE);
	}

	for (int y = -GRID_SIZE / 2; y <= GRID_SIZE / 2; ++y)
	{
		float thickness = LINE_THICKNESS * 1.1f;
		Rgba8 color = Rgba8::GREY;

		if (y % 5 == 0)
		{
			thickness = BOLD_LINE_THICKNESS * 1.1f;
			color = Rgba8::GREEN; 
		}
		if (y == 0)
		{
			thickness = ORIGIN_LINE_THICKNESS * 1.1f;
			color = Rgba8(50, 255, 50, 255);
		}

		AABB3 bounds(Vec3((float)y - thickness / 2.f, -GRID_SIZE / 2.f , -thickness/2.f),
			Vec3((float)y + thickness / 2.f, GRID_SIZE / 2.f, thickness/2.f));

		AddVertsForAABB3D(m_gridVertexes, bounds, color, AABB2::ZERO_TO_ONE);
	}
}

void Game::DebugRenderSystemInputUpdate()
{
	Vec3 pos = m_player->m_position;
	/*std::string reportHUD = " Player Position: " +
		RoundToOneDecimalString(pos.x) + ", " + RoundToOneDecimalString(pos.y) + ", " + RoundToOneDecimalString(pos.z);
	DebugAddMessage(reportHUD, 0.f, m_screenCamera);*/

	if (g_theApp->WasKeyJustPressed('1'))
	{
		Vec3 playerI = m_player->GetModelToWorldTransform().GetIBasis3D();
		Vec3 end = pos + playerI * 20.f;
		DebugAddWorldLine(pos, end, 0.1f, 10.f, Rgba8::YELLOW, Rgba8::YELLOW, DebugRenderMode::X_RAY);
	}
	if (g_theApp->IsKeyDown('2'))
	{
		DebugAddWorldPoint(Vec3(pos.x,pos.y,0.f), 0.2f, 60.f, Rgba8(150, 75, 0), Rgba8(150, 75, 0), DebugRenderMode::USE_DEPTH);
	}
	if (g_theApp->WasKeyJustPressed('3'))
	{
		Vec3 playerI = m_player->GetModelToWorldTransform().GetIBasis3D();
		Vec3 center = pos + playerI * 2.f;
		DebugAddWorldWireSphere(center, 1.f, 5.f, Rgba8::GREEN, Rgba8::RED);//, DebugRenderMode::USE_DEPTH);
	}
	if (g_theApp->WasKeyJustPressed('4'))
	{
		Mat44 playerMat = m_player->GetModelToWorldTransform();
		Vec3 playerI = m_player->GetModelToWorldTransform().GetIBasis3D().GetNormalized();
		Vec3 playerJ = m_player->GetModelToWorldTransform().GetJBasis3D().GetNormalized();
		Vec3 playerK = m_player->GetModelToWorldTransform().GetKBasis3D().GetNormalized();
		playerMat.SetIJK3D(playerI, playerJ, playerK);
		/*Mat44 mat;
		mat.SetIJK3D(Vec3(0.f, -1.f, 0.f), Vec3(0.f, 0.f, 1.f), Vec3(1.f, 0.f, 0.f));
		playerMat.Append(mat);*/
		//DebugAddWorldBasis(playerMat, 20.f);//, DebugRenderMode::USE_DEPTH);
		Vec3 newK = pos + playerK;
		Vec3 newI = pos + playerI;
		Vec3 newJ = pos + playerJ;
		DebugAddWorldArrow(pos, newK, 0.05f, 20.f, Rgba8::AQUA, Rgba8::AQUA);
		DebugAddWorldArrow(pos, newI, 0.05f, 20.f, Rgba8::MAGENTA, Rgba8::MAGENTA);
		DebugAddWorldArrow(pos, newJ, 0.05f, 20.f, Rgba8::MINTGREEN, Rgba8::MINTGREEN);
	}
	if (g_theApp->WasKeyJustPressed('5'))
	{
		EulerAngles ori = m_player->m_orientation;
		Vec3 playerI = m_player->GetModelToWorldTransform().GetIBasis3D().GetNormalized();
		Vec3 pivot = pos + playerI * 2.f;
		/*std::string report = "Position: " +
			std::to_string(RoundToOneDecimal(pos.x)) + ", " + std::to_string(RoundToOneDecimal(pos.y)) + ", " + std::to_string(RoundToOneDecimal(pos.z)) +
			" Orientation: " + std::to_string(RoundToOneDecimal(ori.m_yawDegrees)) + ", " + std::to_string(RoundToOneDecimal(ori.m_pitchDegrees)) + ", "
			+ std::to_string(RoundToOneDecimal(ori.m_rollDegrees));*/
		std::string report = "Position: " +
			RoundToOneDecimalString(pos.x) + ", " + RoundToOneDecimalString(pos.y) + ", " + RoundToOneDecimalString(pos.z) +
			" Orientation: " + RoundToOneDecimalString(ori.m_yawDegrees) + ", " + RoundToOneDecimalString(ori.m_pitchDegrees) + ", "
			+ RoundToOneDecimalString(ori.m_rollDegrees);

		DebugAddWorldBillboardText(report, pivot, 0.2f, Vec2(0.25f, 0.25f), 10.f, Rgba8::WHITE);// , DebugRenderMode::USE_DEPTH);
	}
	if (g_theApp->WasKeyJustPressed('6'))
	{
		DebugAddWorldWireCylinder(pos, pos + Vec3(0.f, 0.f, 2.f), 0.5f, 10.f, Rgba8::WHITE, Rgba8::RED);
	}
	if (g_theApp->WasKeyJustPressed('7'))
	{
		EulerAngles ori = m_player->m_orientation;
		std::string report = " Camera Orientation: " + RoundToOneDecimalString(ori.m_yawDegrees) + ", " + RoundToOneDecimalString(ori.m_pitchDegrees) + ", "
			+ RoundToOneDecimalString(ori.m_rollDegrees);
		DebugAddMessage(report, 5.f, m_screenCamera);
	}

	//Add fps...
	float timeTotal = (float)m_gameClock->GetTotalSeconds();
	float fps = (float)m_gameClock->GetFrameRate();
	float timeScale = m_gameClock->GetTimeScale();
	std::string timeReportHUD = " Time: " + RoundToTwoDecimalsString(timeTotal) 
	+ " FPS: " + RoundToOneDecimalString(fps) + " Scale: " + RoundToTwoDecimalsString(timeScale);
	float textWidth = GetTextWidth(12.f, timeReportHUD, 0.7f);
	DebugAddScreenText(timeReportHUD, m_screenCamera.GetOrthographicTopRight() - Vec2(textWidth + 1.f, 15.f), 12.f, Vec2::ZERO, 0.f);
}

void Game::DebugAddWorldAxisText(Mat44 worldMat)
{
	Mat44 xMat;
	xMat.SetIJK3D(Vec3(-1.f, 0.f, 0.f), Vec3(0.f, 0.f, 1.f), Vec3(0.f, 1.f, 0.f));
	xMat.Append(worldMat);
	DebugAddWorldText("x-Forward", xMat, 0.2f, Vec2(-0.05f, -0.3f), -1.f, Rgba8::MAGENTA);

	Mat44 yMat;
	yMat.SetIJK3D(Vec3(0.f, 1.f, 0.f), Vec3(0.f, 0.f, 1.f), Vec3(1.f, 0.f, 0.f));
	yMat.Append(worldMat);
	DebugAddWorldText("y-Left", yMat, 0.2f, Vec2( 1.f,-0.3f), -1.f, Rgba8::MINTGREEN);

	Mat44 zMat;
	zMat.SetIJK3D(Vec3(0.f, 0.f, -1.f), Vec3(0.f, 1.f, 0.f), Vec3(1.f, 0.f, 0.f));
	zMat.Append(worldMat);
	DebugAddWorldText("z-Up", zMat, 0.2f, Vec2(-0.3f, 1.5f), -1.f, Rgba8::AQUA);
}
