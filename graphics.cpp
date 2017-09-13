#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <thread>

#include "graphics.hpp"
#include "camera.hpp"
#include "shaders.glsl"
#include <iostream>


float angleNorth= 0.f;


glm::mat4 setupCam()
{
  glm::mat4 proj = g_camera.BuildProjectionMatrix();

  glm::mat4 translate1 = glm::translate(glm::mat4(),glm::vec3(-g_camera.m_center.x,-g_camera.m_center.y,0.f));
  
  glm::mat4 rotN = glm::rotate(glm::mat4(), glm::radians(angleNorth), glm::vec3(0.0f, 0.0f, 1.0f));

  glm::mat4 translate2 =  glm::translate(glm::mat4(),glm::vec3(g_camera.m_center.x,g_camera.m_center.y,0.f));
  
  glm::mat4 trans = proj*translate2*rotN*translate1;

  //glm::mat4 trans = Model*rotN;
  return trans;
};



GLuint createShader(GLenum type, const GLchar* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    char buffer[512];
    glGetShaderInfoLog(shader, 512, NULL, buffer);
    
    if (status != GL_TRUE)
      {
	printf("%s\n", buffer);
      }
    
    return shader;
}


shaderData drawLineShaderInit( float* points, int numPoints) {

  shaderData outSh;
  
  outSh.vertexShader = createShader(GL_VERTEX_SHADER, lineVertexShaderSrc);
  outSh.fragmentShader = createShader(GL_FRAGMENT_SHADER, lineFragmentShaderSrc);
  outSh.geometryShader = createShader(GL_GEOMETRY_SHADER, lineGeometryShaderSrc);

  outSh.shaderProgram = glCreateProgram();

  

  glAttachShader(outSh.shaderProgram, outSh.vertexShader);
  glAttachShader(outSh.shaderProgram, outSh.fragmentShader);
  glAttachShader(outSh.shaderProgram, outSh.geometryShader);
  
  glLinkProgram(outSh.shaderProgram);
  glUseProgram(outSh.shaderProgram);

  
  glGenBuffers(1, &outSh.vbo);

  outSh.data = points;
  outSh.vertexCount = numPoints;
  

  glBindBuffer(GL_ARRAY_BUFFER, outSh.vbo);
  glBufferData(GL_ARRAY_BUFFER, outSh.vertexCount*2*sizeof(float), outSh.data, GL_DYNAMIC_DRAW);

  // Create VAO
  glGenVertexArrays(1, &outSh.vao);
  glBindVertexArray(outSh.vao);

  // Specify layout of point data
  GLint posAttrib = glGetAttribLocation(outSh.shaderProgram, "pos");
  glEnableVertexAttribArray(posAttrib);
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

  return outSh;
  
}

void drawLine(shaderData sh, Camera cam) {


  glBindVertexArray(sh.vao);
  
  glUseProgram(sh.shaderProgram);
  /*
  glm::mat4 Model = cam.BuildProjectionMatrix();

   glm::mat4 rotN;

  rotN = glm::rotate(rotN, glm::radians(angleNorth), glm::vec3(0.0f, 0.0f, 1.0f));
  */
  glm::mat4 trans = setupCam();
  
  GLuint uniTrans = glGetUniformLocation(sh.shaderProgram, "Model");
  
  glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(trans));

  GLint uniColor = glGetUniformLocation(sh.shaderProgram, "lineColor");
  
  glUniform4f(uniColor, 0.0f, 0.0f, 1.0f, 1.0f);
   
  glDrawArrays(GL_LINES, 0, sh.vertexCount);

};

shaderData drawBuildingOutlinesInit(float* verts, const int nverts)
{
  shaderData outSh;

  outSh.vertexShader = createShader(GL_VERTEX_SHADER, vertexSource);
  outSh.fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentSource);
 
  outSh.shaderProgram = glCreateProgram(); 

  glAttachShader(outSh.shaderProgram, outSh.vertexShader);
  glAttachShader(outSh.shaderProgram, outSh.fragmentShader);
 
  //glBindFragDataLocation(outSh.shaderProgram, 0, "outColor");
  
  glLinkProgram(outSh.shaderProgram);
  glUseProgram(outSh.shaderProgram);

  glGenBuffers(1, &outSh.vbo); // Generate 1 buffer

  outSh.data = verts;
  outSh.vertexCount = nverts;

  
  glBindBuffer(GL_ARRAY_BUFFER, outSh.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*nverts*2, verts, GL_STATIC_DRAW);

  
  glGenVertexArrays(1, &outSh.vao);
  
  glBindVertexArray(outSh.vao);
  
  

  GLint posAttrib = glGetAttribLocation(outSh.shaderProgram, "position");
  glEnableVertexAttribArray(posAttrib);			
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
			
  return outSh;
  
};

void drawBuildingOutlines( shaderData sh, Camera cam)
{
  glBindVertexArray(sh.vao);
  
  glUseProgram(sh.shaderProgram);
  /*
  glm::mat4 Model = cam.BuildProjectionMatrix();

  glm::mat4 rotN;

  rotN = glm::rotate(rotN, glm::radians(angleNorth), glm::vec3(0.0f, 0.0f, 1.0f));

  glm::mat4 trans = Model*rotN;
  */
  glm::mat4 trans = setupCam();
  GLuint uniTrans = glGetUniformLocation(sh.shaderProgram, "Model");
  
  glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(trans));


  GLint uniColor = glGetUniformLocation(sh.shaderProgram, "setColor");
  
  glUniform4f(uniColor, 1.0f, 1.0f, 1.0f, 1.0f);
   
  glDrawArrays(GL_LINES, 0, sh.vertexCount);

};

