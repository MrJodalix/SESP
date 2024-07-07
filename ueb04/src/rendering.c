/**
 * Modul zum Rendern der 3D Szene.
 *
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

#include "rendering.h"

#include <string.h>

#include "shader.h"
#include "model.h"
#include "utils.h"
#include "input.h"
#include "camera.h"
#include "gbuffer.h"

 ////////////////////////////// LOKALE DATENTYPEN ///////////////////////////////

 // Datentyp für alle persistenten Daten des Renderers.
struct RenderingData {
	Shader* modelShader;
	Shader* dirLightShader;
	Shader* pointLightShader;
	Shader* nullShader;
	Shader* postProcessShader;
	Shader* dirShadowShader;
	Shader* pointShadowShader;
};
typedef struct RenderingData RenderingData;

float interval = 0.0f;
float pauseInterval = 0.0f;

unsigned int quadVAO = 0;
unsigned int quadVBO;

unsigned int sphereVAO = 0;
unsigned int sphereVBO;

unsigned int planeVAO;

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;

GBuffer* gBuffer;

// [0] = width, [1] = height
int lastScreenSize[2];

const unsigned int DIR_SHADOW_SIZE = 1024;

// create a framebuffer object for rendering the depthMap
unsigned int depthMapFBO;
// 2D texture to be used as the framebuffer's depth buffer
unsigned int depthMap;

////////////////////////////// LOKALE FUNKTIONEN ///////////////////////////////


/**
* Rendert ein Quadrat in Groesse des Bildschirms
*/
static void rendering_renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

/**
* Loescht alle Inhalte der Shader
*/
void rendering_deleteAllShader(RenderingData* data)
{
	shader_deleteShader(data->modelShader);
	shader_deleteShader(data->dirLightShader);
	shader_deleteShader(data->pointLightShader);
	shader_deleteShader(data->nullShader);
	shader_deleteShader(data->postProcessShader);
	shader_deleteShader(data->dirShadowShader);
	shader_deleteShader(data->pointShadowShader);
}

/**
 * Lädt alle Shader, die im Rendering-Modul verwendet werden.
 *
 * @param data Zugriff auf das Rendering Datenobjekt.
 */
static void rendering_loadShaders(RenderingData* data)
{
	data->modelShader = shader_createVeTessFrShader("Model",
		UTILS_CONST_RES("shader/model/model.vert"),
		UTILS_CONST_RES("shader/model/model.tesc"),
		UTILS_CONST_RES("shader/model/model.tese"),
		UTILS_CONST_RES("shader/model/model.frag")
	);
	data->dirLightShader = shader_createVeFrShader("DirLight",
		UTILS_CONST_RES("shader/dirLight/dirLight.vert"),
		UTILS_CONST_RES("shader/dirLight/dirLight.frag")
	);
	data->pointLightShader = shader_createVeFrShader("PointLight",
		UTILS_CONST_RES("shader/pointLight/pointLight.vert"),
		UTILS_CONST_RES("shader/pointLight/pointLight.frag")
	);
	data->nullShader = shader_createVeFrShader("Null",
		UTILS_CONST_RES("shader/null/null.vert"),
		UTILS_CONST_RES("shader/null/null.frag")
	);
	data->postProcessShader = shader_createVeFrShader("PostProcess",
		UTILS_CONST_RES("shader/postProcess/postProcess.vert"),
		UTILS_CONST_RES("shader/postProcess/postProcess.frag")
	);
	data->dirShadowShader = shader_createVeFrShader("DirShadow",
		UTILS_CONST_RES("shader/dirShadow/dirShadow.vert"),
		UTILS_CONST_RES("shader/dirShadow/dirShadow.frag")
	);
	data->pointShadowShader = shader_createVeFrShader("PointShadow",
		UTILS_CONST_RES("shader/pointShadow/pointShadow.vert"),
		UTILS_CONST_RES("shader/pointShadow/pointShadow.frag")
	);
}

