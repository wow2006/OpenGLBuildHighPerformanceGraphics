#pragma once
// GLM
#include <glm/gtc/matrix_transform.hpp>
// Internal
#include "Plane.hpp"

class CAbstractCamera {
public:
  CAbstractCamera();

  virtual ~CAbstractCamera();

  void SetupProjection(const float fovy, const float aspectRatio,
                       const float near = 0.1f, const float far = 1000.0f);

  virtual void Update() = 0;

  virtual void Rotate(const float yaw, const float pitch, const float roll);

  const glm::mat4 GetViewMatrix() const;

  const glm::mat4 GetProjectionMatrix() const;

  void SetPosition(const glm::vec3 &v);

  const glm::vec3 GetPosition() const;

  void SetFOV(const float fov);

  float GetFOV() const;

  float GetAspectRatio() const;

  void CalcFrustumPlanes();

  bool IsPointInFrustum(const glm::vec3 &point);

  bool IsSphereInFrustum(const glm::vec3 &center, const float radius);

  bool IsBoxInFrustum(const glm::vec3 &min, const glm::vec3 &max);

  void GetFrustumPlanes(glm::vec4 planes[6]);

  // frustum points
  glm::vec3 farPts[4];

  glm::vec3 nearPts[4];

protected:
  const glm::vec3 UP = glm::vec3{0, 1, 0};

  float yaw, pitch, roll, fov, aspect_ratio, Znear, Zfar;

  glm::vec3 look;
  glm::vec3 up;
  glm::vec3 right;
  glm::vec3 position;

  glm::mat4 V; // view matrix
  glm::mat4 P; // projection matrix

  // Frsutum planes
  CPlane planes[6];
};
