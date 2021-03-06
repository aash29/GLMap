/*
* Copyright (c) 2006-2013 Erin Catto http://www.box2d.org
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#include "DebugDraw.h"

#if defined(__APPLE_CC__)
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdarg.h>

#include "imgui.h"

#include "RenderGL3.h"

#define BUFFER_OFFSET(x)  ((const void*) (x))

DebugDraw g_debugDraw;
Camera g_camera;

//
b2Vec2 Camera::ConvertScreenToWorld(const b2Vec2& ps)
{
    float32 w = float32(m_width);
    float32 h = float32(m_height);
	float32 u = ps.x / w;
	float32 v = (h - ps.y) / h;

	float32 ratio = w / h;
	b2Vec2 extents(ratio * 25.0f, 25.0f);
	extents *= m_zoom;

	b2Vec2 lower = m_center - extents;
	b2Vec2 upper = m_center + extents;

	b2Vec2 pw;
	pw.x = (1.0f - u) * lower.x + u * upper.x;
	pw.y = (1.0f - v) * lower.y + v * upper.y;
	return pw;
}

//
b2Vec2 Camera::ConvertWorldToScreen(const b2Vec2& pw)
{
	float32 w = float32(m_width);
	float32 h = float32(m_height);
	float32 ratio = w / h;
	b2Vec2 extents(ratio * 25.0f, 25.0f);
	extents *= m_zoom;

	b2Vec2 lower = m_center - extents;
	b2Vec2 upper = m_center + extents;

	float32 u = (pw.x - lower.x) / (upper.x - lower.x);
	float32 v = (pw.y - lower.y) / (upper.y - lower.y);

	b2Vec2 ps;
	ps.x = u * w;
	ps.y = (1.0f - v) * h;
	return ps;
}

// Convert from world coordinates to normalized device coordinates.
// http://www.songho.ca/opengl/gl_projectionmatrix.html
void Camera::BuildProjectionMatrix(float32* m, float32 zBias)
{
	float32 w = float32(m_width);
	float32 h = float32(m_height);
	float32 ratio = w / h;
	b2Vec2 extents(ratio * 25.0f, 25.0f);
	extents *= m_zoom;

	b2Vec2 lower = m_center - extents;
	b2Vec2 upper = m_center + extents;

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
}

//
static void sCheckGLError()
{
	GLenum errCode = glGetError();
	if (errCode != GL_NO_ERROR)
	{
		fprintf(stderr, "OpenGL error = %d\n", errCode);
		assert(false);
	}
}

// Prints shader compilation errors
static void sPrintLog(GLuint object)
{
	GLint log_length = 0;
	if (glIsShader(object))
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
	else if (glIsProgram(object))
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
	else
	{
		fprintf(stderr, "printlog: Not a shader or a program\n");
		return;
	}

	char* log = (char*)malloc(log_length);

	if (glIsShader(object))
		glGetShaderInfoLog(object, log_length, NULL, log);
	else if (glIsProgram(object))
		glGetProgramInfoLog(object, log_length, NULL, log);

	fprintf(stderr, "%s", log);
	free(log);
}


//
static GLuint sCreateShaderFromString(const char* source, GLenum type)
{
	GLuint res = glCreateShader(type);
	const char* sources[] = { source };
	glShaderSource(res, 1, sources, NULL);
	glCompileShader(res);
	GLint compile_ok = GL_FALSE;
	glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
	if (compile_ok == GL_FALSE)
	{
		fprintf(stderr, "Error compiling shader of type %d!\n", type);
		sPrintLog(res);
		glDeleteShader(res);
		return 0;
	}

	return res;
}

//
static GLuint sCreateShaderProgram(const char* vs, const char* fs)
{
	GLuint vsId = sCreateShaderFromString(vs, GL_VERTEX_SHADER);
	GLuint fsId = sCreateShaderFromString(fs, GL_FRAGMENT_SHADER);
	assert(vsId != 0 && fsId != 0);

	GLuint programId = glCreateProgram();
	glAttachShader(programId, vsId);
	glAttachShader(programId, fsId);
    glBindFragDataLocation(programId, 0, "color");
	glLinkProgram(programId);

	glDeleteShader(vsId);
	glDeleteShader(fsId);

	GLint status = GL_FALSE;
	glGetProgramiv(programId, GL_LINK_STATUS, &status);
	assert(status != GL_FALSE);

	return programId;
}

//
struct GLRenderPoints
{
	void Create()
	{
		const char* vs = \
        "#version 400\n"
        "uniform mat4 projectionMatrix;\n"
        "layout(location = 0) in vec2 v_position;\n"
        "layout(location = 1) in vec4 v_color;\n"
		"layout(location = 2) in float v_size;\n"
        "out vec4 f_color;\n"
        "void main(void)\n"
        "{\n"
        "	f_color = v_color;\n"
        "	gl_Position = projectionMatrix * vec4(v_position, 0.0f, 1.0f);\n"
		"   gl_PointSize = v_size;\n"
        "}\n";

		const char* fs = \
        "#version 400\n"
        "in vec4 f_color;\n"
        "out vec4 color;\n"
        "void main(void)\n"
        "{\n"
        "	color = f_color;\n"
        "}\n";



		m_programId = sCreateShaderProgram(vs, fs);
		m_projectionUniform = glGetUniformLocation(m_programId, "projectionMatrix");
		m_vertexAttribute = 0;
		m_colorAttribute = 1;
		m_sizeAttribute = 2;

		// Generate
		glGenVertexArrays(1, &m_vaoId);
		glGenBuffers(3, m_vboIds);

		glBindVertexArray(m_vaoId);
		glEnableVertexAttribArray(m_vertexAttribute);
		glEnableVertexAttribArray(m_colorAttribute);
		glEnableVertexAttribArray(m_sizeAttribute);

		// Vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[0]);
		glVertexAttribPointer(m_vertexAttribute, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[1]);
		glVertexAttribPointer(m_colorAttribute, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glBufferData(GL_ARRAY_BUFFER, sizeof(m_colors), m_colors, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[2]);
		glVertexAttribPointer(m_sizeAttribute, 1, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glBufferData(GL_ARRAY_BUFFER, sizeof(m_sizes), m_sizes, GL_DYNAMIC_DRAW);

		sCheckGLError();

		// Cleanup
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		m_count = 0;
	}

	void Destroy()
	{
		if (m_vaoId)
		{
			glDeleteVertexArrays(1, &m_vaoId);
			glDeleteBuffers(2, m_vboIds);
			m_vaoId = 0;
		}

		if (m_programId)
		{
			glDeleteProgram(m_programId);
			m_programId = 0;
		}
	}

	void Vertex(const b2Vec2& v, const b2Color& c, float32 size)
	{
		if (m_count == e_maxVertices)
			Flush();

		m_vertices[m_count] = v;
		m_colors[m_count] = c;
		m_sizes[m_count] = size;
		++m_count;
	}

    void Flush()
	{
        if (m_count == 0)
            return;

		glUseProgram(m_programId);

		float32 proj[16] = { 0.0f };
		g_camera.BuildProjectionMatrix(proj, 0.1f);

		glUniformMatrix4fv(m_projectionUniform, 1, GL_FALSE, proj);

		glBindVertexArray(m_vaoId);

		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[0]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(b2Vec2), m_vertices);

		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[1]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(b2Color), m_colors);

		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[2]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(float32), m_sizes);

		glEnable(GL_PROGRAM_POINT_SIZE);
		glDrawArrays(GL_POINTS, 0, m_count);
        glDisable(GL_PROGRAM_POINT_SIZE);

		sCheckGLError();

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		m_count = 0;
	}

	enum { e_maxVertices = 512 };
	b2Vec2 m_vertices[e_maxVertices];
	b2Color m_colors[e_maxVertices];
    float32 m_sizes[e_maxVertices];

	int32 m_count;

	GLuint m_vaoId;
	GLuint m_vboIds[3];
	GLuint m_programId;
	GLint m_projectionUniform;
	GLint m_vertexAttribute;
	GLint m_colorAttribute;
	GLint m_sizeAttribute;
};


struct GLRenderSprites {
	void Create()
	{
		const char* vs = \
			"#version 150 core \n"
			"in vec2 position;\n"
			"in vec3 color;\n"
			"in vec2 texcoord;\n"
			"out vec3 Color;\n"
			"out vec2 Texcoord;\n"
			"uniform mat4 projectionMatrix;\n"
			"void main()\n"
			"{\n"
			"Color = color;\n"
			"Texcoord = texcoord;\n"
			"gl_Position = projectionMatrix * vec4(position, -1.0, 1.0);\n"
			"}\n";

		const char* fs = \
			"#version 150 core \n"
			"in vec3 Color; \n"
			"in vec2 Texcoord; \n"
			"out vec4 outColor; \n"
			"uniform sampler2D texKitten; \n"
			"void main() \n"
			"{ \n"
			"outColor = texture(texKitten, Texcoord); \n"
			"} \n";


		m_programId = sCreateShaderProgram(vs, fs);
		m_projectionUniform = glGetUniformLocation(m_programId, "projectionMatrix");
		m_vertexAttribute = 0;
		m_colorAttribute = 1;
		m_texAttribute = 2;

		// Generate
		glGenVertexArrays(1, &m_vaoId);
		glGenBuffers(2, m_vboIds);

		glBindVertexArray(m_vaoId);
		glEnableVertexAttribArray(m_vertexAttribute);
		glEnableVertexAttribArray(m_colorAttribute);
		glEnableVertexAttribArray(m_texAttribute);

		// Vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[0]);
		glVertexAttribPointer(m_vertexAttribute, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[1]);
		glVertexAttribPointer(m_colorAttribute, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glBufferData(GL_ARRAY_BUFFER, sizeof(m_colors), m_colors, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[2]);
		glVertexAttribPointer(m_texAttribute, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glBufferData(GL_ARRAY_BUFFER, sizeof(m_texCoords), m_texCoords, GL_DYNAMIC_DRAW);


		sCheckGLError();

		// Cleanup
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		m_count = 0;
	}


	void Vertex(const b2Vec2& v, const b2Color& c, const b2Vec2& t)
	{
		if (m_count == e_maxVertices)
			Flush();

		m_vertices[m_count] = v;
		m_colors[m_count] = c;
		m_texCoords[m_count] = t;
		++m_count;
	}

	void Flush() {

		if (m_count == 0)
			return;

		glUseProgram(m_programId);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, g_debugDraw.textures[1]);

		glUniform1i(glGetUniformLocation(m_programId, "texKitten"), 1);

		float32 proj[16] = { 0.0f };
		g_camera.BuildProjectionMatrix(proj, 0.1f);

		glUniformMatrix4fv(m_projectionUniform, 1, GL_FALSE, proj);

		glBindVertexArray(m_vaoId);

		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[0]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(b2Vec2), m_vertices);

		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[1]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(b2Color), m_colors);

		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[2]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(b2Vec2), m_texCoords);


		glDrawArrays(GL_TRIANGLES, 0, m_count);

		sCheckGLError();

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		m_count = 0;
	}


	void Destroy()
	{
		if (m_vaoId)
		{
			glDeleteVertexArrays(1, &m_vaoId);
			glDeleteBuffers(2, m_vboIds);
			m_vaoId = 0;
		}

		if (m_programId)
		{
			glDeleteProgram(m_programId);
			m_programId = 0;
		}
	}

	enum { e_maxVertices = 3 * 512 };
	b2Vec2 m_vertices[e_maxVertices];
	b2Color m_colors[e_maxVertices];
	b2Vec2 m_texCoords[e_maxVertices];

	int32 m_count;

	GLuint m_vaoId;
	GLuint m_vboIds[2];
	GLuint m_programId;
	GLint m_projectionUniform;
	GLint m_vertexAttribute;
	GLint m_texAttribute;
	GLint m_colorAttribute;

};

//
struct GLRenderLines
{
	void Create()
	{
		const char* vs = \
        "#version 400\n"
        "uniform mat4 projectionMatrix;\n"
        "layout(location = 0) in vec2 v_position;\n"
        "layout(location = 1) in vec4 v_color;\n"
        "out vec4 f_color;\n"
        "void main(void)\n"
        "{\n"
        "	f_color = v_color;\n"
        "	gl_Position = projectionMatrix * vec4(v_position, 0.0f, 1.0f);\n"
        "}\n";

		const char* fs = \
        "#version 400\n"
        "in vec4 f_color;\n"
        "out vec4 color;\n"
        "void main(void)\n"
        "{\n"
        "	color = f_color;\n"
        "}\n";

		m_programId = sCreateShaderProgram(vs, fs);
		m_projectionUniform = glGetUniformLocation(m_programId, "projectionMatrix");
		m_vertexAttribute = 0;
		m_colorAttribute = 1;

		// Generate
		glGenVertexArrays(1, &m_vaoId);
		glGenBuffers(2, m_vboIds);

		glBindVertexArray(m_vaoId);
		glEnableVertexAttribArray(m_vertexAttribute);
		glEnableVertexAttribArray(m_colorAttribute);

		// Vertex buffer
		
		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[0]);
		glVertexAttribPointer(m_vertexAttribute, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[1]);
		glVertexAttribPointer(m_colorAttribute, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glBufferData(GL_ARRAY_BUFFER, sizeof(m_colors), m_colors, GL_DYNAMIC_DRAW);
		


		sCheckGLError();

		// Cleanup
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		m_count = 0;
	}

	void Destroy()
	{
		if (m_vaoId)
		{
			glDeleteVertexArrays(1, &m_vaoId);
			glDeleteBuffers(2, m_vboIds);
			m_vaoId = 0;
		}

		if (m_programId)
		{
			glDeleteProgram(m_programId);
			m_programId = 0;
		}
	}

	void Vertex(const b2Vec2& v, const b2Color& c)
	{
		if (m_count == e_maxVertices)
			Flush();

		m_vertices[m_count] = v;
		m_colors[m_count] = c;
		++m_count;
	}

	void VertexArray(b2Vec2* vertices, int vertexCount, b2Color* color)
	{
		//m_vertices = vertices;
		//m_colors = color;
		//m_count = vertexCount;

        //std::copy(std::begin(vertices), std::begin(vertices)+vertexCount, std::begin(m_vertices));
        //std::copy(std::begin(color), std::begin(color)+vertexCount, std::begin(m_colors));

        memcpy(m_vertices,vertices,vertexCount*sizeof(b2Vec2));
        memcpy(m_colors,color,vertexCount*sizeof(b2Color));
        m_count+=vertexCount;

		/*
		if (m_count == e_maxVertices)
			Flush();

		m_vertices[m_count] = v;
		m_colors[m_count] = c;
		++m_count;
		*/
	}

    void Flush()
	{
        if (m_count == 0)
            return;

		glUseProgram(m_programId);

		float32 proj[16] = { 0.0f };
		g_camera.BuildProjectionMatrix(proj, 0.1f);

		glUniformMatrix4fv(m_projectionUniform, 1, GL_FALSE, proj);

		glBindVertexArray(m_vaoId);

		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[0]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(b2Vec2), m_vertices);

		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[1]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(b2Color), m_colors);

		glDrawArrays(GL_LINES, 0, m_count);

		sCheckGLError();

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		m_count = 0;
	}

	enum { e_maxVertices =  500000 };
    b2Vec2 m_vertices[e_maxVertices];
	b2Color m_colors[e_maxVertices];

	int32 m_count;
    int32 m_starting_index = 0;

	GLuint m_vaoId;
	GLuint m_vboIds[3];
	GLuint m_programId;
	GLint m_projectionUniform;
	GLint m_vertexAttribute;
	GLint m_colorAttribute;
};

