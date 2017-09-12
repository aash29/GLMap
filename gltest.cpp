#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <thread>

#include "graphics.hpp"
  
  float points[] = {
    -0.45f,  0.45f,
    0.45f,  0.45f,
    0.45f,  0.45f,
    0.45f, -0.45f,
    0.45f, -0.45f,
    -0.45f, -0.45f,
  };
  



shaderData drawLine() {

const char* vertexShaderSrc = R"glsl(
    #version 150 core
    in vec2 pos;

    void main()
    {
        gl_Position = vec4(pos, 0.0, 1.0);
    }
)glsl";

// Fragment shader
const char* fragmentShaderSrc = R"glsl(
    #version 150 core
    out vec4 outColor;

    void main()
    {
        outColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
)glsl";

  
  const char* geometryShaderSrc = R"glsl(
#version 150 core

layout(lines) in;
layout(triangle_strip, max_vertices = 100) out;

void main()
{
    vec4 d1 = normalize(gl_in[1].gl_Position - gl_in[0].gl_Position);
    vec4 n1 = vec4(-d1.y,d1.x, 0.0, 0.0);
    
    gl_Position = gl_in[0].gl_Position + 0.1 * n1;
    EmitVertex();

    gl_Position = gl_in[1].gl_Position + 0.1 * n1;
    EmitVertex();

    gl_Position = gl_in[0].gl_Position - 0.1 * n1;
    EmitVertex();

    EndPrimitive();


    gl_Position = gl_in[1].gl_Position + 0.1 * n1;
    EmitVertex();

    gl_Position = gl_in[1].gl_Position - 0.1 * n1;
    EmitVertex();

    gl_Position = gl_in[0].gl_Position - 0.1 * n1;
    EmitVertex();

    EndPrimitive();


})glsl";

  shaderData outSh;
  
  outSh.vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSrc);
  outSh.fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
  outSh.geometryShader = createShader(GL_GEOMETRY_SHADER, geometryShaderSrc);

  outSh.shaderProgram = glCreateProgram();

  

  glAttachShader(outSh.shaderProgram, outSh.vertexShader);
  glAttachShader(outSh.shaderProgram, outSh.fragmentShader);
  glAttachShader(outSh.shaderProgram, outSh.geometryShader);
  
  glLinkProgram(outSh.shaderProgram);
  glUseProgram(outSh.shaderProgram);

  
  glGenBuffers(1, &outSh.vbo);

  /*
  float points[] = {
    -0.45f,  0.45f,
    0.45f,  0.45f,
    0.45f,  0.45f,
    0.45f, -0.45f,
    0.45f, -0.45f,
    -0.45f, -0.45f,
  };
  */
  outSh.data = points;
  outSh.vertexCount = 6;
  

  glBindBuffer(GL_ARRAY_BUFFER, outSh.vbo);
  glBufferData(GL_ARRAY_BUFFER, outSh.vertexCount*2*sizeof(float), outSh.data, GL_DYNAMIC_DRAW);

  // Create VAO
  //GLuint vao;
  glGenVertexArrays(1, &outSh.vao);
  glBindVertexArray(outSh.vao);

  // Specify layout of point data
  GLint posAttrib = glGetAttribLocation(outSh.shaderProgram, "pos");
  glEnableVertexAttribArray(posAttrib);
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

  return outSh;
  
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", nullptr, nullptr); // Windowed

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	glewInit();

	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);

	printf("%u\n", vertexBuffer);


	GLuint vao;
	glGenVertexArrays(1, &vao);

	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo); // Generate 1 buffer

	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	const char* vertexSource = R"glsl(
	#version 150 core

    in vec2 position;
    out vec2 fPosition;

	void main()
	{
		fPosition = position;
		gl_Position = vec4(position, 0.0, 1.0);
	}
	)glsl";

	GLuint vertexShader = createShader(GL_VERTEX_SHADER, vertexSource);

	const char* fragmentSource = R"glsl(
	#version 150 core

		out vec4 outColor;

	void main()
	{
		outColor = vec4(1.0, 1.0, 1.0, 1.0);
	}
	)glsl";

	GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER,fragmentSource);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	glBindFragDataLocation(shaderProgram, 0, "outColor");

	glLinkProgram(shaderProgram);

	glUseProgram(shaderProgram);

	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");

	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(posAttrib);


	GLuint circleVao;
	glGenVertexArrays(1, &circleVao);

	glBindVertexArray(circleVao);

	GLuint circleVbo;
	glGenBuffers(1, &circleVbo); // Generate 1 buffer

	glBindBuffer(GL_ARRAY_BUFFER, circleVbo);

	float circleVertices[6][2] = {
		{ -1.f, -1.f },
		{ 1.f, -1.f },
		{ -1.f, 1.f },
		{ 1.f, -1.f },
		{ 1.f, 1.f },
		{ -1.f, 1.f }
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 2, circleVertices, GL_STATIC_DRAW);





	shaderData sh1 = drawLineShaderInit(points);


	while (!glfwWindowShouldClose(window))
	{
	  /*

		glUseProgram(circleProgram);

		glBindVertexArray(circleVao);

		glDrawArrays(GL_TRIANGLES, 0, 6);


		glUseProgram(shaderProgram);

		glBindVertexArray(vao);

		glDrawArrays(GL_TRIANGLES, 0, 3);
	  */

	  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	  glClear(GL_COLOR_BUFFER_BIT);
	  
	  /*
	  points[0]=1.f;
	  points[1]=1.f;

	  glBindBuffer(GL_ARRAY_BUFFER, sh1.vbo);

	  glBufferSubData(GL_ARRAY_BUFFER, 0, sh1.vertexCount*2*sizeof(float), sh1.data);

	  */

	  //glDrawArrays(GL_LINES, 0, sh1.vertexCount);
	  drawLine(sh1);
	  
	  glfwSwapBuffers(window);
	  glfwPollEvents();
	}

	glfwTerminate();
}
