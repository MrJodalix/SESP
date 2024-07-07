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
};
typedef struct RenderingData RenderingData;

float interval = 0.0f;
float pauseInterval = 0.0f;

unsigned int quadVAO = 0;
unsigned int quadVBO;

GBuffer* gBuffer;

// [0] = width, [1] = height
int lastScreenSize[2];

////////////////////////////// LOKALE FUNKTIONEN ///////////////////////////////


/**
* Rendert ein Quadrat in Groesse des Bildschirms
*/
static void renderQuad()
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
	model_drawModel(input->rendering.userScene->model, data->modelShader, true);
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
static void rendering_renderDirLight(RenderingData* data, InputData* input, vec2* screenSize) {

	shader_useShader(data->dirLightShader);

	// Lichtfarbe
	vec3 lightColor;
	glm_vec4_copy3(input->rendering.lightColor, lightColor);

	// Lichtquellenposition
	vec3 lightPosVec = { input->rendering.lightDir[0],input->rendering.lightDir[1],input->rendering.lightDir[2] };

	// Licht ambient
	float matAmbient = { input->rendering.lightComp[0] };

	// Licht specular
	float matSpecular = { input->rendering.lightComp[1] };

	// Licht diffuse
	float matDiffuse = { input->rendering.lightComp[2] };

	vec3 viewPos = { 0.0, 0.0, 0.0 };
	camera_getPosition(input->mainCamera, viewPos);

	shader_setInt(data->dirLightShader, "u_Position", 0);
	shader_setInt(data->dirLightShader, "u_Normal", 1);
	shader_setInt(data->dirLightShader, "u_AlbedoSpec", 2);
	shader_setInt(data->dirLightShader, "u_Emission", 3);

	shader_setVec2(data->dirLightShader, "u_screenSize", screenSize);

	shader_setVec3(data->dirLightShader, "u_lightPosVec", &lightPosVec);
	shader_setVec3(data->dirLightShader, "u_lightColor", &lightColor);
	shader_setFloat(data->dirLightShader, "u_matAmbient", matAmbient);
	shader_setFloat(data->dirLightShader, "u_matSpecular", matSpecular);
	shader_setFloat(data->dirLightShader, "u_matDiffuse", matDiffuse);

	shader_setVec3(data->dirLightShader, "u_viewPos", &viewPos);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	// dirLight zeichnen
	common_pushRenderScope("Scene dirLight");
	renderQuad();
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

	shader_useShader(data->pointLightShader);

	// Lichtfarbe
	vec3 lightColor;
	glm_vec4_copy3(input->rendering.lightColor, lightColor);
	lightColor[0] = 1.0;
	lightColor[1] = 0.0;
	lightColor[2] = 0.0;

	// Licht ambient
	float matAmbient = { input->rendering.lightComp[0] };

	// Licht specular
	float matSpecular = { input->rendering.lightComp[1] };

	// Licht diffuse
	float matDiffuse = { input->rendering.lightComp[2] };

	vec3 viewPos = { 0.0, 0.0, 0.0 };
	camera_getPosition(input->mainCamera, viewPos);

	unsigned int countPointLights = input->rendering.userScene->countPointLights;

	for (unsigned int i = 0; i < countPointLights; i++) {
		PointLight *light = input->rendering.userScene->pointLights[i];
		shader_setVec3(data->pointLightShader, "u_Position", &light->position);
	}

	//shader_setInt(data->pointLightShader, "u_Position", GBUFFER_COLORATTACH_POSITION);
	shader_setInt(data->pointLightShader, "u_Normal", GBUFFER_COLORATTACH_NORMAL);
	shader_setInt(data->pointLightShader, "u_AlbedoSpec", GBUFFER_COLORATTACH_ALBEDOSPEC);
	shader_setInt(data->pointLightShader, "u_Emission", GBUFFER_COLORATTACH_EMISSION);

	shader_setVec2(data->pointLightShader, "u_screenSize", screenSize);

	shader_setVec3(data->pointLightShader, "u_lightColor", &lightColor);
	shader_setFloat(data->pointLightShader, "u_matAmbient", matAmbient);
	shader_setFloat(data->pointLightShader, "u_matSpecular", matSpecular);
	shader_setFloat(data->pointLightShader, "u_matDiffuse", matDiffuse);

	shader_setVec3(data->pointLightShader, "u_viewPos", &viewPos);

	// Shader projection Matrix uebergeben
	shader_setMat4(data->pointLightShader, "u_projectionMatrix", projectionMatrix);
	// Shader model Matrix uebergeben
	shader_setMat4(data->pointLightShader, "u_modelMatrix", modelMatrix);
	// Shader view Matrix uebergeben
	shader_setMat4(data->pointLightShader, "u_viewMatrix", viewMatrix);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	// dirLight zeichnen
	common_pushRenderScope("Scene pointLight");
	renderQuad();
	common_popRenderScope();
	glDisable(GL_BLEND);
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
	renderQuad();
	common_popRenderScope();
}

/**
 * erstellt die Projektions Matrix
 *
 * @param ctx der Programm Context
 * @param data die daten des Programms
 * @param input der Gui Input
 * @param projectionMatrix die Projektionsmatrix
 */
void setProjectionMatrix(ProgContext* ctx, InputData* input, mat4* projectionMatrix)
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
void setModelMatrix(InputData* input, mat4* modelMatrix)
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
}

void rendering_draw(ProgContext* ctx)
{
	RenderingData* data = ctx->rendering;
	InputData* input = ctx->input;
	vec2 screenSize = { (float)ctx->winData->realWidth, (float)ctx->winData->realHeight };

	// Shader vorbereiten.
	if (input->reloadShader == true)
	{
		shader_deleteShader(data->modelShader);
		shader_deleteShader(data->dirLightShader);
		shader_deleteShader(data->nullShader);
		shader_deleteShader(data->postProcessShader);
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
		// Zuerst die Projection Matrix aufsetzen.
		mat4 projectionMatrix;
		setProjectionMatrix(ctx, input, &projectionMatrix);

		// Dann die View-Matrix bestimmen.
		mat4 viewMatrix;
		camera_getViewMatrix(input->mainCamera, viewMatrix);

		// ModelMatrix festlegen.
		mat4 modelMatrix;
		setModelMatrix(input, &modelMatrix);

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

		//STencikl

		glDisable(GL_CULL_FACE);
		gbuffer_bindGBufferForLightPass(gBuffer);
		glEnable(GL_CULL_FACE); 
		glCullFace(GL_BACK);
		glDisable(GL_BLEND);

		if (data->pointLightShader != NULL)
		{
			rendering_renderPointLight(data, input, &screenSize, &projectionMatrix, &viewMatrix, &modelMatrix);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		gbuffer_bindGBufferForLightPass(gBuffer);

		if (data->dirLightShader != NULL)
		{
			rendering_renderDirLight(data, input, &screenSize);
		}

		// Tiefentest nach der 3D Szene wieder deaktivieren.
		glDisable(GL_DEPTH_TEST);

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
	shader_deleteShader(data->modelShader);
	shader_deleteShader(data->dirLightShader);
	shader_deleteShader(data->nullShader);
	shader_deleteShader(data->postProcessShader);

	gbuffer_deleteGBuffer(gBuffer);

	free(ctx->rendering);
}