//
struct GLRenderTriangles
{
	void Create()
	{
		const char* vs = \
			"#version 400\n"
			"uniform mat4 projectionMatrix;\n"
			"layout(location = 0) in vec3 v_position;\n"
			"layout(location = 1) in vec4 v_color;\n"
			"out vec4 f_color;\n"
			"void main(void)\n"
			"{\n"
			"	f_color = v_color;\n"
			"	gl_Position = projectionMatrix * vec4(v_position, 1.0f);\n"
			"}\n";

		const char* fs = \
			"#version 400\n"
			"in vec4 f_color;\n"
            "out vec4 color;\n"
			"void main(void)\n"
			"{\n"
			"	color = f_color;\n"
			"}\n";

		m_programId = sCreateShaderProgram(vs, fs);
		m_projectionUniform = glGetUniformLocation(m_programId, "projectionMatrix");
		m_vertexAttribute = 0;
		m_colorAttribute = 1;

		// Generate
		glGenVertexArrays(1, &m_vaoId);
		glGenBuffers(2, m_vboIds);

		glBindVertexArray(m_vaoId);
		glEnableVertexAttribArray(m_vertexAttribute);
		glEnableVertexAttribArray(m_colorAttribute);

		// Vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[0]);
		glVertexAttribPointer(m_vertexAttribute, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[1]);
		glVertexAttribPointer(m_colorAttribute, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glBufferData(GL_ARRAY_BUFFER, sizeof(m_colors), m_colors, GL_DYNAMIC_DRAW);




		sCheckGLError();

		// Cleanup
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		m_count = 0;
	}

	void Destroy()
	{
		if (m_vaoId)
		{
			glDeleteVertexArrays(1, &m_vaoId);
			glDeleteBuffers(2, m_vboIds);
			m_vaoId = 0;
		}

		if (m_programId)
		{
			glDeleteProgram(m_programId);
			m_programId = 0;
		}
	}

	void Vertex(const b2Vec2& v, const b2Color& c, float layer = 0.f)
	{
		if (m_count == e_maxVertices)
			Flush();

		m_vertices[m_count] = b2Vec3(v.x,v.y,layer);
		m_colors[m_count] = c;
		++m_count;
	}

    void Flush()
	{
        if (m_count == 0)
            return;

		glUseProgram(m_programId);

		float32 proj[16] = { 0.0f };
		g_camera.BuildProjectionMatrix(proj, 0.1f);

		glUniformMatrix4fv(m_projectionUniform, 1, GL_FALSE, proj);

		glBindVertexArray(m_vaoId);

		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[0]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(b2Vec3), m_vertices);

		glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[1]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(b2Color), m_colors);

