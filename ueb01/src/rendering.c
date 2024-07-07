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

////////////////////////////// LOKALE DATENTYPEN ///////////////////////////////

// Datentyp für alle persistenten Daten des Renderers.
struct RenderingData {
    Shader* modelShader;
    Shader* skyboxShader;
};
typedef struct RenderingData RenderingData;

////////////////////////////// LOKALE FUNKTIONEN ///////////////////////////////

/**
 * Lädt alle Shader, die im Rendering-Modul verwendet werden.
 * 
 * @param data Zugriff auf das Rendering Datenobjekt.
 */
static void rendering_loadShaders(RenderingData* data)
{
    data->modelShader = shader_createVeFrShader("Model",
        UTILS_CONST_RES("shader/model/model.vert"),
        UTILS_CONST_RES("shader/model/model.frag")
    );    
    data->skyboxShader = shader_createVeFrShader("Skybox",
        UTILS_CONST_RES("shader/skybox/skybox.vert"),
        UTILS_CONST_RES("shader/skybox/skybox.frag")
    );
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

    // Alle Shader laden.
    rendering_loadShaders(data);
}

void rendering_draw(ProgContext* ctx)
{
    RenderingData* data = ctx->rendering;
    InputData* input = ctx->input;

    // Shader vorbereiten.
    if (input->reloadShader == true)
    {
        shader_deleteShader(data->modelShader);
        shader_deleteShader(data->skyboxShader);
        rendering_loadShaders(data);
        input->reloadShader = false;
    }

    // Bildschirm leeren.
    glClearColor(
        input->rendering.clearColor[0],
        input->rendering.clearColor[1],
        input->rendering.clearColor[2],
        input->rendering.clearColor[3]
    );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Überprüfen, ob der Wireframe Modus verwendet werden soll.
    if(input->showWireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_CULL_FACE); // deaktivierung des Face Cullings
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_CULL_FACE);  // aktivierung des Face Cullings
    }

    // Tiefentest aktivieren.
    glEnable(GL_DEPTH_TEST);

    // Zuerst die Projection Matrix aufsetzen.
    mat4 projectionMatrix;
    float aspect = (float)ctx->winData->width / (float)ctx->winData->height;
    float zoom = camera_getZoom(input->mainCamera);
    glm_perspective(glm_rad(zoom), aspect, 0.1f, 200.0f, projectionMatrix);

    // Dann die View-Matrix bestimmen.
    mat4 viewMatrix;
    camera_getViewMatrix(input->mainCamera, viewMatrix);

    // Das Nutzermodell nur dann Rendern, wenn es existiert.
    if (input->rendering.userModel)
    {
        // ModelMatrix festlegen.
        mat4 modelMatrix;	
        glm_mat4_identity(modelMatrix);

        // Beispielhafte Veränderungen an der Modelmatrix.
        // Mehr Informationen auf: https://cglm.readthedocs.io/en/latest/affine.html

        glm_scale_make(modelMatrix, input->rendering.scale);

        glm_rotate_x(modelMatrix, glm_rad(input->rendering.rotate[0]), modelMatrix);
        glm_rotate_y(modelMatrix, glm_rad(input->rendering.rotate[1]), modelMatrix);
        glm_rotate_z(modelMatrix, glm_rad(input->rendering.rotate[2]), modelMatrix);

        glm_translate(modelMatrix, input->rendering.translate);

        // MVP Matrix vorberechnen.
        mat4 mvpMatrix;
        glm_mat4_mul(projectionMatrix, viewMatrix, mvpMatrix);
        glm_mat4_mul(mvpMatrix, modelMatrix, mvpMatrix);

        if (data->modelShader != NULL) 
        {

        

        //shader_useShader(data->skyboxShader);
        shader_useShader(data->modelShader);
        shader_setMat4(data->modelShader, "u_mvpMatrix", &mvpMatrix);

        // Shader model Matrix uebergeben
        shader_setMat4(data->modelShader, "u_modelMatrix", &modelMatrix);
        // Shader view Matrix uebergeben
        shader_setMat4(data->modelShader, "u_viewMatrix", &viewMatrix);


        // Lichtquelle
        vec3 lightPosVec = { input->rendering.lightDir[0],input->rendering.lightDir[1],input->rendering.lightDir[2] };

        // Lichtquelle
        vec3 lightComp;
        glm_vec4_copy3(input->rendering.lightComp, lightComp);

        // Lichtquelle
        vec3 lightColor;
        glm_vec4_copy3(input->rendering.lightColor, lightColor);
        
        /** Lichtfarbe ambient*/
        vec3 matAmbient = { input->rendering.lightComp[0],input->rendering.lightComp[0],input->rendering.lightComp[0] };

        /** Lichtfarbe specular*/
        vec3 matSpecular = { input->rendering.lightComp[1],input->rendering.lightComp[1],input->rendering.lightComp[1] };

        /** Lichtfarbe diffuse*/
        vec3 matDiffuse = { input->rendering.lightComp[2],input->rendering.lightComp[2],input->rendering.lightComp[2] };

        /** Lichtfarbe diffuse*/
        vec3 viewPos = { 0.0, 0.0, 0.0 };
        camera_getPosition(input->mainCamera, viewPos);

        shader_setVec3(data->modelShader, "u_lightPosVec", &lightPosVec);
        shader_setVec3(data->modelShader, "u_lightComp", &lightComp);
        shader_setVec3(data->modelShader, "u_lightColor", &lightColor);
        shader_setVec3(data->modelShader, "u_matAmbient", &matAmbient);
        shader_setVec3(data->modelShader, "u_matDiffuse", &matDiffuse);
        shader_setVec3(data->modelShader, "u_matSpecular", &matSpecular);
        shader_setVec3(data->modelShader, "u_viewPos", &viewPos);

        vec3 fogColor;
        glm_vec4_copy3(input->rendering.clearColor, fogColor);
        shader_setVec3(data->modelShader, "u_fogColor", &fogColor);

        shader_setBool(data->modelShader, "u_useFog", input->showFog);

        shader_setFloat(data->modelShader, "u_density", input->density);

        //int shaderChoice = input->shaderChoice;
        shader_setInt(data->modelShader, "u_shaderChoice", input->shaderChoice);

        // Modell zeichnen
        common_pushRenderScope("Scene");
        model_drawModel(input->rendering.userModel, data->modelShader);
        common_popRenderScope();
        }
    }

    // Tiefentest nach der 3D Szene wieder deaktivieren.
    glDisable(GL_DEPTH_TEST);

    // Wireframe am Ende wieder deaktivieren.
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void rendering_cleanup(ProgContext* ctx)
{
    RenderingData* data = ctx->rendering;

    // Zum Schluss müssen noch die belegten Ressourcen freigegeben werden.
    shader_deleteShader(data->modelShader);
    shader_deleteShader(data->skyboxShader);

    free(ctx->rendering);
}
