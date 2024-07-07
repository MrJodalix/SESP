/**
 * Modul für das Speichern von Lichtquellen.
 * 
 * Um die Funktionen light_activateDirLight und light_activatePointLight
 * nutzen zu können, müssen im Shader die folgenden structs und uniforms
 * vorhanden sein:
 * 
 * // Für Richtungslichter:
 * struct DirLight
 * {
 *      vec3 dir;
 *      vec3 amb;
 *      vec3 diff;
 *      vec3 spec;
 * };
 * 
 * uniform DirLight dirLight;
 * 
 * // Für Punktlichter:
 * struct PointLight
 * {
 *      vec3 pos;
 *      vec3 amb;
 *      vec3 diff;
 *      vec3 spec;
 *      float constant;
 *      float linear;
 *      float quadratic;
 * };
 * 
 * uniform PointLight pointLight;
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

#include "light.h"

////////////////////////////////// KONSTANTEN //////////////////////////////////

#define AMBIENT_FACTOR 0.2f
#define DIFFUSE_FACTOR 0.7f
#define SPECULAR_FACTOR 1.0f

#define DEFAULT_CONSTANT 1.0f
#define DEFAULT_LINEAR 0.14f
#define DEFAULT_QUADRATIC 0.07f

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

DirLight* light_createDirLight(vec3 dir, vec3 color)
{
    DirLight* light = malloc(sizeof(DirLight));

    glm_vec3_scale(color, AMBIENT_FACTOR, light->ambient);
    glm_vec3_scale(color, DIFFUSE_FACTOR, light->diffuse);
    glm_vec3_scale(color, SPECULAR_FACTOR, light->specular);

    glm_vec3_copy(dir, light->direction);

    return light;
}

PointLight* light_createPointLight(vec3 pos, vec3 color)
{
    return light_createPointLightEx(
        pos, 
        color, 
        DEFAULT_CONSTANT, 
        DEFAULT_LINEAR, 
        DEFAULT_QUADRATIC
    );
}

PointLight* light_createPointLightEx(vec3 pos, vec3 color, float constant, 
                                     float linear, float quadratic)
{
    PointLight* light = malloc(sizeof(PointLight));

    glm_vec3_scale(color, AMBIENT_FACTOR, light->ambient);
    glm_vec3_scale(color, DIFFUSE_FACTOR, light->diffuse);
    glm_vec3_scale(color, SPECULAR_FACTOR, light->specular);

    glm_vec3_copy(pos, light->position);

    light->constant = constant;
    light->linear = linear;
    light->quadratic = quadratic;

    return light;
}

void light_activateDirLight(DirLight* light, Shader* shader)
{
    shader_setVec3(shader, "dirLight.dir", &light->direction);
    shader_setVec3(shader, "dirLight.amb", &light->ambient);
    shader_setVec3(shader, "dirLight.diff", &light->diffuse);
    shader_setVec3(shader, "dirLight.spec", &light->specular);
}

void light_activatePointLight(PointLight* light, Shader* shader)
{
    shader_setVec3(shader, "pointLight.pos", &light->position);
    shader_setVec3(shader, "pointLight.amb", &light->ambient);
    shader_setVec3(shader, "pointLight.diff", &light->diffuse);
    shader_setVec3(shader, "pointLight.spec", &light->specular);

    shader_setFloat(shader, "pointLight.constant", light->constant);
    shader_setFloat(shader, "pointLight.linear", light->linear);
    shader_setFloat(shader, "pointLight.quadratic", light->quadratic);
}

void light_deleteDirLight(DirLight* light)
{
    // Aktuell ist kein Cleanup nötig.
    free(light);
}

void light_deletePointLight(PointLight* light)
{
    // Aktuell ist kein Cleanup nötig.
    free(light);
}