/**
* Uebergibt die Daten an den Model Shader
*
* @param data die zu renderden Daten
* @param input gui Input
* @param projectionMatrix die Projektions Matrix
* @param viewMatrix die View Matrix
* @param modelMatrix die Model Matrix
*/
static void rendering_renderModel(RenderingData* data, InputData* input, mat4* projectionMatrix, mat4* viewMatrix, mat4* modelMatrix)
{
	shader_useShader(data->modelShader);

	// Shader projection Matrix uebergeben
	shader_setMat4(data->modelShader, "u_projectionMatrix", projectionMatrix);
	// Shader model Matrix uebergeben
	shader_setMat4(data->modelShader, "u_modelMatrix", modelMatrix);
	// Shader view Matrix uebergeben
	shader_setMat4(data->modelShader, "u_viewMatrix", viewMatrix);

	vec3 fogColor;
	//glm_vec4_copy3(input->rendering.fogColor, fogColor);
	shader_setVec3(data->modelShader, "u_fogColor", &fogColor);

	shader_setBool(data->modelShader, "u_useFog", input->showFog);

	shader_setFloat(data->modelShader, "u_density", input->density);

	shader_setInt(data->modelShader, "u_shaderChoice", input->shaderChoice);

	shader_setBool(data->modelShader, "u_showNormalMap", input->showNormalMap);

	//Tesselation
	shader_setBool(data->modelShader, "u_useTess", input->showTess);
	shader_setFloat(data->modelShader, "u_TessLevelInner", input->rendering.tessInner);
	shader_setFloat(data->modelShader, "u_TessLevelOuter", input->rendering.tessOuter);

	// Modell zeichnen
	common_pushRenderScope("Scene Model");
	model_drawModel(input->rendering.userScene->model, data->modelShader, input->showTess);
	common_popRenderScope();

	// Wireframe vorm Rendern des Frames wieder deaktivieren.
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

/**
* Uebergibt die Daten an den Direct Light Shader
*
* @param data die zu renderden Daten
* @param input gui Input
* @param modelMatrix die Model Matrix
* @param screenSize Die Fenstergroesze
*/
static void rendering_renderDirLight(RenderingData* data, InputData* input, vec2* screenSize, mat4* modelMatrix, mat4* lightSpaceMatrix) {

	shader_useShader(data->dirLightShader);

	// Lichtfarbe
	vec3 lightColor;
	glm_vec4_copy3(input->rendering.lightColor, lightColor);

	// Lichtquellenposition
	vec3 u_lightDirVec = { input->rendering.lightDir[0],input->rendering.lightDir[1],input->rendering.lightDir[2] };

	// Licht ambient
	float matAmbient = { input->rendering.lightComp[0] };

	// Licht specular
	float matSpecular = { input->rendering.lightComp[1] };

	// Licht diffuse
	float matDiffuse = { input->rendering.lightComp[2] };

	vec3 viewPos = { 0.0, 0.0, 0.0 };
	camera_getPosition(input->mainCamera, viewPos);

	shader_setInt(data->dirLightShader, "u_Position", GBUFFER_COLORATTACH_POSITION);
	shader_setInt(data->dirLightShader, "u_Normal", GBUFFER_COLORATTACH_NORMAL);
	shader_setInt(data->dirLightShader, "u_AlbedoSpec", GBUFFER_COLORATTACH_ALBEDOSPEC);
	shader_setInt(data->dirLightShader, "u_Emission", GBUFFER_COLORATTACH_EMISSION);

	shader_setInt(data->dirLightShader, "u_shadowMap", 6);

	shader_setVec2(data->dirLightShader, "u_screenSize", screenSize);

	shader_setVec3(data->dirLightShader, "u_lightDirVec", &u_lightDirVec);
	shader_setVec3(data->dirLightShader, "u_lightColor", &lightColor);
	shader_setFloat(data->dirLightShader, "u_matAmbient", matAmbient);
	shader_setFloat(data->dirLightShader, "u_matSpecular", matSpecular);
	shader_setFloat(data->dirLightShader, "u_matDiffuse", matDiffuse);

	shader_setVec3(data->dirLightShader, "u_viewPos", &viewPos);

	shader_setMat4(data->dirLightShader, "u_modelMatrix", modelMatrix);
	shader_setMat4(data->dirLightShader, "u_lightSpaceMatrix", lightSpaceMatrix);


	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	// dirLight zeichnen
	common_pushRenderScope("Scene dirLight");
	rendering_renderQuad();
	common_popRenderScope();

	glDisable(GL_BLEND);
}

/**
* Uebergibt die Daten an den Direct Light Shader
*
* @param data die zu renderden Daten
* @param input gui Input
* @param modelMatrix die Model Matrix
* @param screenSize Die Fenstergroesze
*/
static void rendering_renderPointLight(RenderingData* data, InputData* input, vec2* screenSize, mat4* projectionMatrix, mat4* viewMatrix, mat4* modelMatrix) {

	// Lichtfarbe
	vec3 lightColor;
	glm_vec4_copy3(input->rendering.lightColor, lightColor);

	// Licht ambient
	float matAmbient = { input->rendering.lightComp[0] };

	// Licht specular
	float matSpecular = { input->rendering.lightComp[1] };

	// Licht diffuse
	float matDiffuse = { input->rendering.lightComp[2] };

	vec3 viewPos = { 0.0, 0.0, 0.0 };
	camera_getPosition(input->mainCamera, viewPos);

	shader_useShader(data->pointLightShader);

	// Shader projection Matrix uebergeben
	shader_setMat4(data->pointLightShader, "u_projectionMatrix", projectionMatrix);
	// Shader model Matrix uebergeben
	shader_setMat4(data->pointLightShader, "u_modelMatrix", modelMatrix);
	// Shader view Matrix uebergeben
	shader_setMat4(data->pointLightShader, "u_viewMatrix", viewMatrix);

	shader_setVec3(data->pointLightShader, "u_viewPos", &viewPos);

	shader_setInt(data->pointLightShader, "u_Position", GBUFFER_COLORATTACH_POSITION);
	shader_setInt(data->pointLightShader, "u_Normal", GBUFFER_COLORATTACH_NORMAL);
	shader_setInt(data->pointLightShader, "u_AlbedoSpec", GBUFFER_COLORATTACH_ALBEDOSPEC);
	shader_setInt(data->pointLightShader, "u_Emission", GBUFFER_COLORATTACH_EMISSION);

	shader_setInt(data->pointLightShader, "u_shadowMap", 4);

	shader_setVec2(data->pointLightShader, "u_screenSize", screenSize);

	shader_setFloat(data->pointLightShader, "u_matAmbient", matAmbient);
	shader_setFloat(data->pointLightShader, "u_matSpecular", matSpecular);
	shader_setFloat(data->pointLightShader, "u_matDiffuse", matDiffuse);

	unsigned int countPointLights = input->rendering.userScene->countPointLights;

	PointLight* light = input->rendering.userScene->pointLights[0];

	for (unsigned int i = 0; i < countPointLights; i++) {
		light = input->rendering.userScene->pointLights[i];

		glEnable(GL_BLEND); // Enable Blending
		glBlendEquation(GL_FUNC_ADD); // GPU will simply add the source and the destination
		glBlendFunc(GL_ONE, GL_ONE); // true addition

		light_activatePointLight(light, data->pointLightShader);

		// pointLight zeichnen
		common_pushRenderScope("Scene pointLight");
		rendering_renderQuad();
		common_popRenderScope();
		glDisable(GL_BLEND);
	}
}

/**
* Uebergibt die Daten an den postProcess Shader
*
* @param der Programm Context
*/
static void rendering_renderPostProcess(RenderingData* data, InputData* input, vec2* screenSize)
{
	shader_useShader(data->postProcessShader);

	shader_setVec2(data->postProcessShader, "u_screenSize", screenSize);

	shader_setFloat(data->postProcessShader, "u_exposure", input->rendering.exposure);
	shader_setFloat(data->postProcessShader, "u_gamma", input->rendering.gamma);
	shader_setInt(data->postProcessShader, "u_final", GBUFFER_COLORATTACH_FINAL);

	// postProcess zeichnen
	common_pushRenderScope("Scene PostProcess");
	rendering_renderQuad();
	common_popRenderScope();
}


/**
* rendert einen Wuerfel
*/
void rendering_renderCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}


