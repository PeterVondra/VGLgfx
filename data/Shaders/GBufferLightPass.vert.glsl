layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUvs;

layout(location = 0) out vec2 UV;

void main()
{
	gl_Position = vec4(inPosition, 0.0f, 1.0f);
	UV = inUvs;
}
