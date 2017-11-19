
const GLchar* texQuadVertex = R"glsl(
    #version 150 core
    in vec2 position;
    in vec3 color;
    in vec2 texcoord;
    out vec3 Color;
    out vec2 Texcoord;
    uniform mat4 Model;


    void main()
    {
        Color = color;
        Texcoord = texcoord;
        //gl_Position = vec4(position, 0.0, 1.0);
	gl_Position = Model * vec4(position, -1.0, 1.0);
    }
)glsl";
const GLchar* texQuadFragment = R"glsl(
    #version 150 core
    in vec3 Color;
    in vec2 Texcoord;
    out vec4 outColor;
    uniform sampler2D texKitten;
    void main()
    {
        outColor = texture(texKitten, Texcoord);
    }
)glsl";


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
    uniform vec4 lineColor;
    void main()
    {
        //outColor = vec4(1.0, 0.0, 0.0, 1.0);
	outColor = lineColor;
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
           gl_Position = Model * vec4(position, -1.0, 1.0);
        }
)glsl";
  
const char* fragmentSource = R"glsl(
	#version 150 core

	out vec4 outColor;
    uniform vec4 setColor;

	void main()
	{
		outColor = setColor;
	}
	)glsl";
