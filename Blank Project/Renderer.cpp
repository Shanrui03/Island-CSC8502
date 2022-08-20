#include "Renderer.h"
#include <algorithm>
using namespace std;
const int POST_PASSES = 10;
#define SHADOWSIZE 2048


//initaiise and destructor part
Renderer::Renderer(Window& parent) : OGLRenderer(parent) {

//load meshes(animations and materials)
	skyboxQuad = Mesh::GenerateQuad();

	sphere = Mesh::LoadFromMeshFile("Sphere.msh");

	palmtree = Mesh::LoadFromMeshFile("Palm_Tree.msh");

	fritree = Mesh::LoadFromMeshFile("Fir_Tree.msh");

	rock = Mesh::LoadFromMeshFile("Rock.msh");


	soldier = Mesh::LoadFromMeshFile("Role_T.msh");
	soldierAnim = new MeshAnimation("Role_T.anm");
	soldierMat = new MeshMaterial("Role_T.mat");

	female = Mesh::LoadFromMeshFile("Female Dancer(Clone).msh");
	femaleAnim = new MeshAnimation("Female Dancer(Clone).anm");
	femaleMat = new MeshMaterial("Female Dancer(Clone).mat");


	heightMap = new HeightMap(TEXTUREDIR"noise.png");



//load Textures
	treeTex = SOIL_load_OGL_texture(TEXTUREDIR"tree_diffuse.png",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,SOIL_FLAG_MIPMAPS);
	if (!treeTex)
	{
		return;
	}

	earthTex = SOIL_load_OGL_texture(
		TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	if (!earthTex)
	{
		return;
	}

	earthBump = SOIL_load_OGL_texture(
		TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	if (!earthBump)
	{
		return;
	}

	waterTex = SOIL_load_OGL_texture(
		TEXTUREDIR"water.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	if (!waterTex)
	{
		return;
	}

	waterBump = SOIL_load_OGL_texture(
		TEXTUREDIR"waterbump.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	if (!waterBump)
	{
		return;
	}

	rockTex = SOIL_load_OGL_texture(
		TEXTUREDIR"Stone_diffuse.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	if (!rockTex)
	{
		return;
	}

	glassTex = SOIL_load_OGL_texture(
		TEXTUREDIR"stainedglass.tga",SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
	if (!glassTex)
	{
		return;
	}

	rockBump = SOIL_load_OGL_texture(
		TEXTUREDIR"Stone_normals.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	if (!rockTex)
	{
		return;
	}

	cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR"CloudySummerDaySkybox_left.tga", TEXTUREDIR"CloudySummerDaySkybox_right.tga",
		TEXTUREDIR"CloudySummerDaySkybox_up.tga", TEXTUREDIR"CloudySummerDaySkybox_down.tga",
		TEXTUREDIR"CloudySummerDaySkybox_front.tga", TEXTUREDIR"CloudySummerDaySkybox_back.tga",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
	if (!cubeMap)
	{
		return;
	}

	for (int i = 0; i < soldier->GetSubMeshCount(); ++i)
	{
		const MeshMaterialEntry* matEntry =
			soldierMat->GetMaterialForLayer(i);

		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO,
			SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		if (!texID)
		{
			return;
		}
		soldiermatTextures.emplace_back(texID);
	}

	for (int i = 0; i < female->GetSubMeshCount(); ++i)
	{
		const MeshMaterialEntry* matEntry =
			femaleMat->GetMaterialForLayer(i);

		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO,
			SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		if (!texID)
		{
			return;
		}
		femalematTextures.emplace_back(texID);
	}




	SetTextureRepeating(earthTex, true);
	SetTextureRepeating(waterBump, true);
	SetTextureRepeating(earthBump, true);
	SetTextureRepeating(waterTex, true);
	SetTextureRepeating(treeTex, true);
	SetTextureRepeating(rockTex, true);
	SetTextureRepeating(rockBump, true);



//load shaders
	skyboxShader = new Shader(
		"skyboxVertex.glsl", "skyboxFragment.glsl"
	);
	if (!skyboxShader->LoadSuccess())
	{
		return;
	}

	blurShader = new Shader(
		"TexturedVertex.glsl", "TexturedFragment.glsl"
	);
	if (!blurShader->LoadSuccess())
	{
		return;
	}

	emitterShader = new Shader(
		"vertex.glsl", "fragment.glsl", "geometry.glsl"
	);
	if (!emitterShader->LoadSuccess())
	{
		return;
	}

	processShader = new Shader(
		"TexturedVertex.glsl", "processfrag.glsl"
	);
	if (!processShader->LoadSuccess())
	{
		return;
	}

	lightShader = new Shader(
		"BumpVertex.glsl", "BumpFragment.glsl"
	);
	if (!lightShader->LoadSuccess())
	{
		return;
	}

	waterShader = new Shader(
		"reflectVertex.glsl", "reflectFragment.glsl"
	);
	if (!waterShader->LoadSuccess())
	{
		return;
	}


	scenegraphShader = new Shader(
		"PerPixelVertex.glsl", "PerPixelFragment.glsl"
	);
	if (!scenegraphShader->LoadSuccess())
	{
		return;
	}


	soldierShader = new Shader(
		"SkinningVertex.glsl", "PerPixelFragment.glsl"
	);
	if (!soldierShader->LoadSuccess())
	{
		return;
	}

	postShader = new Shader(
		"TexturedVertex.glsl", "processfrag.glsl"
	);
	if (!postShader->LoadSuccess())
	{
		return;
	}


	heightmapNolightShader = new Shader(
		"BumpVertex.glsl", "bufferFragment.glsl"
	);
	if (!heightmapNolightShader->LoadSuccess())
	{
		return;
	}

	pointlightShader = new Shader(
		"pointlightvert.glsl", "pointlightfrag.glsl"
	);
	if (!pointlightShader->LoadSuccess())
	{
		return;
	}

	combineShader = new Shader(
		"combinevert.glsl", "combinefrag.glsl"
	);
	if (!combineShader->LoadSuccess())
	{
		return;
	}

	shadowShader = new Shader(
		"shadowVertex.glsl", "shadowFragment.glsl"
	);
	if (!shadowShader->LoadSuccess())
	{
		return;
	}

	watershadowShader = new Shader(
		"reflectShadowVertex.glsl", "reflectShadowFragment.glsl"
	);
	if (!watershadowShader->LoadSuccess())
	{
		return;
	}

	bumpshadowShader = new Shader(
		"BumpShadowVertex.glsl", "BumpShadowFragment.glsl"
	);
	if (!bumpshadowShader->LoadSuccess())
	{
		return;
	}

	PerPixelshaodwShader = new Shader(
		"PerPixelShadowVertex.glsl", "PerPixelShadowFragment.glsl"
	);
	if (!PerPixelshaodwShader->LoadSuccess())
	{
		return;
	}

	soldiershadowShader = new Shader(
		"SkinningshadowVertex.glsl", "shadowFragment.glsl"
	);
	if (!soldiershadowShader->LoadSuccess())
	{
		return;
	}

	animationshadowShader = new Shader(
		"SkinningVertex01.glsl", "PerPixelShadowFragment.glsl"
	);
	if (!animationshadowShader->LoadSuccess())
	{
		return;
	}

//create scenegraph
	Vector3 heightmapSize = heightMap->GetHeightmapSize();

	root = new SceneNode();

	rain = new Rain();

	root->AddChild(rain);

	for (int i = 1; i < 4; ++i)
	{
		SceneNode* s = new SceneNode();
		s->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		s->SetTransform(Matrix4::Translation(heightmapSize * Vector3(0.3f * i, 0.8f, 0.3f * i)));
		s->SetModelScale(Vector3(100.0f, 100.0f, 100.0f));
		s->SetMesh(palmtree);
		s->SetTexture(treeTex);
		s->SetType(TYPE_NORMAL);
		root->AddChild(s);
	}

	for (int i = 0; i < 5; ++i)
	{
		SceneNode* s = new SceneNode();
		s->SetColour(Vector4(1.0f, 1.0f, 1.0f, 0.5f));
		s->SetTransform(Matrix4::Translation(heightmapSize* Vector3(0.2f * i, 0.8f, 0.2f * i)));
		s->SetModelScale(Vector3(100.0f, 100.0f, 100.0f));
		s->SetBoundingRadius(100.0f);
		s->SetMesh(skyboxQuad);
		s->SetTexture(glassTex);
		s->SetType(TYPE_NORMAL);
		root->AddChild(s);
	}


	soldierNode = new SceneNode();
	soldierNode->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	soldierNode->SetTransform(Matrix4::Translation(heightmapSize * Vector3(0.7f, 0.3f, 0.8f)));
	soldierNode->SetModelScale(Vector3(50.0f, 50.0f, 50.0f));
	soldierNode->SetMesh(soldier);
	soldierNode->SetTextures(soldiermatTextures);
	soldierNode->SetAnimation(soldierAnim);
	soldierNode->SetType(TYPE_ANIMATION);
	root->AddChild(soldierNode);


	femaleNode = new SceneNode();
	femaleNode->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	femaleNode->SetTransform(Matrix4::Translation(heightmapSize* Vector3(0.4f, 0.6f, 0.4f)));
	femaleNode->SetModelScale(Vector3(50.0f, 50.0f, 50.0f));
	femaleNode->SetMesh(female);
	femaleNode->SetTextures(femalematTextures);
	femaleNode->SetAnimation(femaleAnim);
	femaleNode->SetType(TYPE_ANIMATION);
	root->AddChild(femaleNode);



	for (int i = 1; i < 4; ++i)
	{
		SceneNode* s = new SceneNode();
		s->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		s->SetTransform(Matrix4::Translation(heightmapSize * Vector3(0.25f * (4-i), 0.4f, 0.3f * i)));
		s->SetModelScale(Vector3(100.0f, 100.0f, 100.0f));
		s->SetMesh(fritree);
		s->SetTexture(treeTex);
		s->SetType(TYPE_NORMAL);
		root->AddChild(s);
	}

	for (int i = 1; i < 3; ++i)
	{
		SceneNode* s = new SceneNode();
		s->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		s->SetTransform(Matrix4::Translation(heightmapSize * Vector3(0.15f * (3 - i), 0.6f, 0.15f * i)) * 
			Matrix4::Rotation(50.0f * i, Vector3(0, 1, 0)));
		s->SetModelScale(Vector3(5.0f, 5.0f, 5.0f));
		s->SetMesh(rock);
		s->SetTexture(rockTex);
		s->SetBumpTexture(rockBump);
		s->SetType(TYPE_BUMP);
		root->AddChild(s);
	}

//create camera and light
	camera = new Camera(-15.0f, 50.0f,
		heightmapSize * Vector3(1.3f, 3.0f, 1.3f));
	skycamera = new Camera(-90.0f, 0.0f,
		heightmapSize * Vector3(0.5f, 10.0f, 0.5f));
	light = new Light(heightmapSize * Vector3(0.5f, 1.5f, 1.0f),
		Vector4(1, 1, 1, 1), heightmapSize.x);
	light2 = new Light(heightmapSize * Vector3(0.5f, 2.0f, 0.5f),
		Vector4(1, 1, 1, 1), 10000.0f);
	pointLights = new Light[40];

	for (int i = 0; i < 40; ++i) 
	{
		Light& l = pointLights[i];
		l.SetPosition(Vector3(rand() % (int)heightmapSize.x, 150.0f, rand() % (int)heightmapSize.z));

		l.SetColour(Vector4(0.5f + (float)(rand() / (float)RAND_MAX),
			0.5f + (float)(rand() / (float)RAND_MAX),
			0.5f + (float)(rand() / (float)RAND_MAX),
			1));

		l.SetRadius(250.0f + (rand() % 250));
	}
	

//set up projMatrix
	defaultprojMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float)width / (float)height, 45.0f);
	projMatrix = defaultprojMatrix;


//water properties
	waterRotate = 0.0f;
	waterCycle = 0.0f;

//animation properties
	soldiercurrentFrame = 0;
	soldierframeTime = 0.0f;

	femalecurrentFrame = 0;
	femaleframeTime = 0.0f;

	rockmovTime = 0.0f;
	rockmodelMatrix = Matrix4::Translation(heightmapSize * Vector3(0.5f, 1.5f, 0.5f)) * Matrix4::Scale(Vector3(10.0f, 10.0f, 10.0f)) * Matrix4::Rotation(90, Vector3(1, 0, 0));

//generate FBOs
	glGenFramebuffers(1, &bufferFBO);
	glGenFramebuffers(1, &processFBO);
	glGenFramebuffers(1, &shadowFBO);
	glGenFramebuffers(1, &pointLightFBO);
	glGenFramebuffers(1, &blurbufferFBO);

//generate color attachment
	GLenum buffers[2] = {
	GL_COLOR_ATTACHMENT0,
	GL_COLOR_ATTACHMENT1
	};

	GenerateScreenTexture(bufferDepthTex, true);
	if (!bufferDepthTex)
	{
		return;
	}
	for (int i = 0; i < 2; ++i)
	{
		GenerateScreenTexture(bufferColourTex[i]);
		if (!bufferColourTex[i])
		{
			return;
		}
	}
	GenerateScreenTexture(bufferNormalTex);
	if (!bufferNormalTex)
	{
		return;
	}
	GenerateScreenTexture(lightDiffuseTex);
	if (!lightDiffuseTex)
	{
		return;
	}
	GenerateScreenTexture(lightSpecularTex);
	if (!lightSpecularTex)
	{
		return;
	}
	//generate shadowTex
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	if (!shadowTex)
	{
		return;
	}
	//generate our blur depth texure
	glGenTextures(1, &blurbufferDepthTex);
	glBindTexture(GL_TEXTURE_2D, blurbufferDepthTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height,
		0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	//generate blur colour texture
	for (int i = 0; i < 2; ++i)
	{
		glGenTextures(1, &blurbufferColourTex[i]);
		glBindTexture(GL_TEXTURE_2D, blurbufferColourTex[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}


//bind FBO's attachments
	//deferred rendering FBO
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, bufferColourTex[0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 
		GL_TEXTURE_2D, bufferNormalTex, 0);
	glDrawBuffers(2, buffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
		GL_FRAMEBUFFER_COMPLETE)
	{
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//light FBO 
	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, lightDiffuseTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
		GL_TEXTURE_2D, lightSpecularTex, 0);
	glDrawBuffers(2, buffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
	{
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//shadow FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
		GL_FRAMEBUFFER_COMPLETE)
	{
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//blur FBO
	glBindFramebuffer(GL_FRAMEBUFFER, blurbufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, blurbufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
		GL_TEXTURE_2D, blurbufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, blurbufferColourTex[0], 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
		GL_FRAMEBUFFER_COMPLETE)
	{
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//enable functions
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	init = true;
}
Renderer::~Renderer(void) {
	//delete meshes
	delete skyboxQuad;
	delete soldier;
	delete fritree;
	delete palmtree;
	delete rock;
	delete female;
	delete sphere;

	//delete shaders
	delete lightShader;
	delete skyboxShader;
	delete scenegraphShader;
	delete waterShader;
	delete emitterShader;
	delete blurShader;
	delete processShader;
	delete soldierShader;
	delete postShader;
	delete heightmapNolightShader;
	delete pointlightShader;
	delete combineShader;
	delete shadowShader;
	delete watershadowShader;
	delete bumpshadowShader;
	delete PerPixelshaodwShader;
	delete soldiershadowShader;
	delete animationshadowShader;

	//delete others
	delete camera;
	delete light;
	delete light2;
	delete root;
	delete[] pointLights;


	//delete FBOs and attachments
	glDeleteTextures(2, bufferColourTex);
	glDeleteTextures(2, blurbufferColourTex);
	glDeleteTextures(1, &bufferNormalTex);
	glDeleteTextures(1, &bufferDepthTex);
	glDeleteTextures(1, &blurbufferDepthTex);
	glDeleteTextures(1, &lightDiffuseTex);
	glDeleteTextures(1, &lightSpecularTex);
	glDeleteTextures(1, &shadowTex);

	glDeleteFramebuffers(1, &bufferFBO);
	glDeleteFramebuffers(1, &processFBO);
	glDeleteFramebuffers(1, &pointLightFBO);
	glDeleteFramebuffers(1, &blurbufferFBO);
	glDeleteFramebuffers(1, &shadowFBO);
}





//camera part
void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	waterRotate += dt * 0.5f;
	waterCycle += dt * 0.15f;

	soldierframeTime -= dt;
	while (soldierframeTime < 0.0f)
	{
		soldiercurrentFrame = (soldiercurrentFrame + 1) % soldierAnim->GetFrameCount();
		soldierframeTime += 1.0f / soldierAnim->GetFrameRate();
	}
	soldierNode->SetCurrentFrame(soldiercurrentFrame);

	femaleframeTime -= dt;
	while (femaleframeTime < 0.0f)
	{
		femalecurrentFrame = (femalecurrentFrame + 1) % femaleAnim->GetFrameCount();
		femaleframeTime += 1.0f / femaleAnim->GetFrameRate();
	}
	femaleNode->SetCurrentFrame(femalecurrentFrame);

	rockmovTime += dt * 0.5;
	Vector3 hSize = heightMap->GetHeightmapSize();
	rockmodelMatrix = Matrix4::Translation(hSize * Vector3(0.4f + sin(rockmovTime), 1.5f, 0.4f)) *
		Matrix4::Scale(Vector3(10.0f, 10.0f, 10.0f)) * 
		Matrix4::Rotation(rockmovTime * 10 , hSize * Vector3(0.0f,0.5f,0.0f));

	root->Update(dt);
}
void Renderer::AutoUpdateCamera(float dt)
{
	Vector3 heightmapsize = heightMap->GetHeightmapSize();
	camera->AutoUpdateCamera(heightmapsize, dt);
	viewMatrix = camera->BuildViewMatrix();
	waterRotate += dt * 0.5f;
	waterCycle += dt * 0.15f;

	soldierframeTime -= dt;
	while (soldierframeTime < 0.0f)
	{
		soldiercurrentFrame = (soldiercurrentFrame + 1) % soldierAnim->GetFrameCount();
		soldierframeTime += 1.0f / soldierAnim->GetFrameRate();
	}
	soldierNode->SetCurrentFrame(soldiercurrentFrame);

	femaleframeTime -= dt;
	while (femaleframeTime < 0.0f)
	{
		femalecurrentFrame = (femalecurrentFrame + 1) % femaleAnim->GetFrameCount();
		femaleframeTime += 1.0f / femaleAnim->GetFrameRate();
	}
	femaleNode->SetCurrentFrame(femalecurrentFrame);

	rockmovTime += dt * 0.5;
	Vector3 hSize = heightMap->GetHeightmapSize();
	rockmodelMatrix = Matrix4::Translation(hSize * Vector3(0.4f + sin(rockmovTime), 1.5f, 0.4f)) *
		Matrix4::Scale(Vector3(10.0f, 10.0f, 10.0f)) *
		Matrix4::Rotation(rockmovTime * 10, hSize * Vector3(0.0f, 0.5f, 0.0f));

	root->Update(dt);
}
void Renderer::AutoUpdateCamera2(float dt,int SW)
{
	Vector3 heightmapsize = heightMap->GetHeightmapSize();
	camera->AutoUpdateCamera2(heightmapsize, SW,dt);
	viewMatrix = camera->BuildViewMatrix();
	waterRotate += dt * 0.5f;
	waterCycle += dt * 0.15f;

	soldierframeTime -= dt;
	while (soldierframeTime < 0.0f)
	{
		soldiercurrentFrame = (soldiercurrentFrame + 1) % soldierAnim->GetFrameCount();
		soldierframeTime += 1.0f / soldierAnim->GetFrameRate();
	}
	soldierNode->SetCurrentFrame(soldiercurrentFrame);

	femaleframeTime -= dt;
	while (femaleframeTime < 0.0f)
	{
		femalecurrentFrame = (femalecurrentFrame + 1) % femaleAnim->GetFrameCount();
		femaleframeTime += 1.0f / femaleAnim->GetFrameRate();
	}
	femaleNode->SetCurrentFrame(femalecurrentFrame);

	rockmovTime += dt * 0.5;
	Vector3 hSize = heightMap->GetHeightmapSize();
	rockmodelMatrix = Matrix4::Translation(hSize * Vector3(0.4f + sin(rockmovTime), 1.5f, 0.4f)) *
		Matrix4::Scale(Vector3(10.0f, 10.0f, 10.0f)) *
		Matrix4::Rotation(rockmovTime * 10, hSize * Vector3(0.0f, 0.5f, 0.0f));

	root->Update(dt);
}
void Renderer::SetDefultCamera(float dt)
{
	Vector3 heightmapsize = heightMap->GetHeightmapSize();
	camera->SetPosition(heightmapsize * Vector3(1.3f, 3.0f, 1.3f));
	camera->SetPitch(-15.0f);
	camera->SetYaw(50.0f);
	viewMatrix = camera->BuildViewMatrix();
	waterRotate += dt * 0.5f;
	waterCycle += dt * 0.15f;

	soldierframeTime -= dt;
	while (soldierframeTime < 0.0f)
	{
		soldiercurrentFrame = (soldiercurrentFrame + 1) % soldierAnim->GetFrameCount();
		soldierframeTime += 1.0f / soldierAnim->GetFrameRate();
	}
	soldierNode->SetCurrentFrame(soldiercurrentFrame);

	femaleframeTime -= dt;
	while (femaleframeTime < 0.0f)
	{
		femalecurrentFrame = (femalecurrentFrame + 1) % femaleAnim->GetFrameCount();
		femaleframeTime += 1.0f / femaleAnim->GetFrameRate();
	}
	femaleNode->SetCurrentFrame(femalecurrentFrame);

	rockmovTime += dt * 0.5;
	Vector3 hSize = heightMap->GetHeightmapSize();
	rockmodelMatrix = Matrix4::Translation(hSize * Vector3(0.4f + sin(rockmovTime), 1.5f, 0.4f)) *
		Matrix4::Scale(Vector3(10.0f, 10.0f, 10.0f)) *
		Matrix4::Rotation(rockmovTime * 10, hSize * Vector3(0.0f, 0.5f, 0.0f));

	root->Update(dt);
}





//scenegraph part
void Renderer::BuildNodeLists(SceneNode* from)
{

	Vector3 dir = from->GetWorldTransform().GetPositionVector() - camera->GetPosition();
	from->SetCameraDistance(Vector3::Dot(dir, dir));

	if (from->GetColour().w < 1.0f)
	{
		transparentNodeList.push_back(from);
	}
	else
	{
		nodeList.push_back(from);
	}

	for (vector<SceneNode*>::const_iterator i = from->GetChildIteratorStart(); i != from->GetChildIteratorEnd(); ++i)
	{
		BuildNodeLists((*i));
	}
}
void Renderer::SortNodeLists()
{
	std::sort(transparentNodeList.rbegin(),
		transparentNodeList.rend(),
		SceneNode::CompareByCameraDistance);
	std::sort(nodeList.begin(),
		nodeList.end(),
		SceneNode::CompareByCameraDistance);
}
void Renderer::DrawNodes(Camera* camera,bool SW,bool shadowSW)
{
	for (const auto& i : nodeList)
	{
		DrawNode(camera,i,SW,shadowSW);
	}
	for (const auto& i : transparentNodeList)
	{
		DrawNode(camera,i,SW,shadowSW);
	}
}
void Renderer::DrawNode(Camera* camera,SceneNode* n,bool SW,bool shadowSW)
{
	if (n->GetMesh())
	{
		//determine to use which shader
		if (SW)
		{
			if (shadowSW)
			{
				if (n->GetType() == TYPE_ANIMATION)
				{
					BindShader(animationshadowShader);
					viewMatrix = camera->BuildViewMatrix();
					projMatrix = defaultprojMatrix;
					SetShaderLight(*light2);
					glUniform1i(glGetUniformLocation(GetCurrentShader()->GetProgram(), "shadowTex"), 2);
					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_2D, shadowTex);
				}
				else if (n->GetType() == TYPE_BUMP)
				{
					viewMatrix = camera->BuildViewMatrix();
					projMatrix = defaultprojMatrix;
					SetShaderLight(*light2);
					glUniform1i(glGetUniformLocation(GetCurrentShader()->GetProgram(), "shadowTex"), 2);
					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_2D, shadowTex);
				}
				else if (n->GetType() == TYPE_NORMAL)
				{
					BindShader(PerPixelshaodwShader);
					SetShaderLight(*light2);
					viewMatrix = camera->BuildViewMatrix();
					projMatrix = defaultprojMatrix;
					glUniform1i(glGetUniformLocation(GetCurrentShader()->GetProgram(), "shadowTex"), 2);
					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_2D, shadowTex);
				}
			}
			else 
			{
				if (n->GetType() == TYPE_ANIMATION)
				{
					BindShader(soldierShader);
					SetShaderLight(*light);
					viewMatrix = camera->BuildViewMatrix();
					projMatrix = defaultprojMatrix;
				}
				else if (n->GetType() == TYPE_BUMP)
				{
					BindShader(lightShader);
					SetShaderLight(*light);
					viewMatrix = camera->BuildViewMatrix();
					projMatrix = defaultprojMatrix;
				}
				else if (n->GetType() == TYPE_NORMAL)
				{
					BindShader(scenegraphShader);
					SetShaderLight(*light);
					viewMatrix = camera->BuildViewMatrix();
					projMatrix = defaultprojMatrix;
				}
			}
		}

		if(n == soldierNode || n == femaleNode)
		{ 
			glUniform1i(glGetUniformLocation(GetCurrentShader()->GetProgram(), "diffuseTex"), 0);
			UpdateShaderMatrices();

			Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
			glUniformMatrix4fv(glGetUniformLocation(GetCurrentShader()->GetProgram(), "modelMatrix"), 1, false, model.values);

			glUniform3fv(glGetUniformLocation(GetCurrentShader()->GetProgram(),"cameraPos"), 1, (float*)&camera->GetPosition());
		    nodeTexture = soldiermatTextures[0];
			glUniform1i(glGetUniformLocation(GetCurrentShader()->GetProgram(), "useTexture"), nodeTexture);


			vector<Matrix4> frameMatrices;

			const Matrix4* invBindPose = n->GetMesh()->GetInverseBindPose();
			const Matrix4* frameData = n->GetAnimation()->GetJointData(n->GetCurrentFrame());

			for (unsigned int i = 0; i < n->GetMesh()->GetJointCount(); ++i)
			{
				frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
			}

			int j = glGetUniformLocation(GetCurrentShader()->GetProgram(), "joints");
			glUniformMatrix4fv(j, frameMatrices.size(), false,
				(float*)frameMatrices.data());

			for (int i = 0; i < n->GetMesh()->GetSubMeshCount(); ++i)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, n->GetTextures()[i]);
				n->GetMesh()->DrawSubMesh(i);
			}
		}
		else if (n->GetBumpTexture() == rockBump)
		{
			UpdateShaderMatrices();



			Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
			glUniformMatrix4fv(glGetUniformLocation(GetCurrentShader()->GetProgram(), "modelMatrix"), 1, false, model.values);
			glUniform4fv(glGetUniformLocation(GetCurrentShader()->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());
			glUniform3fv(glGetUniformLocation(GetCurrentShader()->GetProgram(),"cameraPos"), 1, (float*)&camera->GetPosition());

			nodeTexture = n->GetTexture();
			glUniform1i(glGetUniformLocation(GetCurrentShader()->GetProgram(), "diffuseTex"), 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, nodeTexture);

			nodeBumpTexture = n->GetBumpTexture();
			glUniform1i(glGetUniformLocation(GetCurrentShader()->GetProgram(),"bumpTex"), 1);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, nodeBumpTexture);


			glUniform1i(glGetUniformLocation(GetCurrentShader()->GetProgram(), "useTexture"), nodeTexture);
			n->Draw(*this);
		}
		else
		{
			UpdateShaderMatrices();



			glUniform1i(glGetUniformLocation(GetCurrentShader()->GetProgram(), "diffuseTex"), 0);

			Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
			glUniformMatrix4fv(glGetUniformLocation(GetCurrentShader()->GetProgram(), "modelMatrix"), 1, false, model.values);
			glUniform4fv(glGetUniformLocation(GetCurrentShader()->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());

			glUniform3fv(glGetUniformLocation(GetCurrentShader()->GetProgram(),"cameraPos"), 1, (float*)&camera->GetPosition());

			nodeTexture = n->GetTexture();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, nodeTexture);


			glUniform1i(glGetUniformLocation(GetCurrentShader()->GetProgram(), "useTexture"), nodeTexture);
			n->Draw(*this);
		}
	}
}





//Sky,water,island part
void Renderer::DrawSkybox()
{
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = defaultprojMatrix;
	modelMatrix.ToIdentity();

	UpdateShaderMatrices();

	skyboxQuad->Draw();

	glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glDepthMask(GL_TRUE);
}
void Renderer::DrawHeightmap(Camera* camera,bool SW,bool shadowSW)
{
	if (SW)
	{
		if (shadowSW == false)
		{
			BindShader(lightShader);
			SetShaderLight(*light);
			modelMatrix.ToIdentity();
			textureMatrix.ToIdentity();
			viewMatrix = camera->BuildViewMatrix();
			projMatrix = defaultprojMatrix;
			UpdateShaderMatrices();
		}
		else
		{
			BindShader(heightmapshadowShader);
			SetShaderLight(*light2);
			glUniform1i(glGetUniformLocation(GetCurrentShader()->GetProgram(),"shadowTex"), 2);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, shadowTex);
			viewMatrix = camera->BuildViewMatrix();
			projMatrix = defaultprojMatrix;
		}
	}
	/*SetShaderLight(*light2);*/
	glUniform3fv(glGetUniformLocation(GetCurrentShader()->GetProgram(),"cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(GetCurrentShader()->GetProgram(),"diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);

	glUniform1i(glGetUniformLocation(GetCurrentShader()->GetProgram(),"bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthBump);

	modelMatrix.ToIdentity();
	UpdateShaderMatrices();


	heightMap->Draw();
}
void Renderer::DrawHeightmapNoLight()
{
	BindShader(heightmapNolightShader);

	glUniform1i(glGetUniformLocation(heightmapNolightShader->GetProgram(),
		"diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);

	glUniform1i(glGetUniformLocation(heightmapNolightShader->GetProgram(),
		"bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthBump);

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = defaultprojMatrix;

	UpdateShaderMatrices();

	heightMap->Draw();
}
void Renderer::DrawWater(Camera* camera,bool SW,bool shadowSW)
{
	if (SW)
	{
		if (shadowSW == false)
		{
			BindShader(waterShader);
			SetShaderLight(*light);
			Vector3 hSize = heightMap->GetHeightmapSize();

			modelMatrix = Matrix4::Translation(hSize * 0.5f) *
				Matrix4::Scale(hSize * 6.0f) *
				Matrix4::Rotation(90, Vector3(1, 0, 0));

			textureMatrix = Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) *
				Matrix4::Scale(Vector3(10, 10, 10)) *
				Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));

			projMatrix = defaultprojMatrix;
			viewMatrix = camera->BuildViewMatrix();
			UpdateShaderMatrices();
		}

		else
		{
			BindShader(watershadowShader);
			SetShaderLight(*light2);
			Vector3 hSize = heightMap->GetHeightmapSize();

			modelMatrix = Matrix4::Translation(hSize * 0.5f) *
				Matrix4::Scale(hSize * 6.0f) *
				Matrix4::Rotation(90, Vector3(1, 0, 0));

			textureMatrix = Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) *
				Matrix4::Scale(Vector3(10, 10, 10)) *
				Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));

			projMatrix = defaultprojMatrix;
			viewMatrix = camera->BuildViewMatrix();
			UpdateShaderMatrices();
		}
	}
	glUniform3fv(glGetUniformLocation(GetCurrentShader()->GetProgram(),
		"cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(GetCurrentShader()->GetProgram(),
		"diffuseTex"), 0);

	glUniform1i(glGetUniformLocation(GetCurrentShader()->GetProgram(),
		"bumpTex"), 1);

	glUniform1i(glGetUniformLocation(GetCurrentShader()->GetProgram(),
		"cubeTex"), 2);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, waterBump);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	Vector3 hSize = heightMap->GetHeightmapSize();
	modelMatrix = Matrix4::Translation(hSize * 0.5f) *
		Matrix4::Scale(hSize * 6.0f) *
		Matrix4::Rotation(90, Vector3(1, 0, 0));
	UpdateShaderMatrices();

	skyboxQuad->Draw();
}






//different scenes renderer
void Renderer::RenderScene()
{
	BuildNodeLists(root);
	SortNodeLists();

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glViewport(0, 0, width, height);
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float)width / (float)height, 45.0f);
	DrawSkybox();
	DrawHeightmap(camera,true,false);
	DrawNodes(camera,true,false);
	DrawWater(camera,true,false);
	//DrawEmitter();

	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0.75 * width, 0.66 * height, (width / height) * width / 3, (width / height) * height / 3);
	DrawHeightmap(skycamera,true,false);
	DrawNodes(skycamera,true,false);
	DrawWater(skycamera,true,false);

	ClearNodeLists();
}
void Renderer::RenderSceneNight()    
{
	BuildNodeLists(root);
	SortNodeLists();

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glViewport(0, 0, width, height);
	DrawScene();
	DrawPointLights();
	CombineBuffers();

	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0.75 * width, 0.66 * height, (width / height) * width / 3, (width / height) * height / 3);
	DrawHeightmap(skycamera,true,false);
	DrawNodes(skycamera,true,false);
	DrawWater(skycamera,true,false);

	ClearNodeLists();
}
void Renderer::RenderSceneBlur()
{
	BuildNodeLists(root);
	SortNodeLists();

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, width, height);
	DrawBlurScene();
	DrawPostProcess();
	PresentScene();

	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0.75 * width, 0.66 * height, (width / height) * width / 3, (width / height) * height / 3);
	DrawHeightmap(skycamera,true,false);
	DrawNodes(skycamera,true,false);
	DrawWater(skycamera,true,false);

	ClearNodeLists();
}
void Renderer::RenderSceneShadow()
{
	BuildNodeLists(root);
	SortNodeLists();

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, width, height);
	DrawSkybox();
	DrawShadowMapping();
	DrawCombinedScene();

	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0.75 * width, 0.66 * height, (width / height) * width / 3, (width / height) * height / 3);
	DrawHeightmap(skycamera,true,false);
	DrawNodes(skycamera,true,false);
	DrawWater(skycamera,true,false);

	ClearNodeLists();
}