		//glEnable(GL_BLEND);
        //glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDrawArrays(GL_TRIANGLES, 0, m_count);
        //glDisable(GL_BLEND);


		sCheckGLError();

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		m_count = 0;
	}

	enum { e_maxVertices = 3 * 512 };
	b2Vec3 m_vertices[e_maxVertices];
	b2Color m_colors[e_maxVertices];

	int32 m_count;

	GLuint m_vaoId;
	GLuint m_vboIds[2];
	GLuint m_programId;
	GLint m_projectionUniform;
	GLint m_vertexAttribute;
	GLint m_colorAttribute;
};

//
DebugDraw::DebugDraw()
{
	m_points = NULL;
    m_lines = NULL;
    m_triangles = NULL;
}

//
DebugDraw::~DebugDraw()
{
	//b2Assert(m_points == NULL);
	//b2Assert(m_lines == NULL);
	//b2Assert(m_triangles == NULL);
}

//
void DebugDraw::Create()
{
	m_points = new GLRenderPoints;
	m_points->Create();
	m_lines = new GLRenderLines;
	m_lines->Create();
	m_triangles = new GLRenderTriangles;
	m_triangles->Create();
	m_sprites = new GLRenderSprites;
	m_sprites->Create();

}

//
void DebugDraw::Destroy()
{
	m_points->Destroy();
	delete m_points;
	m_points = NULL;

	m_lines->Destroy();
	delete m_lines;
	m_lines = NULL;

	m_triangles->Destroy();
	delete m_triangles;
	m_triangles = NULL;
}


