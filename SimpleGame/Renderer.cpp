#include "stdafx.h"
#include "Renderer.h"

Renderer::Renderer(int windowSizeX, int windowSizeY)
{
	Initialize(windowSizeX, windowSizeY);
}


Renderer::~Renderer()
{
}

void Renderer::Initialize(int windowSizeX, int windowSizeY)
{
	//Set window size
	m_WindowSizeX = windowSizeX;
	m_WindowSizeY = windowSizeY;

	//Load shaders
	m_SolidRectShader = CompileShaders("./Shaders/SolidRect.vs", "./Shaders/SolidRect.fs");
	m_TriangleShader = CompileShaders("./Shaders/Triangle.vs", "./Shaders/Triangle.fs");
	m_DrawShader = CompileShaders("./Shaders/FS.vs", "./Shaders/FS.fs");
	//Create VBOs
	/*CreateVertexBufferObjects();*/
	CreateDrawVBO();
	GenParticles(1000);

	// Fill rain info
	int index = 0;
	for(int i = 0; i < 500; ++i)
	{
		float x = rand() / (float)RAND_MAX; ;
		float y = rand() / (float)RAND_MAX; ;
		float sTime = 5 * (float)rand() / (float)RAND_MAX;
		float lTime = 0.5 * (float)rand() / (float)RAND_MAX ;

		m_RainInfo[index] = x; index++;
		m_RainInfo[index] = y; index++;
		m_RainInfo[index] = sTime; index++;
		m_RainInfo[index] = lTime; index++;
	}

	if (m_SolidRectShader > 0 && m_VBORect > 0)
	{
		m_Initialized = true;
	}
}

bool Renderer::IsInitialized()
{
	return m_Initialized;
}

void Renderer::CreateVertexBufferObjects()
{ 
	// vertex : x,y,z 라는 정보를 우선 넣자
	// 기본적인 화면상의 값이 무엇인지 알고 vertex를 넣어줄 것임.
	float rect[]
		=
	{
		-1.f / m_WindowSizeX, -1.f / m_WindowSizeY, 0.f, -1.f / m_WindowSizeX, 1.f / m_WindowSizeY, 0.f, 1.f / m_WindowSizeX, 1.f / m_WindowSizeY, 0.f, //Triangle1
		-1.f / m_WindowSizeX, -1.f / m_WindowSizeY, 0.f,  1.f / m_WindowSizeX, 1.f / m_WindowSizeY, 0.f, 1.f / m_WindowSizeX, -1.f / m_WindowSizeY, 0.f, //Triangle2
	};

	glGenBuffers(1, &m_VBORect);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBORect);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rect), rect, GL_STATIC_DRAW);
	float centerX = 0;
	float centerY = 0;
	float vx = 1;
	float vy = 1;
	float size = 0.1;
	float mass = 1;
	float triangle[]
		=
	{
		centerX-size/2,centerY-size/2,0,  //v0
		mass, vx, vy,
		centerX + size / 2,centerY - size / 2,0,
		mass,vx, vy,//v1
		centerX + size / 2,centerY + size / 2,0,
		mass,vx, vy,//v2 , triangle 1


		centerX - size / 2,centerY - size / 2,0, 
		mass,vx, vy,//v0
		centerX + size / 2,centerY + size / 2,0,
		mass,vx, vy,//v1
		centerX - size / 2,centerY + size / 2,0	,
		mass, vx, vy,//v2 , triangle 2
	};

	glGenBuffers(1, &m_TriangleVBO);  // 0보다 큰값이 보통 넘어옴
	glBindBuffer(GL_ARRAY_BUFFER, m_TriangleVBO);  // GL_ARRAY_BUFFER 바인드 될 놈의 용도를 설명함. 
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW); // GL_ARRAY_BUFFER에 올릴거야 triagle 사이즈 만큼, triangle 데이터를 GL_STATIC_DRAW 방식으로 사용할거야. 한 번 값을 넣고 안바꿀 것이다.

}

void Renderer::AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	//쉐이더 오브젝트 생성
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
	}

	const GLchar* p[1];
	p[0] = pShaderText;
	GLint Lengths[1];
	Lengths[0] = strlen(pShaderText);
	//쉐이더 코드를 쉐이더 오브젝트에 할당
	glShaderSource(ShaderObj, 1, p, Lengths);

	//할당된 쉐이더 코드를 컴파일
	glCompileShader(ShaderObj);

	GLint success;
	// ShaderObj 가 성공적으로 컴파일 되었는지 확인
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];

		//OpenGL 의 shader log 데이터를 가져옴
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		printf("%s \n", pShaderText);
	}

	// ShaderProgram 에 attach!!
	glAttachShader(ShaderProgram, ShaderObj);
}

bool Renderer::ReadFile(char* filename, std::string *target)
{
	std::ifstream file(filename);
	if (file.fail())
	{
		std::cout << filename << " file loading failed.. \n";
		file.close();
		return false;
	}
	std::string line;
	while (getline(file, line)) {
		target->append(line.c_str());
		target->append("\n");
	}
	return true;
}