//post processing rendering
void Renderer::DrawBlurScene()
{
	glBindFramebuffer(GL_FRAMEBUFFER, blurbufferFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	DrawSkybox();
	DrawHeightmap(camera,true,false);
	DrawNodes(camera,true,false);
	DrawWater(camera,true,false);



	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void Renderer::DrawPostProcess()
{
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, blurbufferColourTex[1], 0);

	BindShader(processShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	textureMatrix.ToIdentity();
	UpdateShaderMatrices();

	glDisable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(processShader->GetProgram(), "sceneTex"), 0);
	for (int i = 0; i < POST_PASSES; ++i)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, blurbufferColourTex[1], 0);
		glUniform1i(glGetUniformLocation(processShader->GetProgram(), "isVertical"), 0);

		glBindTexture(GL_TEXTURE_2D, blurbufferColourTex[0]);
		skyboxQuad->Draw();

		//now to swap colour buffers,and do the second blur pass
		glUniform1i(glGetUniformLocation(processShader->GetProgram(), "isVertical"), 1);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, blurbufferColourTex[0], 0);

		glBindTexture(GL_TEXTURE_2D, blurbufferColourTex[1]);
		skyboxQuad->Draw();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
}
void Renderer::PresentScene()
{
	BindShader(blurShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, blurbufferColourTex[0]);
	glUniform1i(glGetUniformLocation(blurShader->GetProgram(), "diffuseTex"), 0);
	skyboxQuad->Draw();
}