void DebugDraw::DrawLines(b2Vec2* data, int vertexCount, b2Color* color)
{
	m_lines->VertexArray(data, vertexCount, color);
	
}

void DebugDraw::DrawTexQuad(b2Vec2 pos, const b2Color& color, sprite curSprite, float angle)
{
	b2Vec2 lt = b2Mul(b2Rot(angle),b2Vec2(curSprite.left, curSprite.top));
	b2Vec2 lb = b2Mul(b2Rot(angle), b2Vec2(curSprite.left, curSprite.bottom));
	b2Vec2 rt = b2Mul(b2Rot(angle), b2Vec2(curSprite.right, curSprite.top));
	b2Vec2 rb = b2Mul(b2Rot(angle), b2Vec2(curSprite.right, curSprite.bottom));



	b2Vec2 p1 = pos + lt;
	m_sprites->Vertex(p1, color, b2Vec2(curSprite.texLeft, curSprite.texTop));

	p1 = pos + rt;
	m_sprites->Vertex(p1, color, b2Vec2(curSprite.texRight, curSprite.texTop));

	p1 = pos + lb;
	m_sprites->Vertex(p1, color, b2Vec2(curSprite.texLeft, curSprite.texBottom));

	p1 = pos + rb;
	m_sprites->Vertex(p1, color, b2Vec2(curSprite.texRight, curSprite.texBottom));

	p1 = pos + lb;
	m_sprites->Vertex(p1, color, b2Vec2(curSprite.texLeft, curSprite.texBottom));

	p1 = pos + rt;
	m_sprites->Vertex(p1, color, b2Vec2(curSprite.texRight, curSprite.texTop));
}


