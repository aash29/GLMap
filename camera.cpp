#include "camera.hpp"
#include <iostream>
glm::vec2 Camera::ConvertScreenToWorld(const glm::vec2 &ps) {
	float w = float(m_width);
	float h = float(m_height);
	float u = ps.x / w;
	float v = (h - ps.y) / h;

	float ratio = w / h;
	glm::vec2 extents(ratio * m_span, m_span);
	extents *= m_zoom;

	glm::vec2 lower = m_center - extents;
	glm::vec2 upper = m_center + extents;

	glm::vec2 pw;
	pw.x = (1.0f - u) * lower.x + u * upper.x;
	pw.y = (1.0f - v) * lower.y + v * upper.y;
	return pw;
}

//
glm::vec2 Camera::ConvertWorldToScreen(const glm::vec2 &pw) {
	float w = float(m_width);
	float h = float(m_height);
	float ratio = w / h;
	glm::vec2 extents(ratio * m_span, m_span);
	extents *= m_zoom;

	glm::vec2 lower = m_center - extents;
	glm::vec2 upper = m_center + extents;

	float u = (pw.x - lower.x) / (upper.x - lower.x);
	float v = (pw.y - lower.y) / (upper.y - lower.y);

	glm::vec2 ps;
	ps.x = u * w;
	ps.y = (1.0f - v) * h;
	return ps;
}

// Convert from world coordinates to normalized device coordinates.
// http://www.songho.ca/opengl/gl_projectionmatrix.html
glm::mat4 Camera::BuildProjectionMatrix() {
	float w = float(m_width);
	float h = float(m_height);

	std::cout << w << "," << h << "\n";
	
	float ratio =  w / h;
	glm::vec2 extents(ratio * m_span, m_span);
	extents *= m_zoom;

	glm::vec2 lower = m_center - extents;
	glm::vec2 upper = m_center + extents;

	glm::mat4 Model = glm::ortho(lower.x,upper.x,lower.y,upper.y,-1.f,1.f);

	std::cout << lower.x << "," << upper.x << "," << lower.y << "," << upper.y << "\n";
	
	return Model;
	
	
	/*
	m[0] = 2.0f / (upper.x - lower.x);
	m[1] = 0.0f;
	m[2] = 0.0f;
	m[3] = 0.0f;

	m[4] = 0.0f;
	m[5] = 2.0f / (upper.y - lower.y);
	m[6] = 0.0f;
	m[7] = 0.0f;

	m[8] = 0.0f;
	m[9] = 0.0f;
	m[10] = 1.0f;
	m[11] = 0.0f;

	m[12] = -(upper.x + lower.x) / (upper.x - lower.x);
	m[13] = -(upper.y + lower.y) / (upper.y - lower.y);
	m[14] = zBias;
	m[15] = 1.0f;
	*/

	

	
  
}
