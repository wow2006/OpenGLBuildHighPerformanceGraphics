// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <cmath>
#include "TargetCamera.hpp"
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

constexpr float CTargetCamera::minRy;
constexpr float CTargetCamera::maxRy;

constexpr float CTargetCamera::minDistance;
constexpr float CTargetCamera::maxDistance;

CTargetCamera::CTargetCamera() {
  right       = glm::vec3(1, 0, 0);
  up          = glm::vec3(0, 1, 0);
  look        = glm::vec3(0, 0,-1);
  distance    = 0.0f;
}

CTargetCamera::~CTargetCamera() = default;
 
void CTargetCamera::Update() {
	const glm::mat4 R = glm::yawPitchRoll(yaw,pitch,0.0f);
	glm::vec3 T       = glm::vec3(0,0,distance);

	T = glm::vec3(R*glm::vec4(T,0.0f));
	position = target + T;

	look  = glm::normalize(target-position);
	up    = glm::vec3(R*glm::vec4(UP,0.0f));
	right = glm::cross(look, up);

	V = glm::lookAt(position, target, up); 
}

void CTargetCamera::SetTarget(const glm::vec3 tgt) {
	target   = tgt;
	distance = glm::distance(position, target);
	distance = std::max(minDistance, std::min(distance, maxDistance));
}

const glm::vec3 CTargetCamera::GetTarget() const {
	return target;
} 

void CTargetCamera::Rotate(const float yaw, const float pitch, const float roll) {
 	const float p = std::min( std::max(pitch, minRy), maxRy);
	CAbstractCamera::Rotate(yaw, p, roll); 
}
 
void CTargetCamera::Pan(const float dx, const float dy) {
	const glm::vec3 X = right*dx;
	const glm::vec3 Y = up*dy;

	position += X + Y;
	  target += X + Y;

	Update();
}

void CTargetCamera::Zoom(const float amount) { 
	position += look * amount;
	distance  = glm::distance(position, target); 
	distance  = std::max(minDistance, std::min(distance, maxDistance));

	Update();
}
 
void CTargetCamera::Move(const float dx, const float dy) {
	const auto X = right * dx;
	const auto Y = look  * dy;

	position += X + Y;
	target   += X + Y;

	Update();
}
