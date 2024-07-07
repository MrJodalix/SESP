/**
 * Modul für das Erstellung und Benutzen von Materialen.
 * Ein Material ist dabei eine Sammlung von Texturen und Einstellungen,
 * die das Aussehen eines Objektes bestimmen.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

#ifndef MATERIAL_H
#define MATERIAL_H

#include "common.h"

#include <assimp/scene.h> 

#include "shader.h"

////////////////////////////////// KONSTANTEN //////////////////////////////////

#define MATERIAL_DEFAULT_AMBIENT (vec3){0.3f,0.3f,0.3f}
#define MATERIAL_DEFAULT_DIFFUSE (vec3){1.0f, 1.0f, 1.0f}
#define MATERIAL_DEFAULT_SPECULAR (vec3){0.2f,0.2f,0.2f}
#define MATERIAL_DEFAULT_SHININESS 2
#define MATERIAL_DEFAULT_EMISSION (vec3){0,0,0}

//////////////////////////// ÖFFENTLICHE DATENTYPEN ////////////////////////////

// Datenstruktur für die Repräsentation eines Materials.
struct Material;
typedef struct Material Material;

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Erstellt ein neues, texturloses Material.
 * 
 * @param ambient ambiente Farbe
 * @param diffuse diffuse Farbe
 * @param specular spekulare Farbe
 * @param emission die Emissionsfarbe
 * @param shininess der Glanzfaktor
 * @return das neue Material
 */
Material* material_createMaterial(vec3 ambient, vec3 diffuse, vec3 specular,
                                  vec3 emission, float shininess);

/**
 * Erstellt ein neues Material, das auch Texturen verwenden kann, sofern diese
 * nicht auf NULL gesetzt  wurden.
 *
 * @param ambient ambiente Farbe
 * @param diffuse diffuse Farbe
 * @param specular spekulare Farbe
 * @param emission die Emissionsfarbe
 * @param shininess der Glanzfaktor
 * @param diffuseMap Texturpfad zur Diffuse Textur
 * @param specularMap Texturpfad zur Spekular Textur
 * @param normalMap Texturpfad zur Normalen Textur
 * @param emissionMap Texturpfad zur Emissions Textur
 * @return das neue Material
 */
Material* material_createMaterialFromMaps(vec3 ambient, vec3 diffuse, 
        vec3 specular, vec3 emission, float shininess, const char* diffuseMap, 
        const char* specularMap, const char* normalMap,
        const char* emissionMap);

/**
 * Konvertiert ein AssImp Material in das eigene Materialsystem.
 *
 * @param aiMat das zu konvertierende Material
 * @param directory das Verzeichnis, in dem die Modelldatei liegt
 * @return das neue Material
 */
Material* material_createMaterialFromAI(struct aiMaterial* aiMat, 
                                        const char* directory);

/**
 * Aktiviert ein Material für einen bestimmten Shader.
 * 
 * @param shader der zu verwendene Shader
 * @param mat das zu aktivierende Material
 */
void material_useMaterial(Shader* shader, Material* mat);

/**
 * Löscht ein Material.
 * 
 * @param mat das zu löschende Material
 */
void material_deleteMaterial(Material* mat);

#endif // MATERIAL_H
