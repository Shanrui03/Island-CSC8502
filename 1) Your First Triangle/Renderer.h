#pragma once
#include "../NCLGL/OGLRenderer.h"
#include "../nclgl/Mesh.h"

class Renderer :public OGLRenderer
{
public:
	Renderer(Window& parent);
	virtual ~Renderer(void);
	virtual void RenderScene();

protected:
	Mesh* triangle;
	Shader* basicShader;
};