//deferred rendering
void Renderer::DrawScene()
{
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float)width / (float)height, 45.0f);

	DrawSkybox();
	DrawHeightmapNoLight();
	DrawNodes(camera,true,false);
	DrawWater(camera,true,false);


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void Renderer::DrawPointLights()
{
	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);
	BindShader(pointlightShader);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glBlendFunc(GL_ONE, GL_ONE);
	glCullFace(GL_FRONT);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);

	glUniform1i(glGetUniformLocation(pointlightShader->GetProgram(), "depthTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);

	glUniform1i(glGetUniformLocation(pointlightShader->GetProgram(), "normTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bufferNormalTex);

	glUniform3fv(glGetUniformLocation(pointlightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform2f(glGetUniformLocation(pointlightShader->GetProgram(), "pixelSize"), 1.0f / width, 1.0f / height);

	Matrix4 invViewProj = (projMatrix * viewMatrix).Inverse();
	glUniformMatrix4fv(glGetUniformLocation(pointlightShader->GetProgram(), "inverseProjView"), 1, false, invViewProj.values);
	UpdateShaderMatrices();
	for (int i = 0; i < 40; ++i) {
		Light& l = pointLights[i];
		SetShaderLight(l);
		sphere->Draw();
	}
	sphere->Draw();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);

	glDepthMask(GL_TRUE);

	glClearColor(0.2f, 0.2f, 0.2f, 1);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void Renderer::CombineBuffers()
{
	BindShader(combineShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "diffuseLight"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, lightDiffuseTex);

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "specularLight"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, lightSpecularTex);

	skyboxQuad->Draw();
}





//shaodw rendering
void Renderer::DrawNodesShadow()
{
	for (const auto& i : nodeList)
	{
		DrawNodeShadow(i);
	}
	for (const auto& i : transparentNodeList)
	{
		DrawNodeShadow(i);
	}
}
void Renderer::DrawNodeShadow(SceneNode* n) 
{
	if (n->GetMesh())
	{
		if (n != rain)
		{
			if (n != soldierNode || n != femaleNode)
			{

				BindShader(shadowShader);
				Vector3 hSize = heightMap->GetHeightmapSize();
				viewMatrix = Matrix4::BuildViewMatrix(light->GetPosition(), Vector3(hSize.x, 0.0f, hSize.z));
				projMatrix = Matrix4::Orthographic(20.0f, 2000000.0f, -10000.0f, 10000.0f, -10000.0f, 10000.0f);
				shadowMatrix = projMatrix * viewMatrix;
				Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
				modelMatrix = model;
				UpdateShaderMatrices();
				n->GetMesh()->Draw();
			}
			else
			{
				BindShader(soldiershadowShader);
				Vector3 hSize = heightMap->GetHeightmapSize();
				viewMatrix = Matrix4::BuildViewMatrix(light->GetPosition(), Vector3(hSize.x, 0.0f, hSize.z));
				projMatrix = Matrix4::Orthographic(20.0f, 2000000.0f, -10000.0f, 10000.0f, -10000.0f, 10000.0f);
				shadowMatrix = projMatrix * viewMatrix;
				Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
				modelMatrix = model;
				UpdateShaderMatrices();
				vector<Matrix4> frameMatrices;

				const Matrix4* invBindPose = n->GetMesh()->GetInverseBindPose();
				const Matrix4* frameData = n->GetAnimation()->GetJointData(n->GetCurrentFrame());

				for (unsigned int i = 0; i < n->GetMesh()->GetJointCount(); ++i)
				{
					frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
				}

				int j = glGetUniformLocation(GetCurrentShader()->GetProgram(), "joints");
				glUniformMatrix4fv(j, frameMatrices.size(), false,
					(float*)frameMatrices.data());

				for (int i = 0; i < n->GetMesh()->GetSubMeshCount(); ++i)
				{
					n->GetMesh()->DrawSubMesh(i);
				}
			}
		}
	}
}
void Renderer::DrawShadowMapping()
{
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);



	BindShader(shadowShader);
	Vector3 hSize = heightMap->GetHeightmapSize();
	viewMatrix = Matrix4::BuildViewMatrix(light->GetPosition(), Vector3(hSize.x, 0.0f, hSize.z));
	projMatrix = Matrix4::Orthographic(20.0f, 2000000.0f, -10000.0f, 10000.0f, -10000.0f, 10000.0f);
	shadowMatrix = projMatrix * viewMatrix;

	modelMatrix.ToIdentity();
	UpdateShaderMatrices();
	heightMap->Draw();

	modelMatrix = rockmodelMatrix;
	UpdateShaderMatrices();
	rock->Draw();

	modelMatrix = Matrix4::Translation(hSize * 0.5f) * Matrix4::Scale(hSize * 6.0f) * Matrix4::Rotation(90, Vector3(1, 0, 0));
	UpdateShaderMatrices();
	skyboxQuad->Draw();

	DrawNodesShadow();


	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glDisable(GL_CULL_FACE);
}
void Renderer::DrawCombinedScene()
{
	BindShader(bumpshadowShader);
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = defaultprojMatrix;
	SetShaderLight(*light2);
	UpdateShaderMatrices();
	DrawHeightmap(camera,false, false);

	Vector3 hSize = heightMap->GetHeightmapSize();
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = defaultprojMatrix;
	glUniform1i(glGetUniformLocation(bumpshadowShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(bumpshadowShader->GetProgram(), "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(bumpshadowShader->GetProgram(), "shadowTex"), 2);
	glUniform3fv(glGetUniformLocation(bumpshadowShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, rockTex);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, rockBump);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	modelMatrix = rockmodelMatrix;
	UpdateShaderMatrices();
	rock->Draw();

	DrawNodes(camera,true, true);

}






//other functions
void Renderer::ClearNodeLists()
{
	transparentNodeList.clear();
	nodeList.clear();
}
void Renderer::SetShaderParticleSize(float f) 
{              
	glUniform1f(glGetUniformLocation(emitterShader->GetProgram(), "particleSize"), f);
}
void Renderer::GenerateScreenTexture(GLuint& into, bool depth)
{
	glGenTextures(1, &into);
	glBindTexture(GL_TEXTURE_2D, into);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	GLuint format = depth ? GL_DEPTH_COMPONENT24 : GL_RGBA8;
	GLuint type = depth ? GL_DEPTH_COMPONENT : GL_RGBA;

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, type, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
}

