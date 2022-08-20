#pragma once
#include "../nclgl/OGLRenderer.h"
//#include "../nclgl/Camera.h"
//#include "../nclgl/HeightMap.h"
class HeightMap;
class Camera;

class Renderer :public OGLRenderer
{
public:
	Renderer(Window& parent);
	~Renderer(void);
	void RenderScene() override;
	void UpdateScene(float dt) override;
protected:
	HeightMap* heightMap;
	Shader* shader;
	Camera* camera;
	GLuint terrainTex;
};

