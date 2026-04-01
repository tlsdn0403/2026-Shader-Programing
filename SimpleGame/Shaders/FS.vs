#version 330

in vec3 a_Pos;
in vec2 a_Tex;
uniform float u_Time;
out vec2 v_Tex;
void main()
{
    vec4 newPosition;
    newPosition = vec4(a_Pos, 1.0);

    v_Tex = a_Tex;
    gl_Position = newPosition;
}