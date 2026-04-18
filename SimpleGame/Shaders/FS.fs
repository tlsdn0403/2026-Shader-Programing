#version 330

layout(location=0) out vec4 FragColor;

in vec2 v_Tex; //데이터 타입과 이름 동일하다.
uniform float u_Time;// 버텍스랑 프레그먼트 다 사용 ㄱㄴ
uniform vec4 u_Points[500];

// R 채널은 X 값 , g 채널은 y 값.

const float c_PI = 3.141592;
const vec4 c_Points[2] = vec4[2](vec4(0.5 ,0.5, 0 ,0.5) , vec4(0.5 , 0.7, 0.5 ,1));


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

    vec2 center = vec2(0.5, 0.5);
	vec2 currpos = v_Tex;
	float count = 12;

	float d =  distance(center,currpos);
	float gray = pow(abs(sin(d*count*c_PI - u_Time)),16);
	FragColor = vec4(gray);
}

void RainDrop()
{
	float accum = 0;
	for(int i = 0; i < 500; i++){
		float sTime = u_Points[i].z;
		float lTime = u_Points[i].w;
		float newTime = u_Time - sTime; 
		if(newTime> 0){
			float t = fract(newTime);
			float oneMinus = 1- t; // 1~0
			t = t*lTime; // 0~lTime
			vec2 center = u_Points[i].xy;
			vec2 currpos = v_Tex;
			float count = 5;
	
			float range = t/5;

			float d =  distance(center,currpos);
			float fade = (1/range)*clamp(range - d, 0 , 1); // 센터는 0.5 , 가장자리로 갈수록 0에 가까워짐. 마이너스 값은 없어야해서 clamp로 0~1 사이로 고정.
			float gray = pow(abs(sin(d * 4 * count * c_PI - u_Time)),16);
			accum += (gray*fade * oneMinus);
		}
		
	}


	FragColor = vec4(accum);
}
void Flame(){
	float amp = 0.5;
	float speed = 4;
	float newY = 1-v_Tex.y;
	float sinInput = newY*c_PI*2 -u_Time*speed; //주기
	float sinValue = newY* amp*(((sin(sinInput)+1)/2) -0.5) + 0.5;
	float fwidth = 0.1;
	float width = 0.3 * mix(fwidth, 1, newY);
	float grey = 0;
	
	
	if((v_Tex.x <sinValue + width/2) && (v_Tex.x >sinValue -width/2)){
		grey = 1;
	}
	else{
		grey = 0;
		discard; // 픽셀 버리기
	}
	FragColor = vec4(grey ); //한주기
}
void Flag(){
	float amp = 0.5;
	float speed = 4;
	float sinInput = v_Tex.x*c_PI*2 -u_Time*speed; //주기
	float sinValue =v_Tex.x* amp*(((sin(sinInput)+1)/2) -0.5) + 0.5;
	float fwidth = 0.1;
	float width = 0.3 * mix(1, fwidth, v_Tex.x);
	float grey = 0;
	
	
	if((v_Tex.y <sinValue + width/2) && (v_Tex.y >sinValue -width/2)){
		grey = 1;
	}
	else{
		grey = 0;
		discard; // 픽셀 버리기
	}
	FragColor = vec4(grey ); //한주기
}

void main()
{
	Flame();
}
