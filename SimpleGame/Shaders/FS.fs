#version 330

layout(location=0) out vec4 FragColor;

in vec2 v_Tex; //데이터 타입과 이름 동일하다.
uniform float u_Time;// 버텍스랑 프레그먼트 다 사용 ㄱㄴ
// R 채널은 X 값 , g 채널은 y 값.

const float c_PI = 3.141592;

void simple(){
	if(v_Tex.x >0.5){
	FragColor = vec4(v_Tex,0,1);
	}
	else{
	FragColor = vec4(0, v_Tex,1);
	}
}

void Line()
{
	float trans = c_PI * 0.5;
	float periodX = v_Tex.x * 10 * c_PI;
	float periodY = v_Tex.y * 10 * c_PI;

	float valueX = pow(abs(sin(periodX+periodY)),16);
	float valueY = pow(abs(sin(periodY- trans)),16);
	FragColor = vec4(valueX );
}

void circle()
{
	vec2 center = vec2(0.5, 0.5);
	vec2 currpos = v_Tex;
	float x =  distance(center,currpos);

	if (x < 0.5 && x>0.49){
		FragColor = vec4(1);
	}
	else{
		FragColor = vec4(0);
	}
	
}

void Circles()
{

	 float t = u_Time;  // 1로 나눈 나머지.
    vec2 center = vec2(0.5, 0.5);
	vec2 currpos = v_Tex;
	float count = 12;
	float d =  distance(center,currpos);
	float gray = pow(abs(sin(d*count*c_PI - u_Time)),16);
	FragColor = vec4(gray);
}
void main()
{
	Circles();
	
}