GLuint Renderer::CompileShaders(char* filenameVS, char* filenameFS)
{
	GLuint ShaderProgram = glCreateProgram(); //빈 쉐이더 프로그램 생성

	if (ShaderProgram == 0) { //쉐이더 프로그램이 만들어졌는지 확인
		fprintf(stderr, "Error creating shader program\n");
	}

	std::string vs, fs;

	//shader.vs 가 vs 안으로 로딩됨
	if (!ReadFile(filenameVS, &vs)) {
		printf("Error compiling vertex shader\n");
		return -1;
	};

	//shader.fs 가 fs 안으로 로딩됨
	if (!ReadFile(filenameFS, &fs)) {
		printf("Error compiling fragment shader\n");
		return -1;
	};

	// ShaderProgram 에 vs.c_str() 버텍스 쉐이더를 컴파일한 결과를 attach함
	AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);

	// ShaderProgram 에 fs.c_str() 프레그먼트 쉐이더를 컴파일한 결과를 attach함
	AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	//Attach 완료된 shaderProgram 을 링킹함
	glLinkProgram(ShaderProgram);

	//링크가 성공했는지 확인
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);

	if (Success == 0) {
		// shader program 로그를 받아옴
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		std::cout << filenameVS << ", " << filenameFS << " Error linking shader program\n" << ErrorLog;
		return -1;
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		std::cout << filenameVS << ", " << filenameFS << " Error validating shader program\n" << ErrorLog;
		return -1;
	}

	glUseProgram(ShaderProgram);
	std::cout << filenameVS << ", " << filenameFS << " Shader compiling is done.";

	return ShaderProgram;
}

void Renderer::DrawSolidRect(float x, float y, float z, float size, float r, float g, float b, float a)
{
	float newX, newY;

	GetGLPosition(x, y, &newX, &newY);

	//Program select
	glUseProgram(m_SolidRectShader);

	glUniform4f(glGetUniformLocation(m_SolidRectShader, "u_Trans"), newX, newY, 0, size);
	glUniform4f(glGetUniformLocation(m_SolidRectShader, "u_Color"), r, g, b, a);

	int attribPosition = glGetAttribLocation(m_SolidRectShader, "a_Position");
	glEnableVertexAttribArray(attribPosition);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBORect);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

	glDrawArrays(GL_TRIANGLES, 0, 100);


}

float gTime = 0;

void Renderer::DrawSolidTriangle()
{
	gTime += 0.001;
	//Program select
	glUseProgram(m_TriangleShader);

	int uTime = glGetUniformLocation(m_TriangleShader, "u_Time"); // ID , 변수
	glUniform1f(uTime, gTime); //float 하나를 넣는다


	// 어티르뷰트가 추가가 되면 여기서 수정이 필요하다.
	int attribPosition = glGetAttribLocation(m_TriangleShader, "a_Position");
	int attribMass = glGetAttribLocation(m_TriangleShader, "a_Mass");   //ID를 받아오는거라서 Int가 맞다 , aMass가 선언은 되지만 사용이 안되어 디버깅 해보면 -1로 뜬다.
	int attribVel = glGetAttribLocation(m_TriangleShader, "a_vel");


	glEnableVertexAttribArray(attribPosition);
	glEnableVertexAttribArray(attribMass);
	glEnableVertexAttribArray(attribVel);

	glBindBuffer(GL_ARRAY_BUFFER, m_TriangleVBO);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_TriangleVBO);
	glVertexAttribPointer(attribMass, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (GLvoid*)(sizeof(float)*3));

	glBindBuffer(GL_ARRAY_BUFFER, m_TriangleVBO);
	glVertexAttribPointer(attribVel, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (GLvoid*)(sizeof(float) * 4));
	glDrawArrays(GL_TRIANGLES, 0, 6);


}