/**
* rendert die Scene
* @param data die Daten um auf den Shader zugreifen zu können
*/
void rendering_renderScene(InputData* input, Shader* shader)
{
	input->rendering.userScene;
	model_drawModel(input->rendering.userScene->model, shader, false);
}

/**
 * erstellt die Projektions Matrix
 *
 * @param ctx der Programm Context
 * @param data die Daten des Programms
 * @param input der Gui Input
 * @param projectionMatrix die Projektionsmatrix
 */
void rendering_setProjectionMatrix(ProgContext* ctx, InputData* input, mat4* projectionMatrix)
{
	float aspect = (float)ctx->winData->width / (float)ctx->winData->height;
	float zoom = camera_getZoom(input->mainCamera);
	glm_perspective(glm_rad(zoom), aspect, 0.1f, 200.0f, *projectionMatrix);
}

/*
 * Erstellt die Model Matrix und übernimmt die Skalierung, Rotation und Translation
 *
 * @param input der Gui Input
 * @param modelMatrix die Matrix
 */
void rendering_setModelMatrix(InputData* input, mat4* modelMatrix)
{
	glm_mat4_identity(*modelMatrix);

	// Beispielhafte Veränderungen an der Modelmatrix.
	// Mehr Informationen auf: https://cglm.readthedocs.io/en/latest/affine.html

	glm_scale_make(*modelMatrix, input->rendering.scale);

	glm_rotate_x(*modelMatrix, glm_rad(input->rendering.rotate[0]), *modelMatrix);
	glm_rotate_y(*modelMatrix, glm_rad(input->rendering.rotate[1]), *modelMatrix);
	glm_rotate_z(*modelMatrix, glm_rad(input->rendering.rotate[2]), *modelMatrix);

	glm_translate(*modelMatrix, input->rendering.translate);
}

