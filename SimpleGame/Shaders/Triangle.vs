#version 330

uniform float u_Time;

in vec3 a_Position;
in float a_Mass;
in vec2 a_vel;
in float a_RV;

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

void Falling()
{
    float t=mod(u_Time+ a_RV, 1.0);
    vec4 newPos;
    newPos.x = a_Position.x+ (a_vel.x* t) + (t*t*1/2 *c_G.x );
    newPos.y = a_Position.y+ (a_vel.y* t) + (t*t*1/2 *c_G.y );
    newPos.z =0;
    newPos.w =1;

    gl_Position = newPos;
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
    float t=mod(u_Time, 1.0);
    float initPosX =a_Position.x + sin(2*c_Pi*a_RV);
    float initPosY =a_Position.y + cos(2*c_Pi*a_RV);
    vec4 newPos;
    newPos.x = initPosX+ (a_vel.x* t) + (t*t*1/2 *c_G.x );
    newPos.y = initPosY+ (a_vel.y* t) + (t*t*1/2 *c_G.y );
    newPos.z =0;
    newPos.w =1;

    gl_Position = newPos;
}
void main()
{
    Answer_1();
}