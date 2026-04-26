#pragma once

#include <string>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include<vector>
#include "Dependencies\glew.h"

class Renderer
{
public:
	Renderer(int windowSizeX, int windowSizeY);
	~Renderer();

	bool IsInitialized();
	void DrawSolidRect(float x, float y, float z, float size, float r, float g, float b, float a);
	void DrawSolidTriangle();

	void DrawDraw();


	// 여러 개 사각형용
	void GenParticles(int num);
	void DrawParticles(float x, float y, float z, float size);
private:
	void Initialize(int windowSizeX, int windowSizeY);
	GLuint CreatePngTexture(char* filePath, GLuint samplingMethod);

	bool ReadFile(char* filename, std::string *target);
	void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);
	GLuint CompileShaders(char* filenameVS, char* filenameFS);

	void CreateVertexBufferObjects();
	void GetGLPosition(float x, float y, float *newX, float *newY);
	void CreateDrawVBO();
	bool m_Initialized = false;
	
	unsigned int m_WindowSizeX = 0;
	unsigned int m_WindowSizeY = 0;

	GLuint m_VBORect = 0;
	GLuint m_SolidRectShader = 0;

	GLuint m_TriangleVBO = 0; //VBO는 한 번 만들면 여기저기서 사용이 된다. 그래서 클래스 범위여야한다.
	GLuint m_TriangleShader = 0;


	GLuint m_VBODraw = 0;
	GLuint m_DrawShader = 0;

	//RainDrop
	float m_RainInfo[500 * 4];


	// particle용
	GLuint m_ParticleVBO = 0;
	int m_ParticleVertexCount = 0;

	//texcture
	GLuint m_RGBTexture = 0;
	GLuint m_NumTexture [10];
	GLuint m_NumsTexture = 0;
};

