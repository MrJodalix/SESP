/**
 * Modul für das Erstellung und Benutzen von Materialen.
 * Ein Material ist dabei eine Sammlung von Texturen und Einstellungen,
 * die das Aussehen eines Objektes bestimmen.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

#include "material.h"

#include "texture.h"

////////////////////////////// LOKALE DATENTYPEN ///////////////////////////////

// Datenstruktur für die Repräsentation eines Materials.
struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 emission;
    float shininess;

    bool useDiffuseMap;
    GLuint diffuseMap;

    bool useNormalMap;
    GLuint normalMap;

    bool useSpecularMap;
    GLuint specularMap;

    bool useEmissionMap;
    GLuint emissionMap;
};

////////////////////////////// LOKALE FUNKTIONEN ///////////////////////////////

/**
 * Lädt eine Textur aus AssImp in den Speicher.
 * 
 * @param aiMat das Material für das die Textur gesetzt ist
 * @param directory der Pfad zu der Modelldatei
 * @param type der Typ der Textur
 * @return eine OpenGL Textur ID oder 0 wenn etwas schief ging
 */
static GLuint material_loadAITexture(struct aiMaterial* aiMat,
                                     const char * directory,
                                     enum aiTextureType type)
{
    // Die Textur-ID, die wir zurückgeben werden.
    GLuint textureID = 0;

    // Als erstes rufen wir den Pfad der Textur ab.
    struct aiString str;
    aiGetMaterialTexture(aiMat, type, 0, &str, NULL, 
                         NULL, NULL, NULL, NULL, NULL);

    // Als nächstes prüfen wir, ob es sich um eine eingebettete Textur handelt.
    if (*str.data == '*') 
    {
        // Aktuell gibt es keine Unterstützung für eingebettete Texturen.
        fprintf(stderr, "Error: Embedded textures are not supported!\n");
    } 
    else
    {
        // Wenn es sich um keine eingebettete Textur handelt, laden wir sie
        // über die normale Texturladefunktion.
        // Dazu müssen wir ersteinmal den Pfad zu der Textur bestimmen.
        char* filePath = malloc(str.length + strlen(directory) + 1);
        strcpy(filePath, directory);
        strcat(filePath, str.data);
        textureID = texture_loadTexture(filePath, GL_REPEAT);
        free(filePath);
    }

    return textureID;
}

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

Material* material_createMaterial(vec3 ambient, vec3 diffuse, vec3 specular,
                                  vec3 emission, float shininess)
{
    // Wir verwenden die Funktion zum Laden von Texturen, übergeben aber
    // dabei keine Pfade. Dadurch entsteht ein Texturloses Material.
    return material_createMaterialFromMaps(
        ambient, diffuse, specular, emission, shininess,
        NULL, NULL, NULL, NULL
    );
}

Material* material_createMaterialFromMaps(vec3 ambient, vec3 diffuse, 
        vec3 specular, vec3 emission, float shininess, const char* diffuseMap, 
        const char* specularMap, const char* normalMap,
        const char* emissionMap)
{
    // Den Speicher für das neue Material reservieren.
    Material* mat = malloc(sizeof(Material));

    // Dann werden alle Eigenschaften kopiert.
    glm_vec3_copy(ambient, mat->ambient);
    glm_vec3_copy(diffuse, mat->diffuse);
    glm_vec3_copy(specular, mat->specular);
    glm_vec3_copy(emission, mat->emission);

    mat->shininess = shininess;

    // Mit dem folgenden Makro können alle gesetzten Texturen geladen werden.
    #define MATERIAL_LOAD_TEX(use, map, wrapping) {                            \
        mat->use = (map != NULL);                                              \
        if (mat->use)                                                          \
        {                                                                      \
            mat->map = texture_loadTexture(map, wrapping);                     \
        }                                                                      \
    }

    MATERIAL_LOAD_TEX(useDiffuseMap, diffuseMap, GL_REPEAT);
    MATERIAL_LOAD_TEX(useNormalMap, normalMap, GL_REPEAT);
    MATERIAL_LOAD_TEX(useSpecularMap, specularMap, GL_REPEAT);
    MATERIAL_LOAD_TEX(useEmissionMap, emissionMap, GL_REPEAT);

    #undef MATERIAL_LOAD_TEX

    return mat;
}