//
void DebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
    b2Vec2 p1 = vertices[vertexCount - 1];
	for (int32 i = 0; i < vertexCount; ++i)
	{
        b2Vec2 p2 = vertices[i];
		m_lines->Vertex(p1, color);
		m_lines->Vertex(p2, color);
        p1 = p2;
	}
}

//
 void DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color, float layer)
{
	//b2Color fillColor(0.5f * color.r, 0.5f * color.g, 0.5f * color.b, 0.5f);
	b2Color fillColor(color.r,color.g,color.b, color.a);

    for (int32 i = 1; i < vertexCount - 1; ++i)
    {
        m_triangles->Vertex(vertices[0], fillColor, layer);
        m_triangles->Vertex(vertices[i], fillColor, layer);
        m_triangles->Vertex(vertices[i+1], fillColor, layer);
    }
/*
    b2Vec2 p1 = vertices[vertexCount - 1];
	for (int32 i = 0; i < vertexCount; ++i)
	{
        b2Vec2 p2 = vertices[i];
		m_lines->Vertex(p1, color);
		m_lines->Vertex(p2, color);
        p1 = p2;
	}
 */
}

//
void DebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
{
	const float32 k_segments = 16.0f;
	const float32 k_increment = 2.0f * b2_pi / k_segments;
    float32 sinInc = sinf(k_increment);
    float32 cosInc = cosf(k_increment);
    b2Vec2 r1(1.0f, 0.0f);
    b2Vec2 v1 = center + radius * r1;
	for (int32 i = 0; i < k_segments; ++i)
	{
        // Perform rotation to avoid additional trigonometry.
        b2Vec2 r2;
        r2.x = cosInc * r1.x - sinInc * r1.y;
        r2.y = sinInc * r1.x + cosInc * r1.y;
		b2Vec2 v2 = center + radius * r2;
        m_lines->Vertex(v1, color);
        m_lines->Vertex(v2, color);
        r1 = r2;
        v1 = v2;
	}
}

