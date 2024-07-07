/**
 * Modul für das Laden von 3D Modellen.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

#include "model.h"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "material.h"
#include "mesh.h"
#include "utils.h"
#include "input.h"

////////////////////////////// LOKALE DATENTYPEN ///////////////////////////////

// Datenstruktur für die Repräsentation eines 3D Modells.
struct Model
{
    Mesh** meshes;
    unsigned int meshCount;
    char* directory;
};

////////////////////////////// LOKALE FUNKTIONEN ///////////////////////////////

/**
 * Verarbeitet ein Mesh aus einem AssImp Knoten.
 * 
 * @param srcMesh das AI Mesh Objekt, das konvertiert werden soll
 * @param transform eine Transformation, die auf alle Vertices angewendet wird
 * @param scene die Szene, aus der das Mesh kommt
 * @param directory das Verzeichnis, in dem die Modelldatei liegt
 * @return das neue Mesh
 */
static Mesh* model_processMesh(struct aiMesh* srcMesh,
                               mat4 transform,
                               const struct aiScene* scene, 
                               const char* directory)
{
    // Zuerst prüfen, ob der Primitiventyp Dreiecke ist. Sonst kann kein Mesh
    // aufgebaut werden.
    if (srcMesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
    {
        fprintf(
            stderr, 
            "Error: Can't load mesh with other primitives than triangles! \n"
        );

        return NULL;
    }

    // Vertices anlegen.
    unsigned int vertexCount = srcMesh->mNumVertices;
    Vertex* vertices = malloc(vertexCount * sizeof(Vertex));

    // Alle Vertices verarbeiten.
    for (unsigned int i = 0; i < vertexCount; i++)
    {
        // Position kopieren und dabei über die Transformationsmatrix
        // anpassen.
        vec4 vPos;

        vPos[0] = srcMesh->mVertices[i].x;
        vPos[1] = srcMesh->mVertices[i].y;
        vPos[2] = srcMesh->mVertices[i].z;
        vPos[3] = 1.0f;

        glm_mat4_mulv(transform, vPos, vPos);

        vertices[i].position[0] = vPos[0];
        vertices[i].position[1] = vPos[1];
        vertices[i].position[2] = vPos[2];

        //Die Normalen müssen beim Kopieren ebenfalls angepasst werden.
        vec4 vNorm;

        vNorm[0] = srcMesh->mNormals[i].x;
        vNorm[1] = srcMesh->mNormals[i].y;
        vNorm[2] = srcMesh->mNormals[i].z;
        vNorm[3] = 0;

        mat4 normalMatrix;
        glm_mat4_inv(transform, normalMatrix);
        glm_mat4_transpose(normalMatrix);
        glm_mat4_mulv(normalMatrix, vNorm, vNorm);
        glm_normalize(vNorm);

        vertices[i].normal[0] = vNorm[0];
        vertices[i].normal[1] = vNorm[1];
        vertices[i].normal[2] = vNorm[2];

        // Prüfen, ob eine Texturkoordinate verfügbar ist.
        if (srcMesh->mTextureCoords[0])
        {
            // Wenn ja kopieren wir sie.
            vertices[i].texCoord[0] = srcMesh->mTextureCoords[0][i].x;
            vertices[i].texCoord[1] = srcMesh->mTextureCoords[0][i].y;
        }
        else
        {
            // Wenn nicht setzen wir die Werte auf 0.
            vertices[i].texCoord[0] = 0.0f;
            vertices[i].texCoord[1] = 0.0f;
        }

        //Die Normalen müssen beim Kopieren ebenfalls angepasst werden.
        vec4 vTangent;

        vTangent[0] = srcMesh->mTangents[i].x;
        vTangent[1] = srcMesh->mTangents[i].y;
        vTangent[2] = srcMesh->mTangents[i].z;
        vTangent[3] = 0;

        glm_mat4_mulv(normalMatrix, vTangent, vTangent);
        glm_normalize(vTangent);

        vertices[i].tangent[0] = vTangent[0];
        vertices[i].tangent[1] = vTangent[1];
        vertices[i].tangent[2] = vTangent[2];

        //Die Normalen müssen beim Kopieren ebenfalls angepasst werden.
        vec4 vBiTangent;

        vBiTangent[0] = srcMesh->mBitangents[i].x;
        vBiTangent[1] = srcMesh->mBitangents[i].y;
        vBiTangent[2] = srcMesh->mBitangents[i].z;
        vBiTangent[3] = 0;

        glm_mat4_mulv(normalMatrix, vBiTangent, vBiTangent);
        glm_normalize(vBiTangent);

        vertices[i].biTangent[0] = vBiTangent[0];
        vertices[i].biTangent[1] = vBiTangent[1];
        vertices[i].biTangent[2] = vBiTangent[2];
    }
    // Indices anlegen.
    GLint* indices;
    indices = malloc((srcMesh->mNumFaces * 3) * sizeof(GLint));

    // Danach alle Indices kopieren.
    unsigned int indexCount = 0;
    for (unsigned int i = 0; i < srcMesh->mNumFaces; i++)
    {
        for (unsigned int j = 0; j < srcMesh->mFaces[i].mNumIndices; j++)
        {
            indices[indexCount] = srcMesh->mFaces[i].mIndices[j];
            indexCount++;
        }
    }

    // Anschließende muss das Material für das Mesh erzeugt werden.
    Material* material;
    if (srcMesh->mMaterialIndex >= 0)
    {
        // Wenn es ein Material gibt, muss es von AI zum eigenen System
        // konvertiert werden.
        material = material_createMaterialFromAI(
            scene->mMaterials[srcMesh->mMaterialIndex],
            directory
        );		
    }
    else 
    {
        // Wenn es kein Material gab, wird ein Standardmaterial erzeugt.
        material = material_createMaterial(
            MATERIAL_DEFAULT_AMBIENT, 
            MATERIAL_DEFAULT_DIFFUSE,
            MATERIAL_DEFAULT_SPECULAR,
            MATERIAL_DEFAULT_EMISSION,
            MATERIAL_DEFAULT_SHININESS
        );
    }

    // Zum Schluss erzeugen wir ein neues Mesh und geben es zurück.
    return mesh_createMesh(
        vertices, vertexCount, 
        indices, indexCount,
        material
    );
}

/**
 * Verarbeitet einen AssImp Knoten. Die erzeugten Meshes werden
 * an das übergebene Modell gehängt. Diese Funktion arbeitet rekursiv.
 * Transformationen der Knoten werden mit einbezogen, sodass die Meshes
 * alle korrekt im Raum angeordnet werden.
 * 
 * @param model das Modell, an das die Meshes gehängt werden sollen
 * @param scene die AI Szene, aus der die Daten stammen
 * @param node der aktuelle Knotenpunkt
 * @param parentTransform die Transformationsmatrix des Elternknoten
 */
static void model_processNode(Model* model, 
                              const struct aiScene* scene, 
                              const struct aiNode* node,
                              mat4 parentTransform)
{
    // Transformationsmatrix auslesen und konvertieren.
    struct aiMatrix4x4 m = node->mTransformation;
    mat4 transform = {
        { m.a1, m.b1, m.c1, m.d1 },
        { m.a2, m.b2, m.c2, m.d2 },
        { m.a3, m.b3, m.c3, m.d3 },
        { m.a4, m.b4, m.c4, m.d4 },
    };

    // Die Transformation des Elternknoten anwenden.
    glm_mat4_mul(parentTransform, transform, transform);

    // Wenn Meshes existieren, werden diese zum Modell hinzugefügt.
    if (node->mNumMeshes > 0)
    {
        // Zuerst vergrößern wir das Mesh-Array.
        unsigned int oldCount = model->meshCount;
        model->meshCount += node->mNumMeshes;
        model->meshes = realloc(
            model->meshes, 
            model->meshCount * sizeof(Mesh*)
        );

        // Danach gehen wir durch alle Meshes und verarbeiten sie.
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            struct aiMesh* srcMesh = scene->mMeshes[node->mMeshes[i]];
            model->meshes[oldCount + i] = model_processMesh(
                srcMesh,
                transform,
                scene, 
                model->directory
            );
        }
    }

    // Alle Kindknoten verarbeiten.
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        model_processNode(model, scene, node->mChildren[i], transform);
    }
}

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

