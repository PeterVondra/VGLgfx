#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;
 
layout(location = 0) in vec3 WorldPosGS[];
layout(location = 1) in vec3 WorldNormalGS[];
 
layout(location = 3) out vec3 WorldPos;
layout(location = 4) out vec3 WorldNormal;
 
layout(location = 0) out vec4 World_Position;
layout(location = 1) out vec4 World_Normal;

void main()
{
    // Plane normal
    const vec3 N = abs(cross(WorldPosGS[1] - WorldPosGS[0], WorldPosGS[2] - WorldNormalGS[0]));
    for (int i = 0; i < 3; ++i)
    {
        WorldPos = WorldPosGS[i];
        WorldNormal = WorldNormalGS[i];
        if (N.z > N.x && N.z > N.y)
        {
            gl_Position = vec4(WorldPos.x, WorldPos.y, 0.0f, 1.0f);
        }
        else if (N.x > N.y && N.x > N.z)
        {
            gl_Position = vec4(WorldPos.y, WorldPos.z, 0.0f, 1.0f);
        }
        else
        {
            gl_Position = vec4(WorldPos.x, WorldPos.z, 0.0f, 1.0f);
        }
        EmitVertex();
    }
    EndPrimitive();
}