#version 330

in vec3 a_Position;

void main()
{
	vec4 newPosition;
	newPosition.xy = a_Position.xy*u_Trans.w + u_Trans.xy;
	newPosition= vec4(a_Position,1);
	gl_Position = newPosition;
}
