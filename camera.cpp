#include "camera.hpp"
#include <iostream>
#include "graphics.hpp"

float geoRatio = 1.f;
Camera g_camera;
glm::vec2 Camera::ConvertScreenToWorld(const glm::vec2 &ps) {
	float w = float(m_width);
	float h = float(m_height);
	float u =  ps.x / w;
	float v =  (h - ps.y) / h;

	float ratio =geoRatio * w / h;
	glm::vec2 extents(ratio * m_span, m_span);


	extents *= m_zoom;

	glm::vec2 lower = m_center - extents;
	glm::vec2 upper = m_center + extents;

	glm::mat4 rotN;

	rotN = glm::rotate(rotN, glm::radians(angleNorth), glm::vec3(0.0f, 0.0f, 1.0f));

	//glm::mat4 = tr1

	
	glm::mat4 Model = glm::ortho(lower.x,upper.x,lower.y,upper.y,-1.f,1.f);

	glm::mat4 trans = rotN*Model;


	glm::vec2 pw;

	
	pw.x =  (1.0f - u) * lower.x + u * upper.x;
	pw.y =  (1.0f - v) * lower.y + v * upper.y;


	glm::vec4 uvr = glm::inverse(trans)*glm::vec4(u,v,0.f,1.f);


	glm::mat4 view = glm::mat4();
	glm::mat4 projection = glm::ortho(lower.x,upper.x,lower.y,upper.y,-1.f,1.f);
	glm::vec4 viewport = glm::vec4(0, 0, w, h);
	glm::vec3 wincoord = glm::vec3(ps.x, h - ps.y, 0.0f);
	glm::vec3 objcoord = glm::unProject(wincoord, view, rotN*projection, viewport);
	//glm::vec4 res = rotN*glm::vec4(objcoord,1.f);
	
	//return glm::vec2(res.x,res.y);

	return objcoord;

//return glm::vec2(uvr.x,uvr.y);

	//return pw;
}

//
glm::vec2 Camera::ConvertWorldToScreen(const glm::vec2 &pw) {
	float w = float(m_width);
	float h = float(m_height);
	float ratio = geoRatio* w / h;
	glm::vec2 extents(ratio * m_span, m_span);
	extents *= m_zoom;

	glm::vec2 lower = m_center - extents;
	glm::vec2 upper = m_center + extents;

	float u = (pw.x - lower.x) / (upper.x - lower.x);
	float v = (pw.y - lower.y) / (upper.y - lower.y);

	glm::vec2 ps;
	ps.x = u * w;
	ps.y = (1.0f - v) * h;

	//glm::mat2 r1 (cos(-angleNorth), -sin(-angleNorth), sin(-angleNorth), cos(-angleNorth) );
	return ps;
}

// Convert from world coordinates to normalized device coordinates.
// http://www.songho.ca/opengl/gl_projectionmatrix.html
glm::mat4 Camera::BuildProjectionMatrix() {
	float w = float(m_width);
	float h = float(m_height);

	//std::cout << w << "," << h << "\n";
	
	float ratio =  geoRatio* w / h;

	glm::vec2 extents(ratio * m_span, m_span);
	extents *= m_zoom;

	glm::vec2 lower = m_center - extents;
	glm::vec2 upper = m_center + extents;

	glm::mat4 Model = glm::ortho(lower.x,upper.x,lower.y,upper.y,-1.f,1.f);

	//std::cout << lower.x << "," << upper.x << "," << lower.y << "," << upper.y << "\n";
	
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