//
void DebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
{
	const float32 k_segments = 16.0f;
	const float32 k_increment = 2.0f * b2_pi / k_segments;
    float32 sinInc = sinf(k_increment);
    float32 cosInc = cosf(k_increment);
    b2Vec2 v0 = center;
    b2Vec2 r1(cosInc, sinInc);
    b2Vec2 v1 = center + radius * r1;
	b2Color fillColor(0.5f * color.r, 0.5f * color.g, 0.5f * color.b, 0.5f);
	for (int32 i = 0; i < k_segments; ++i)
	{
        // Perform rotation to avoid additional trigonometry.
        b2Vec2 r2;
        r2.x = cosInc * r1.x - sinInc * r1.y;
        r2.y = sinInc * r1.x + cosInc * r1.y;
		b2Vec2 v2 = center + radius * r2;
		m_triangles->Vertex(v0, fillColor);
        m_triangles->Vertex(v1, fillColor);
        m_triangles->Vertex(v2, fillColor);
        r1 = r2;
        v1 = v2;
	}

    r1.Set(1.0f, 0.0f);
    v1 = center + radius * r1;
	for (int32 i = 0; i < k_segments; ++i)
	{
        b2Vec2 r2;
        r2.x = cosInc * r1.x - sinInc * r1.y;
        r2.y = sinInc * r1.x + cosInc * r1.y;
		b2Vec2 v2 = center + radius * r2;
        m_lines->Vertex(v1, color);
        m_lines->Vertex(v2, color);
        r1 = r2;
        v1 = v2;
	}

    // Draw a line fixed in the circle to animate rotation.
	b2Vec2 p = center + radius * axis;
	m_lines->Vertex(center, color);
	m_lines->Vertex(p, color);
}

