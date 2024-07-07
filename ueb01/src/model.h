/**
 * Modul für das Laden von 3D Modellen.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

#ifndef MODEL_H
#define MODEL_H

#include "common.h"

#include "shader.h"

//////////////////////////// ÖFFENTLICHE DATENTYPEN ////////////////////////////

// Datenstruktur für die Repräsentation eines 3D Modells.
struct Model;
typedef struct Model Model;

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Lädt ein 3D Modell aus einer Datei.
 * Dieser Aufruf kann abhängig von der Modellgröße länger dauern.
 * 
 * @param filename der Dateiname des Modells
 * @return ein neues 3D Modell oder NULL wenn ein Fehler aufgetreten ist
 */
Model* model_loadModel(const char* filename);

/**
 * Zeigt ein 3D Modell an.
 * 
 * @param model das anzuzeigende 3D Modell
 * @param shader der zu verwendende Shader
 */
void model_drawModel(Model* model, Shader* shader);

/**
 * Löscht ein zuvor geladenes 3D Modell wieder.
 * 
 * @param model das zu löschende 3D Modell
 */
void model_deleteModel(Model* model);

#endif // MODEL_H
