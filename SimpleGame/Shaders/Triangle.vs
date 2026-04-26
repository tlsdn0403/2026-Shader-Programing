#version 330

uniform float u_Time;

in vec3 a_Position;
in float a_Mass; //중력의 영향을 받는 모양새는 중량을 가지고 있음
in vec2 a_vel;   
in float a_RV;
in float a_RV1;
in float a_RV2;

out float v_Grey;

const float c_Pi = 3.141592;
const vec2 c_G = vec2(0, -9.8);

void Basic()
{
    //vec4 newPosition;
    //newPosition.x = a_Position.x + 0.5;
    //newPosition.y = a_Position.y;
    //newPosition.z = a_Position.z;
    //gl_Position = newPosition;

    float t = mod(u_Time,1.0);  // 1로 나눈 나머지.

    vec3 pos = a_Position;
    pos.x+= t;
    gl_Position = vec4(pos, 1.0);
}
void sin1()
{
 float t = mod(u_Time,1.0);  // 1로 나눈 나머지.

    vec3 pos = a_Position;
    pos.x+= t;
    pos.y += 0.5 * sin(3.141592 * 2 * t);
    gl_Position = vec4(pos, 1.0);
}
void sin2()
{
    float t = mod(u_Time, 1.0);   // 0 ~ 1 반복

    vec3 pos = a_Position;

    pos.x += -1.0 + 2.0 * t;

    // y: sin 1주기
    pos.y += 0.5 * sin(6.2831853*2 * t);

    gl_Position = vec4(pos, 1.0);
}
void circle()
{
    float t = mod(u_Time, 1.0);   // 0 ~ 1 반복

    vec3 pos = a_Position;

    pos.x +=  cos(3.141592*2 * t);

    // y: sin 1주기
    pos.y += sin(3.141592*2 * t);

    gl_Position = vec4(pos, 1.0);
}
void circle2()
{
    float t = mod(u_Time, 1.0);   // 0 ~ 1 반복

    // 공전: 1주기 동안 원 한 바퀴
    float orbitAngle = 2.0 * 3.141592 * t;

    // 자전: 공전보다 훨씬 빠르게
    float spinAngle = 2.0 * 3.141592 * t * 10.0;   // 1주기 동안 10바퀴 자전

    vec3 pos = a_Position;

    // 1. 사각형 자체를 빠르게 회전(자전)
    float c = cos(spinAngle);
    float s = sin(spinAngle);

    float rx = pos.x * c - pos.y * s;
    float ry = pos.x * s + pos.y * c;

    pos.x = rx;
    pos.y = ry;

    // 2. 원 궤도를 따라 이동(공전)
    float radius = 1.0;
    pos.x += radius * cos(orbitAngle);
    pos.y += radius * sin(orbitAngle);

    gl_Position = vec4(pos, 1.0);
}
float pseudoRandom(float n)
{
    return fract(sin(n) * 43758.5453123);
}
void Falling()
{
    // emitTime을 계산하기 위함
    float newTime = u_Time - a_RV1*3;
    if(newTime>0)
    {
        float lifeTime = (a_RV2 +0.5)*0.2 ; // 얘가 파티클 별로 다르면 어떨까?  ->0.5~1.5 초
        
        float t= mod(newTime, lifeTime); // mod = 시간을 1로 나눈 나머지 -> life time이 1초였다.

        float size = pseudoRandom(a_RV1)* (lifeTime -t)/lifeTime;  // 어떻게 해야 작아질까?? (시간이 지날수록 시험)

        float initPosX =size* a_Position.x + sin(2*c_Pi*a_RV);
        float initPosY =size* a_Position.y + cos(2*c_Pi*a_RV);
        vec4 newPos;
       
        newPos.x = initPosX+ (a_vel.x/10* t) + (0.5 * t * t *c_G.x );
        newPos.y = initPosY+ (a_vel.y/10* t) + (0.5 * t * t *c_G.y );
        newPos.z = 0;
        newPos.w =1;

        gl_Position = newPos;
    }
    else
    {
        gl_Position = vec4(100.0, 100.0, 100.0, 1.0);
    }
   
}

void snow()
{
    float t=mod(u_Time+ a_RV, 1.0);
    float initPosX =cos(a_Position.x*c_Pi*t)-1;
    float initPosY =sin(a_Position.y*c_Pi*t);
    vec4 newPos;
    newPos.x = initPosX+ (a_vel.x* t) + (t*t*1/2 *c_G.x );
    newPos.y = initPosY+ (a_vel.y* t) + (t*t*1/2 *c_G.y );
    newPos.z =0;
    newPos.w =1;

    gl_Position = newPos;
}
void practiceTest_1()
{
    float t=mod(u_Time, 1.0);
    float initPosX =cos(a_Position.x);
    float initPosY =sin(a_Position.y);
    vec4 newPos;
    newPos.x = initPosX+ (a_vel.x* t) + (t*t*1/2 *c_G.x );
    newPos.y = initPosY+ (a_vel.y* t) + (t*t*1/2 *c_G.y );
    newPos.z =0;
    newPos.w =1;

    gl_Position = newPos;
}
void Answer_1()
{
    // emitTime을 계산하기 위함
    float newTime = u_Time - pseudoRandom(a_RV1);
    if(newTime>0){
        float size = pseudoRandom(a_RV1);
        float t=mod(newTime, 1.0);
        float initPosX =size* a_Position.x + sin(2*c_Pi*a_RV);
        float initPosY =size* a_Position.y + cos(2*c_Pi*a_RV);
        vec4 newPos;
       
        newPos.x = initPosX+ (a_vel.x/10* t) + (0.5 * t * t *c_G.x );
        newPos.y = initPosY+ (a_vel.y/10* t) + (0.5 * t * t *c_G.y );
        newPos.z = 0;
        newPos.w =1;

        gl_Position = newPos;
    }
    else
    {
        gl_Position = vec4(100.0, 100.0, 100.0, 1.0);
    }
   
}
void Thurst()
{
    float newTime = u_Time - a_RV1;
    if(newTime>0){
        float period = a_RV2;
        float t = mod(newTime ,1.0);  // 지속적으로 증가하는 값은 얘가 유일하다.

        float ampScale =t*0.5;
        float amp = 2*(a_RV -0.5)*ampScale;
        float size = (2-t)*2;  // 0~2
      
        vec4 newPos;
       
        newPos.x = a_Position.x+(2*t-1);
        newPos.y = a_Position.y+amp * sin(c_Pi*2 * period * t) * size;
        newPos.z = a_Position.z;
        newPos.w =1;
        gl_Position = newPos;

        v_Grey = 1;
    }
    else{
        gl_Position = vec4(10000,0,0,1);
        v_Grey = 0;
    }
   
}


void main()
{
   Thurst();
     v_Grey = 1;
}