Model* model_loadModel(const char* filename)
{
    // Die gewünschte Datei importieren.
    const struct aiScene* scene = aiImportFile(
        filename,
        aiProcess_FlipUVs               | // Alle UV Koord. spiegeln
        aiProcess_Triangulate           | // Trianguliert Flächen wenn nötig
        aiProcess_CalcTangentSpace      | // Berechnet Tangente und Bitangente
        aiProcess_JoinIdenticalVertices | // Fügt gleiche Vertices zusammen
        aiProcess_SortByPType           | // Zerteilt das Mesh nach Primitiven
        aiProcess_GenSmoothNormals        // Normalen erzeugen, wenn sie fehlen
    );
    if (scene == NULL || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
    {
        fprintf(
            stderr, 
            "Error: Couldn't import model \"%s\" because: %s\n",
            filename, aiGetErrorString()
        );
        return NULL;
    }
    if (!scene->mRootNode)
    {
        fprintf(
            stderr, 
            "Error: Couldn't import model \"%s\" because it has no root node\n",
            filename
        );
        return NULL;
    }

    // Den nötigen Speicher reservieren.
    Model* model = malloc(sizeof(Model));
    model->meshCount = 0;
    model->meshes = NULL;

    // Wir brauchen den Ordnerpfad um die Texturen des Modells zu finden.
    model->directory = utils_getDirectory(filename);

    // Das Modell rekursiv in OpenGL laden.
    mat4 identity;
    glm_mat4_identity(identity);
    model_processNode(model, scene, scene->mRootNode, identity);

    // Zum Schluss müssen nur noch die Assimp-Ressourcen wieder frei gegeben
    // werden.
    aiReleaseImport(scene);

    return model;
}

void model_drawModel(Model* model, Shader* shader, bool isModel)
{
    // Alle Meshes des Modells werden nacheinander gerendert.
    for (unsigned int i = 0; i < model->meshCount; i++)
    {
        mesh_drawMesh(model->meshes[i], shader, isModel);
    }
}

void model_deleteModel(Model* model)
{
    // Zuerst werden alle Meshes gelöscht.
    for (unsigned int i = 0; i < model->meshCount; i++)
    {
        mesh_deleteMesh(model->meshes[i]);
    }

    // Danach wird das Modell freigegeben.
    free(model->meshes);
    free(model->directory);
    free(model);
}
