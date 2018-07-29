#pragma once
#include "AbstractCamera.hpp"

class CTargetCamera : public CAbstractCamera {
public:
  CTargetCamera();

  ~CTargetCamera();

  void Update();

  void Rotate(const float yaw, const float pitch, const float roll);

  void SetTarget(const glm::vec3 tgt);

  const glm::vec3 GetTarget() const;

  void Pan(const float dx, const float dy);

  void Zoom(const float amount);

  void Move(const float dx, const float dz);

protected:
  glm::vec3 target;

  static constexpr float maxRy =  60.f;
  static constexpr float minRy = -60.f;

  static constexpr float minDistance = 1.f;
  static constexpr float maxDistance = 10.f;

  float distance;
};