/*
 * Hilfsfunktion zum Initialisieren der Schatten-Einstellungen.
 *
 * @param data die Daten des Programms
 */
static void rendering_shadowInit(RenderingData* data)
{
	glGenFramebuffers(1, &depthMapFBO);

	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	common_labelObjectByType(GL_TEXTURE, depthMap, "Shadow Depth");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, DIR_SHADOW_SIZE, DIR_SHADOW_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// attach depth texture as FBO
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE); // we don't need a color buffer! explicitly tell OpenGL that
	glReadBuffer(GL_NONE); // we don't need a color buffer! explicitly tell OpenGL that
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shader_useShader(data->dirShadowShader);
	shader_setInt(data->dirShadowShader, "depthMap", 0);

	//shader_useShader(data->pointShadowShader);
	//shader_setInt(data->pointShadowShader, "depthMap", 0);
}

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

void rendering_init(ProgContext* ctx)
{
	ctx->rendering = malloc(sizeof(RenderingData));
	RenderingData* data = ctx->rendering;

	// Speicher bereinigen.
	memset(data, 0, sizeof(RenderingData));

	// OpenGL Flags setzen.
	glCullFace(GL_BACK);        // setzten Faceculling auf Back-Face
	glFrontFace(GL_CCW);        // front Faces sind gegen den Uhrzeigersinn

	glEnable(GL_DEPTH_TEST);

	// Alle Shader laden.
	rendering_loadShaders(data);

	// Bildschirm leeren.
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	lastScreenSize[0] = ctx->winData->realWidth;
	lastScreenSize[1] = ctx->winData->realHeight;

	// inform GBuffer about the start of new Frames
	gBuffer = gbuffer_createGBuffer(ctx->winData->realWidth, ctx->winData->realHeight);
	// Setup cube VAO	
	float planeVertices[] = {
		// positions            // normals         // texcoords
	    -25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f,
		-25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,

		-25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f,
		25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,

		25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 25.0f
    };
	GLuint planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);
	rendering_shadowInit(data);
}