//
void DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	m_lines->Vertex(p1, color);
	m_lines->Vertex(p2, color);
}

//
void DebugDraw::DrawTransform(const b2Transform& xf)
{
	const float32 k_axisScale = 0.4f;
    b2Color red(1.0f, 0.0f, 0.0f);
    b2Color green(0.0f, 1.0f, 0.0f);
	b2Vec2 p1 = xf.p, p2;

	m_lines->Vertex(p1, red);
	p2 = p1 + k_axisScale * xf.q.GetXAxis();
	m_lines->Vertex(p2, red);

	m_lines->Vertex(p1, green);
	p2 = p1 + k_axisScale * xf.q.GetYAxis();
	m_lines->Vertex(p2, green);
}




void DebugDraw::DrawString(int x, int y, const char *string, ...)
{



	va_list arg;
	va_start(arg, string);
	ImGui::Begin("Overlay", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
	ImGui::SetCursorPos(ImVec2(float(x), float(y)));
	ImGui::TextColoredV(ImColor(230, 10, 10, 255), string, arg);
	ImGui::End();
	va_end(arg);

}


void DebugDraw::DrawPoint(const b2Vec2& p, float32 size, const b2Color& color){
};

//
/*
void DebugDraw::DrawString(const b2Vec2& pw, const char *string, ...)
{
	ImVec2 ps = g_camera.ConvertWorldToScreen(pw);


	va_list arg;
	va_start(arg, string);
	ImGui::Begin("Overlay", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
	ImGui::SetCursorPos(ps);
	ImGui::TextColoredV(ImColor(230, 153, 153, 255), string, arg);
	ImGui::End();
	va_end(arg);
}
*/
/*

void DebugDraw::DrawString(int x, int y, const char *string, ...)
{
	float32 h = float32(g_camera.m_height);

	char buffer[128];

	va_list arg;
	va_start(arg, string);
	vsprintf(buffer, string, arg);
	va_end(arg);

	AddGfxCmdText(float(x), h - float(y), TEXT_ALIGN_LEFT, buffer, SetRGBA(230, 153, 153, 255));
}

void DebugDraw::DrawString(const b2Vec2& pw, const char *string, ...)
{
	b2Vec2 ps = g_camera.ConvertWorldToScreen(pw);
	float32 h = float32(g_camera.m_height);

	char buffer[128];

	va_list arg;
	va_start(arg, string);
	vsprintf(buffer, string, arg);
	va_end(arg);

	AddGfxCmdText(ps.x, h - ps.y, TEXT_ALIGN_LEFT, buffer, SetRGBA(230, 153, 153, 255));
}
*/
void DebugDraw::DrawAABB(b2AABB* aabb, const b2Color& c)
{
    b2Vec2 p1 = aabb->lowerBound;
    b2Vec2 p2 = b2Vec2(aabb->upperBound.x, aabb->lowerBound.y);
    b2Vec2 p3 = aabb->upperBound;
    b2Vec2 p4 = b2Vec2(aabb->lowerBound.x, aabb->upperBound.y);

    m_lines->Vertex(p1, c);
    m_lines->Vertex(p2, c);

    m_lines->Vertex(p2, c);
    m_lines->Vertex(p3, c);

    m_lines->Vertex(p3, c);
    m_lines->Vertex(p4, c);

    m_lines->Vertex(p4, c);
    m_lines->Vertex(p1, c);
}

//
void DebugDraw::Flush()
{
	m_lines->Flush();
	m_triangles->Flush();
    m_points->Flush();
	m_sprites->Flush();
}
