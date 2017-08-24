#ifndef CAMERA_HPP
#define CAMERA_HPP


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Camera {
	Camera() {
		m_center.x = 0.0f;
		m_center.y = 0.0f;
		m_extent = 25.0f;
		m_zoom = 1.0f;
		m_width = 1280;
		m_height = 800;
	}

	glm::vec2 ConvertScreenToWorld(const glm::vec2 &screenPoint);

	glm::vec2 ConvertWorldToScreen(const glm::vec2 &worldPoint);

	void BuildProjectionMatrix(float *m, float zBias);

	glm::vec2 m_center;
	float m_extent;
	float m_zoom;
	int m_width;
	int m_height;
};

#endif