void Renderer::GenParticles(int num)
{
	if (num <= 0)
	{
		m_ParticleVertexCount = 0;
		return;
	}

	std::vector<float> vertices;

	float size = 0.01f;

	for (int i = 0; i < num; ++i)
	{
		// 각 파티클 중심 위치
	/*	float centerX = -0.9f + (1.8f * i / (float)(num - 1 > 0 ? num - 1 : 1));
		float centerY = -0.5f + (1.8f * i / (float)(num - 1 > 0 ? num - 1 : 1));*/
		float centerX = 0;
		float centerY = 0;
		// 각 파티클 속성
		float mass = 1.0f;
		float rv = rand() / (float)RAND_MAX;   // 0.0 ~ 1.0 랜덤값
		float vx = (rand() / (float)RAND_MAX) * 2.0f - 1.0f;
		float vy = (rand() / (float)RAND_MAX) * 2.0f - 1.0f;
		
		float rv1 = rand() / (float)RAND_MAX;   // 0.0 ~ 1.0 랜덤값
		float rv2 = rand() / (float)RAND_MAX;   // 0.0 ~ 1.0 랜덤값
		float triangle[] =
		{
			// triangle 1
			centerX - size / 2, centerY - size / 2, 0.0f,  mass, vx, vy, rv,rv1,rv2,
			centerX + size / 2, centerY - size / 2, 0.0f,  mass, vx, vy, rv,rv1,rv2,
			centerX + size / 2, centerY + size / 2, 0.0f,  mass, vx, vy, rv,rv1,rv2,

			// triangle 2
			centerX - size / 2, centerY - size / 2, 0.0f,  mass, vx, vy, rv ,rv1,rv2,
			centerX + size / 2, centerY + size / 2, 0.0f,  mass, vx, vy, rv, rv1,rv2,
			centerX - size / 2, centerY + size / 2, 0.0f,  mass, vx, vy, rv, rv1,rv2
		};

		vertices.insert(vertices.end(), triangle, triangle + 6*9);
	}

	if (m_ParticleVBO != 0)
	{
		glDeleteBuffers(1, &m_ParticleVBO);
		m_ParticleVBO = 0;
	}

	glGenBuffers(1, &m_ParticleVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_ParticleVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	m_ParticleVertexCount = num * 6;
}

void Renderer::DrawParticles(float x, float y, float z, float size)
{
	if (m_ParticleVBO == 0 || m_ParticleVertexCount == 0)
		return;

	gTime += 0.0001f;

	float newX, newY;
	GetGLPosition(x, y, &newX, &newY);

	glUseProgram(m_TriangleShader);

	int uTime = glGetUniformLocation(m_TriangleShader, "u_Time");
	glUniform1f(uTime, gTime);

	int attribPosition = glGetAttribLocation(m_TriangleShader, "a_Position");
	int attribMass = glGetAttribLocation(m_TriangleShader, "a_Mass");
	int attribVel = glGetAttribLocation(m_TriangleShader, "a_vel");
	int attribRV = glGetAttribLocation(m_TriangleShader, "a_RV");
	int attribRV1 = glGetAttribLocation(m_TriangleShader, "a_RV1");
	int attribRV2 = glGetAttribLocation(m_TriangleShader, "a_RV2");

	glEnableVertexAttribArray(attribPosition);
	glEnableVertexAttribArray(attribMass);
	glEnableVertexAttribArray(attribVel);
	glEnableVertexAttribArray(attribRV);
	glEnableVertexAttribArray(attribRV1);
	glEnableVertexAttribArray(attribRV2);

	glBindBuffer(GL_ARRAY_BUFFER, m_ParticleVBO);

	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), 0);
	glVertexAttribPointer(attribMass, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (GLvoid*)(sizeof(float) * 3));
	glVertexAttribPointer(attribVel, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (GLvoid*)(sizeof(float) * 4));
	glVertexAttribPointer(attribRV, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (GLvoid*)(sizeof(float) * 6));
	glVertexAttribPointer(attribRV1, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (GLvoid*)(sizeof(float) * 7));
	glVertexAttribPointer(attribRV2, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (GLvoid*)(sizeof(float) * 8));

	glDrawArrays(GL_TRIANGLES, 0, m_ParticleVertexCount);

	glDisableVertexAttribArray(attribPosition);
	glDisableVertexAttribArray(attribMass);
	glDisableVertexAttribArray(attribVel);
	glDisableVertexAttribArray(attribRV);
	glDisableVertexAttribArray(attribRV1);
	glDisableVertexAttribArray(attribRV2);
}
void Renderer::GetGLPosition(float x, float y, float *newX, float *newY)
{
	*newX = x * 2.f / m_WindowSizeX;
	*newY = y * 2.f / m_WindowSizeY;
}

void Renderer::CreateDrawVBO()
{
	float rectFS[] =
	{
		// x,  y,  z,   tx, ty
		-1, -1, 0,   0, 1,
		 1,  1, 0,   1, 0,
		-1,  1, 0,   0, 0,   // triangle 1

		-1, -1, 0,   0, 1,
		 1, -1, 0,   1, 1,
		 1,  1, 0,   1, 0
	};

	glGenBuffers(1, &m_VBODraw);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBODraw);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectFS), rectFS, GL_STATIC_DRAW);
}
void Renderer::DrawDraw()
{
	glUseProgram(m_DrawShader);
	GLuint shader = m_DrawShader;

	int attribPosition = glGetAttribLocation(shader, "a_Pos");
	int attribTex = glGetAttribLocation(shader, "a_Tex");

	gTime += 0.0016f;


	int uTime = glGetUniformLocation(m_DrawShader, "u_Time");
	glUniform1f(uTime, gTime);

	int uPoints = glGetUniformLocation(m_DrawShader, "u_Points");
	glUniform4fv(uPoints, 1000, m_RainInfo);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBODraw);

	glEnableVertexAttribArray(attribPosition);
	glVertexAttribPointer(
		attribPosition,
		3,
		GL_FLOAT,
		GL_FALSE,
		5 * sizeof(float),
		(void*)0
	);

	glEnableVertexAttribArray(attribTex);
	glVertexAttribPointer(
		attribTex,
		2,
		GL_FLOAT,
		GL_FALSE,
		5 * sizeof(float),
		(GLvoid*)(3 * sizeof(float))
	);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}