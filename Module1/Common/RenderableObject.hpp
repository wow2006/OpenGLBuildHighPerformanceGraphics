#pragma once
#include "GLSLShader.hpp"

class RenderableObject
{
public:
	RenderableObject();

	virtual ~RenderableObject();

	void Render(const float* MVP);

	virtual int GetTotalVertices()=0;
	virtual int GetTotalIndices()=0;
	virtual GLenum GetPrimitiveType() =0;

	virtual void FillVertexBuffer(GLfloat* pBuffer)=0;
	virtual void FillIndexBuffer(GLuint* pBuffer)=0;

	virtual void SetCustomUniforms(){}
	GLSLShader* GetShader();

	void Init();
	void Destroy();

protected:
	GLuint vaoID = 0;
	GLuint vboVerticesID = 0;
	GLuint vboIndicesID = 0;

	GLSLShader shader;

	GLenum primType = 0;
	int totalVertices = 0, totalIndices = 0;
};

