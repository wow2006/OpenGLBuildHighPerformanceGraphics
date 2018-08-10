#pragma once
#include "AbstractCamera.hpp"

class CFreeCamera : public CAbstractCamera {
public:
  CFreeCamera();

  ~CFreeCamera() override;

  void Update() override;

  void Walk(const float dt);

  void Strafe(const float dt);

  void Lift(const float dt);

  void SetTranslation(const glm::vec3 &t);

  glm::vec3 GetTranslation() const;

  void SetSpeed(const float speed);

  float GetSpeed() const;

protected:
  float speed; // move speed of camera in m/s
  glm::vec3 translation;
};