void rendering_draw(ProgContext* ctx)
{
	RenderingData* data = ctx->rendering;
	InputData* input = ctx->input;
	vec2 screenSize = { (float)ctx->winData->realWidth, (float)ctx->winData->realHeight };

	// Shader vorbereiten.
	if (input->reloadShader == true)
	{
		rendering_deleteAllShader(data);
		rendering_loadShaders(data);
		input->reloadShader = false;
	}

	if ((lastScreenSize[0] != ctx->winData->realWidth) || (lastScreenSize[1] != ctx->winData->realHeight))
	{
		gbuffer_deleteGBuffer(gBuffer);
		gBuffer = gbuffer_createGBuffer(ctx->winData->realWidth, ctx->winData->realHeight);
		lastScreenSize[0] = ctx->winData->realWidth;
		lastScreenSize[1] = ctx->winData->realHeight;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);




	// Überprüfen, ob der Wireframe Modus verwendet werden soll.
	if (input->showWireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_CULL_FACE); // deaktivierung des Face Cullings
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_CULL_FACE);  // aktivierung des Face Cullings
	}

	if (input->rendering.userScene)
	{
		// 1. first render to depth map
		mat4 lightProjection;
		mat4 lightView;
		mat4 lightSpaceMatrix;

		// Zuerst die Projection Matrix aufsetzen.
		mat4 projectionMatrix;
		rendering_setProjectionMatrix(ctx, input, &projectionMatrix);

		// Dann die View-Matrix bestimmen.
		mat4 viewMatrix;
		camera_getViewMatrix(input->mainCamera, viewMatrix);

		// ModelMatrix festlegen.
		mat4 modelMatrix;
		rendering_setModelMatrix(input, &modelMatrix);

		gbuffer_clearFinalTexture(gBuffer);

		// only geometry pass updates the depth buffer
		// enable writing into depth buffer befor clearing it!
		glDepthMask(GL_TRUE);

		//setting GBuffer object for writing
		gbuffer_bindGBufferForGeomPass(gBuffer);
		glEnable(GL_DEPTH_TEST);

		// prevent anything but this pass from writing into the depth buffer
		// needs the depth buffer in order to populate the G-Buffer with closest pixels
		// in lightpass we have a single texel per screen pixel so we don't have anything to write into the depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the current FBO (G buffer)

		// Das Nutzermodell nur dann Rendern, wenn es existiert.

		if (data->modelShader != NULL)
		{
			rendering_renderModel(data, input, &projectionMatrix, &viewMatrix, &modelMatrix);
		}

		glDepthMask(GL_FALSE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		gbuffer_bindGBufferForLightPass(gBuffer);

		if (data->pointLightShader != NULL)
		{
			rendering_renderPointLight(data, input, &screenSize, &projectionMatrix, &viewMatrix, &modelMatrix);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glDepthMask(GL_TRUE);

		if (data->dirShadowShader != NULL && input->showShadow)
		{
			glm_mat4_identity(lightProjection);
			glm_mat4_identity(lightView);
			glm_mat4_identity(lightSpaceMatrix);

			float near_plane = input->nearPlane, far_plane = input->farPlane;
			glm_ortho(-input->expression, input->expression, -input->expression, input->expression, near_plane, far_plane, lightProjection);	// orthographic proj. matrix for light src, since all light rays are parallel

			vec3 center = { 0.0f };
			vec3 up = { 0.0f, 1.0f, 0.0f };
			// make sure size of the projection frustum correctly contains the objects to be render in the depth map
			//Create view Matrix

			vec3 lightPos = { 0.0f };
			glm_vec3_add(lightPos, input->rendering.lightDir, lightPos);

			glm_vec3_scale(lightPos, input->distance, lightPos);

			float isParallel = glm_vec3_dot(up, lightPos);
			
		    if(isParallel > (1 - FLT_EPSILON) && isParallel < (1 + FLT_EPSILON))
			{
				up[0] = 1.0f;
			}

			glm_lookat(lightPos, center, up, lightView);

			//light space transformation matrix (transforms each world-space vector into the space visible from the light source)
			glm_mat4_mul(lightProjection, lightView, lightSpaceMatrix);

			//rendering_shadowInit(data);
			shader_useShader(data->dirShadowShader);

			shader_setMat4(data->dirShadowShader, "u_modelMatrix", &modelMatrix);
			shader_setMat4(data->dirShadowShader, "u_lightSpaceMatrix", &lightSpaceMatrix);

			glViewport(0, 0, DIR_SHADOW_SIZE, DIR_SHADOW_SIZE);
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			common_labelObjectByType(GL_FRAMEBUFFER, depthMapFBO, "Shadow Depth");
			
			glClear(GL_DEPTH_BUFFER_BIT);

			common_pushRenderScope("Scene DirShadow");
			rendering_renderScene(input, data->dirShadowShader);
			common_popRenderScope();
		}

		glDepthMask(GL_FALSE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 2. then render scene as normal with shadow mapping (using depth map)
		//Reset view port
		glViewport(0, 0, ctx->winData->realWidth, ctx->winData->realHeight);



		gbuffer_bindGBufferForLightPass(gBuffer);

		if (data->dirLightShader != NULL)
		{
			rendering_renderDirLight(data, input, &screenSize, &modelMatrix, &lightSpaceMatrix);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		gbuffer_bindGBufferForLightPass(gBuffer);

		if (data->postProcessShader != NULL)
		{
			rendering_renderPostProcess(data, input, &screenSize);
		}

		gbuffer_bindGBufferForFinalPass(gBuffer);

		if (input->shaderChoice == 0)
		{
			glBlitFramebuffer(0, 0, ctx->winData->realWidth, ctx->winData->realHeight,
				0, 0, ctx->winData->realWidth, ctx->winData->realHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

		}
		else if (input->shaderChoice == 1)
		{
			GLint HalfWidth = (GLint)((float)(ctx->winData->realWidth / 2.0));
			GLint HalfHeight = (GLint)((float)(ctx->winData->realHeight / 2.0));

			gbuffer_bindGBufferForTextureRead(GBUFFER_COLORATTACH_POSITION);
			glBlitFramebuffer(0, 0, ctx->winData->realWidth, ctx->winData->realHeight, 0, 0,
				HalfWidth, HalfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

			gbuffer_bindGBufferForTextureRead(GBUFFER_COLORATTACH_NORMAL);
			glBlitFramebuffer(0, 0, ctx->winData->realWidth, ctx->winData->realHeight, HalfWidth, HalfHeight,
				ctx->winData->realWidth, ctx->winData->realHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

			gbuffer_bindGBufferForTextureRead(GBUFFER_COLORATTACH_EMISSION);
			glBlitFramebuffer(0, 0, ctx->winData->realWidth, ctx->winData->realHeight, 0,
				HalfHeight, HalfWidth, ctx->winData->realHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

			gbuffer_bindGBufferForTextureRead(GBUFFER_COLORATTACH_ALBEDOSPEC);
			glBlitFramebuffer(0, 0, ctx->winData->realWidth, ctx->winData->realHeight, HalfWidth, 0,
				ctx->winData->realWidth, HalfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}
	}
}

void rendering_cleanup(ProgContext* ctx)
{
	RenderingData* data = ctx->rendering;

	// Zum Schluss müssen noch die belegten Ressourcen freigegeben werden.
	rendering_deleteAllShader(data);

	gbuffer_deleteGBuffer(gBuffer);

	free(ctx->rendering);
}