Material* material_createMaterialFromAI(struct aiMaterial* aiMat, 
                                        const char* directory)
{
    // Speicher für das Material reservieren.
    Material* mat = malloc(sizeof(Material));

    // Temporäre Variable zum Einlesen von Farben.
    struct aiColor4D tempColor;

    // Das folgende Makro wird benutzt, um die Farben aus AI zu importieren.
    #define MATERIAL_LOAD_AI_COLOR(key, aiKey, default) {                      \
        if (AI_SUCCESS == aiGetMaterialColor(aiMat, aiKey, &tempColor))        \
        {                                                                      \
            mat->key[0] = tempColor.r;                                         \
            mat->key[1] = tempColor.g;                                         \
            mat->key[2] = tempColor.b;                                         \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            glm_vec3_copy(MATERIAL_DEFAULT_ ## default, mat->key);             \
        }                                                                      \
    }

    MATERIAL_LOAD_AI_COLOR(ambient, AI_MATKEY_COLOR_AMBIENT, AMBIENT);
    MATERIAL_LOAD_AI_COLOR(diffuse, AI_MATKEY_COLOR_DIFFUSE, DIFFUSE);
    MATERIAL_LOAD_AI_COLOR(specular, AI_MATKEY_COLOR_SPECULAR, SPECULAR);
    MATERIAL_LOAD_AI_COLOR(emission, AI_MATKEY_COLOR_EMISSIVE, EMISSION);

    #undef MATERIAL_LOAD_AI_COLOR

    // Die Shininess auslesen.
    float shininess;
    if (AI_SUCCESS == 
        aiGetMaterialFloatArray(aiMat, AI_MATKEY_SHININESS, &shininess, NULL))
    {
        mat->shininess = shininess;
    }
    else
    {
        mat->shininess = MATERIAL_DEFAULT_SHININESS;
    }

    // Als nächstes müssen die Texturen geladen werden.
    // Dabei hilft das folgende Makro.
    #define MATERIAL_LOAD_AI_TEX(aiType, use, map) {                           \
        GLuint n = aiGetMaterialTextureCount(aiMat, aiTextureType_ ## aiType); \
        mat->use = n > 0;                                                      \
        if (mat->use)                                                          \
        {                                                                      \
            mat->map = material_loadAITexture(aiMat,                           \
                directory, aiTextureType_ ## aiType);                          \
        }                                                                      \
    }

    MATERIAL_LOAD_AI_TEX(DIFFUSE, useDiffuseMap, diffuseMap);
    MATERIAL_LOAD_AI_TEX(NORMALS, useNormalMap, normalMap);
    MATERIAL_LOAD_AI_TEX(SPECULAR, useSpecularMap, specularMap);
    MATERIAL_LOAD_AI_TEX(EMISSIVE, useEmissionMap, emissionMap);

    #undef MATERIAL_LOAD_AI_TEX

    return mat;
}

void material_useMaterial(Shader* shader, Material* mat)
{
    // Zuerst müssen wir den Shader aktivieren.
    shader_useShader(shader);

    // Danach übertragen wir die Materialeigenschaften mit dem folgenden Makro.
    #define MATERIAL_SET_VEC3(term) {                                          \
        shader_setVec3(shader, "u_material." #term, &mat->term);                 \
    }

    MATERIAL_SET_VEC3(ambient);
    MATERIAL_SET_VEC3(diffuse);
    MATERIAL_SET_VEC3(specular);
    MATERIAL_SET_VEC3(emission);

    #undef MATERIAL_SET_VEC3
    
    // Shininess & dispFactor werden als einzelne Floats übergeben.
    shader_setFloat(shader, "u_material.shininess", mat->shininess);

    // Als nächstes setzen wir die Texturen über das folgende Makro.
    #define MATERIAL_SET_TEX(idx, use, map) {                                  \
        shader_setBool(shader, "u_material." #use, mat->use);                    \
        if (mat->use)                                                          \
        {                                                                      \
            glActiveTexture(GL_TEXTURE ## idx);                                \
            glBindTexture(GL_TEXTURE_2D, mat->map);                            \
            shader_setInt(shader, "u_material." #map, idx);                      \
        }                                                                      \
    }

    MATERIAL_SET_TEX(0, useDiffuseMap, diffuseMap);
    MATERIAL_SET_TEX(1, useSpecularMap, specularMap);
    MATERIAL_SET_TEX(2, useNormalMap, normalMap);
    MATERIAL_SET_TEX(3, useEmissionMap, emissionMap);

    #undef MATERIAL_SET_TEX
}

void material_deleteMaterial(Material* mat)
{
    // Material nur löschen, wenn es existiert.
    if (mat == NULL)
    {
        return;
    }

    // Mithilfe des folgenden Makros alle gesetzten Texturen löschen.
    #define MATERIAL_DELETE_TEX(a, b)  {if (mat->a) {\
                                            texture_deleteTexture(mat->b);}}

    MATERIAL_DELETE_TEX(useDiffuseMap, diffuseMap);
    MATERIAL_DELETE_TEX(useNormalMap, normalMap);
    MATERIAL_DELETE_TEX(useSpecularMap, specularMap);
    MATERIAL_DELETE_TEX(useEmissionMap, emissionMap);

    #undef MATERIAL_DELETE_TEX

    free(mat);
}
