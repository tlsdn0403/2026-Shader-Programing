#version 330

in vec3 a_Position;

void main()
{
    // u_Trans를 사용하는 잘못된 코드를 지우고
    // 정점(Vertex) 좌표를 그대로 화면 위치로 넘겨줍니다.
    gl_Position = vec4(a_Position, 1.0);
}