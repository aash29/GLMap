#ifndef CAMERA_HPP
#define CAMERA_HPP


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Camera {
  Camera() {
    m_center.x = 0.5f;
    m_center.y = 0.5f;
    m_span = 1.0f;
    m_zoom = 1.0f;
    m_width = 1280;
    m_height = 800;
  }

  glm::vec2 ConvertScreenToWorld(const glm::vec2 &screenPoint);

  glm::vec2 ConvertWorldToScreen(const glm::vec2 &worldPoint);

  glm::mat4 BuildProjectionMatrix();

  glm::vec2 m_center;
  float m_span;
  float m_zoom;
  int m_width;
  int m_height;

  static constexpr float gridSize = 1.f;
  
};
extern Camera g_camera;
extern float geoRatio;

#endif
