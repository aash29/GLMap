
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



const char* pathGraphVertexShaderSrc = R"glsl(
    #version 400

    //layout (location = 0) in vec3 VertexPosition;
    //layout (location = 1) in vec3 VertexTexCoord;

    in vec2 pos;
    uniform mat4 Model;
    //uniform mat4 gl_ModelViewMatrix;
    //uniform mat4 gl_ViewMatrixInverse;
    //in vec2 TexCoord;
    //out vec2 texcoord;


    void main()
    {
       gl_Position = vec4(pos, -1.0, 1.0);
       //texcoord = TexCoord;
    }
)glsl";

// Fragment shader
const char* pathGraphFragmentShaderSrc = R"glsl(
    #version 400
    in vec2 TexCoord;
    //layout (location = 0) out vec4 FragColor;

    out vec4 outColor;
    uniform vec4 lineColor;
    vec2 st = gl_FragCoord.xy;
    in vec4 graphVertex;
    uniform mat4 ModelInv;
    uniform mat4 Model;


    //vec4 d1 = ModelInv*vec4(st,0.f,0.f);

    //vec4 d2 = graphVertex;
/*
    float circle(in vec2 _st, in float _radius){

      vec2 dist = _st-vec2(d2.x, d2.y);

	  return 1.-smoothstep(_radius-0.01f,
                         _radius+0.01f,
                         sqrt(dot(dist,dist)));
}
*/

void main() {
 float dx = TexCoord.x - 0.5;
 float dy = TexCoord.y - 0.5;
 float dist = sqrt(dx * dx + dy * dy);
 outColor =
 mix( vec4(1.f,0.f,0.f,1.f), vec4(0.f,0.f,0.f,0.f),
 smoothstep( 0.25, 0.45, dist )
 );

 //outColor = vec4( 1.f, 0.f, 0.f, 1.0 );

}
 /*
    void main()
    {
        //outColor = vec4(1.0, 0.0, 0.0, 1.0);
    outColor = lineColor;

    //vec3 color = vec3(circle(vec2(d1.x,d1.y),0.05),0.f,0.f);
    //vec3 color = vec3(1.f,0.f,0.f);
	//outColor = vec4( color, 1.0 );
    }
    */
)glsl";


  const char* pathGraphGeometryShaderSrc = R"glsl(
#version 150 core

uniform mat4 Model;
out vec2 TexCoord;


layout(lines) in;
layout(triangle_strip, max_vertices = 100) out;

void main()
{

    float side = 0.05f;

    gl_Position = Model*(gl_in[0].gl_Position +vec4(-side,-side,0.f,0.f));
    TexCoord = vec2(0.f,0.f);
    EmitVertex();


    gl_Position = Model*(gl_in[0].gl_Position +vec4(-side,side,0.f,0.f));
    TexCoord = vec2(0.f,1.f);
    EmitVertex();

    gl_Position = Model*(gl_in[0].gl_Position +vec4(side,-side,0.f,0.f));
    TexCoord = vec2(1.f,0.f);
    EmitVertex();

    gl_Position = Model*(gl_in[0].gl_Position +vec4(side,side,0.f,0.f));
    TexCoord = vec2(1.f,1.f);
    EmitVertex();


   EndPrimitive();


    gl_Position = Model*(gl_in[1].gl_Position +vec4(-side,-side,0.f,0.f));
    TexCoord = vec2(0.f,0.f);
    EmitVertex();

    gl_Position = Model*(gl_in[1].gl_Position +vec4(-side,side,0.f,0.f));
    TexCoord = vec2(0.f,1.f);
    EmitVertex();

    gl_Position = Model*(gl_in[1].gl_Position +vec4(side,-side,0.f,0.f));
    TexCoord = vec2(1.f,0.f);
    EmitVertex();

    gl_Position = Model*(gl_in[1].gl_Position +vec4(side,side,0.f,0.f));
    TexCoord = vec2(1.f,1.f);
    EmitVertex();


    EndPrimitive();



})glsl";


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
    
    float linewidth = 0.025f;

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
