#pragma once
#include "App.hpp"
#include "Gamecommon.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/Camera.hpp"

class Game;
class Camera;

enum class CameraMode
{
	AUTO,
	FREE,
	COUNT
};

class Player
{
public:
	Player(Game* owner);
	~Player();

	void Update(float deltaSeconds);
	void Render() const;

	void UpdateFreeCamera(float deltaSeconds);
	void UpdateAutoCamera();
	void ToggleCameraMode();

	Vec3 GetForwardVectorDueToOrientation() const;
	Vec3 GetLeftVectorDueToOrientation() const;
	Mat44 GetModelToWorldTransform() const;

	void ResetPositionAndOrientation(Vec3 pos, EulerAngles orientation);

public:
	Game* m_game = nullptr;
	CameraMode m_mode = CameraMode::FREE;
	Vec3 m_position;
	EulerAngles m_orientation;
	EulerAngles m_angularVelocity;
	Vec3 m_velocity;
	
	Vec3 m_originV;
	float m_originYaw;
	float m_originPitch;
	float m_originRoll;

	Camera m_worldCamera;
};