shaderData drawMapShaderInit(const float* verts, const int nverts, const int* elements, const  int nelements )
{
  shaderData outSh;
  
  outSh.vertexShader = createShader(GL_VERTEX_SHADER, vertexSource);
  outSh.fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentSource);
 
  outSh.shaderProgram = glCreateProgram(); 

  glAttachShader(outSh.shaderProgram, outSh.vertexShader);
  glAttachShader(outSh.shaderProgram, outSh.fragmentShader);
 
  glBindFragDataLocation(outSh.shaderProgram, 0, "outColor");
  
  glLinkProgram(outSh.shaderProgram);
  glUseProgram(outSh.shaderProgram);

  glGenVertexArrays(1, &outSh.vao);
  
  glBindVertexArray(outSh.vao);
  
  
  glGenBuffers(1, &outSh.vbo); // Generate 1 buffer
  
  glBindBuffer(GL_ARRAY_BUFFER, outSh.vbo);
  
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*nverts*2, verts, GL_STATIC_DRAW);

  glGenBuffers(1, &outSh.ebo);
  
  outSh.elementCount = nelements;
  // printf("nelements:%d \n", nelements);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, outSh.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
	       sizeof(int)*nelements*3, elements, GL_STATIC_DRAW);

  GLint posAttrib = glGetAttribLocation(outSh.shaderProgram, "position");
			
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
			
  glEnableVertexAttribArray(posAttrib);

  /*
  cam.m_center = glm::vec2(0.5f,0.5f);

  //  float proj[16] = { 0.0f };
  glm::mat4 Model = cam.BuildProjectionMatrix();

  uniTrans = glGetUniformLocation(shaderProgram, "Model");
  glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(Model));
  */
  return outSh;
};

void drawMap( shaderData sh, Camera cam){

  glBindVertexArray(sh.vao);
  
  glUseProgram(sh.shaderProgram);


  /*
  glm::mat4 Model = cam.BuildProjectionMatrix();

  glm::mat4 rotN;

  rotN = glm::rotate(rotN, glm::radians(angleNorth), glm::vec3(0.0f, 0.0f, 1.0f));

  glm::mat4 trans = Model*rotN;
  */

  glm::mat4 trans = setupCam();
  
  GLuint uniTrans = glGetUniformLocation(sh.shaderProgram, "Model");
  
  glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(trans));

  GLint uniColor = glGetUniformLocation(sh.shaderProgram, "setColor");
  
  glUniform4f(uniColor, 0.2f, 0.4f, 0.2f, 1.0f);

  
  glDrawElements(GL_TRIANGLES, sh.elementCount*3, GL_UNSIGNED_INT, 0);
   
}



/*


  const char* circleSource = R"glsl(
#version 330
    
in vec2 fPosition;
out vec4 fColor;

void main() {
    vec4 colors[4] = vec4[](
        vec4(1.0, 0.0, 0.0, 1.0), 
        vec4(0.0, 1.0, 0.0, 1.0), 
        vec4(0.0, 0.0, 1.0, 1.0), 
        vec4(0.0, 0.0, 0.0, 1.0)
    );
    fColor = vec4(1.0);

    for(int row = 0; row < 2; row++) {
        for(int col = 0; col < 2; col++) {
            float dist = distance(fPosition, vec2(-0.50 + col, 0.50 - row));
            float alpha = step(0.45, dist);
            fColor = mix(colors[row*2+col], fColor, alpha);
        }
    }
}
)glsl";


  GLuint circleShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(circleShader, 1, &circleSource, NULL);
  glCompileShader(circleShader);

  GLint status0;
  glGetShaderiv(circleShader, GL_COMPILE_STATUS, &status0);

  char buffer0[512];

  glGetShaderInfoLog(circleShader, 512, NULL, buffer0);

  if (status0 != GL_TRUE)
  {
	  printf("%s\n", buffer0);
  }



	const char* circleSource = R"glsl(
#version 330
    
in vec2 fPosition;
out vec4 fColor;

void main() {
    vec4 colors[4] = vec4[](
        vec4(1.0, 0.0, 0.0, 1.0), 
        vec4(0.0, 1.0, 0.0, 1.0), 
        vec4(0.0, 0.0, 1.0, 1.0), 
        vec4(0.0, 0.0, 0.0, 1.0)
    );
    fColor = vec4(1.0);

    for(int row = 0; row < 2; row++) {
        for(int col = 0; col < 2; col++) {
            float dist = distance(fPosition, vec2(-0.50 + col, 0.50 - row));
            float delta = fwidth(dist);
            float alpha = smoothstep(0.15-delta, 0.15, dist);
            fColor = mix(colors[row*2+col], fColor, alpha);
        }
    }
}
)glsl";


	GLuint circleVertexShader = createShader(GL_VERTEX_SHADER,vertexSource);
	GLuint circleShader = createShader(GL_FRAGMENT_SHADER,circleSource);

	GLuint circleProgram = glCreateProgram();
	glAttachShader(circleProgram, circleVertexShader);
	glAttachShader(circleProgram, circleShader);

	//glBindFragDataLocation(shaderProgram, 0, "outColor");

	glLinkProgram(circleProgram);



	GLint circlePosAttrib = glGetAttribLocation(circleProgram, "position");

	glVertexAttribPointer(circlePosAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(circlePosAttrib);

	glUseProgram(circleProgram);  circleProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(circleProgram, circleShader);
  
  //glBindFragDataLocation(shaderProgram, 0, "outColor");

  glLinkProgram(circleProgram);





*/
