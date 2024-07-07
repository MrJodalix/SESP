/**
 * Modul für das verarbeiten von einzelnen Meshes / 3D Modellen.
 * Ein Mesh ist dabei eine Sammlung von Vertices und Indecies die gerendert
 * werden können. Außerdem besitzt ein Mesh auch ein Material.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

#ifndef MESH_H
#define MESH_H

#include "common.h"

#include "shader.h"
#include "material.h"

//////////////////////////// ÖFFENTLICHE DATENTYPEN ////////////////////////////

// Datenstruktur für einen Vertex.
struct Vertex
{
    vec3 position;
    vec3 normal;
    vec2 texCoord;
};
typedef struct Vertex Vertex;

// Datenstruktur für die Repräsentation eines Meshs.
struct Mesh;
typedef struct Mesh Mesh;

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Erstellt ein neues Mesh aus Vertex- und Indexdaten.
 * Alle Daten werden dabei übernommen und dürfen vom Aufrufer nicht gelöscht
 * werden. Die Löschung erfolgt automatisch beim Löschen des Meshes.
 * 
 * @param vertices die Vertices des Meshes
 * @param vertexCount die Anzahl der Vertices
 * @param indices die Indices des Meshes
 * @param indexCount die Anzahl der Indices
 * @param material das zu verwendende Material
 * @return ein neues Mesh
 */
Mesh* mesh_createMesh(Vertex* vertices, GLuint vertexCount, 
                      GLint* indices, GLuint indexCount, Material* material);

/**
 * Zeigt ein Mesh mit einem festgelegten Shader an.
 * Der Shader muss zuvor nicht aktiviert werden.
 * 
 * @param mesh das zu zeichnende Mesh
 * @param shader der zu verwendene Shader
 */
void mesh_drawMesh(Mesh* mesh, Shader* shader);

/**
 * Löscht ein Mesh.
 * 
 * @param mesh das Mesh, das gelöscht werden soll
 */
void mesh_deleteMesh(Mesh* mesh);

#endif // MESH_H
