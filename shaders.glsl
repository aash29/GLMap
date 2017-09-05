
const char* lineVertexShaderSrc = R"glsl(
    #version 150 core
    in vec2 pos;
    uniform mat4 Model;

    void main()
    {
	gl_Position = vec4(pos, 0.0, 1.0);
    }
)glsl";

// Fragment shader
const char* lineFragmentShaderSrc = R"glsl(
    #version 150 core
    out vec4 outColor;

    void main()
    {
        outColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
)glsl";

  
  const char* lineGeometryShaderSrc = R"glsl(
#version 150 core

uniform mat4 Model;


layout(lines) in;
layout(triangle_strip, max_vertices = 100) out;

void main()
{
    vec4 d1 = normalize(gl_in[1].gl_Position - gl_in[0].gl_Position);
    vec4 n1 = vec4(-d1.y,d1.x, 0.0, 0.0);
    
    float linewidth = 0.0005f;

    gl_Position = Model*(gl_in[0].gl_Position + linewidth * n1);
    EmitVertex();

    gl_Position = Model*(gl_in[1].gl_Position + linewidth * n1);
    EmitVertex();

    gl_Position = Model*(gl_in[0].gl_Position - linewidth * n1);
    EmitVertex();

    EndPrimitive();


    gl_Position = Model*(gl_in[1].gl_Position + linewidth * n1);
    EmitVertex();

    gl_Position = Model*(gl_in[1].gl_Position - linewidth * n1);
    EmitVertex();

    gl_Position = Model*(gl_in[0].gl_Position - linewidth * n1);
    EmitVertex();

    EndPrimitive();


})glsl";

  const char* vertexSource = R"glsl(
	#version 150 core
        in vec2 position;

        uniform mat4 Model;

        void main()
        {
            gl_Position = Model * vec4(position, 0.0, 1.0);
        }
)glsl";
  
const char* fragmentSource = R"glsl(
	#version 150 core

	out vec4 outColor;

	void main()
	{
		outColor = vec4(1.0, 1.0, 1.0, 1.0);
	}
	)glsl";
