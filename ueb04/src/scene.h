/**
 * Modul für das Verarbeiten und Laden von 3D Szenen.
 * Eine Szene ist dabei ein serialisiertes Datenobjekt, das festlegt:
 * -> welche 3D Modelle angezeigt werden (nur eins zur Zeit)
 * -> welche Skybox verwendet werden soll (nicht implementiert)
 * -> welche Lichter (inklusiver Eigenschaften dieser) in der Szene sind
 * -> ggf. weitere Eigenschaften
 * 
 * Szenen werden dabei aus JSON Dateien geladen.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

#ifndef SCENE_H
#define SCENE_H

#include "common.h"

#include "model.h"
#include "light.h"

//////////////////////////// ÖFFENTLICHE DATENTYPEN ////////////////////////////

// Datenstruktur zum Speichern einer kompletten Szene.
struct Scene
{
    char* name;
    Model* model;

    int countDirLights;
    DirLight** dirLights;

    int countPointLights;
    PointLight** pointLights;
};
typedef struct Scene Scene;

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Lädt eine Szene aus einer JSON Datei.
 * 
 * Die Funktion gibt bei einem Fehler direkt auf der Konsole eine Meldung aus.
 * 
 * @param filename der Dateiname der JSON Datei.
 * @return eine neue Szene oder NULL wenn etwas schief ging.
 */
Scene* scene_loadScene(const char* filename);

/**
 * Erstellt eine Szene aus einer 3D Modelldatei.
 * Die Szene wird dabei keinerlei Lichter, etc. enthalten. Ausschließlich das
 * 3D Modell wird gesetzt sein.
 * 
 * Die Funktion gibt bei einem Fehler direkt auf der Konsole eine Meldung aus.
 * 
 * @param filename der Dateiname der 3D Modell Datei.
 * @return eine neue Szene oder NULL wenn etwas schief ging.
 */
Scene* scene_fromModel(const char* filename);

/**
 * Fügt ein neues Richtungslicht zu einer Szene hinzu.
 * 
 * @param scene die Szene an die das Richtungslicht geängt werden soll
 * @param light das Licht, das hinzugefügt werden soll
 */
void scene_addDirLight(Scene* scene, DirLight* light);

/**
 * Fügt ein neues Punktlicht zu einer Szene hinzu.
 * 
 * @param scene die Szene an die das Punktlicht geängt werden soll
 * @param light das Licht, das hinzugefügt werden soll
 */
void scene_addPointLight(Scene* scene, PointLight* light);

/**
 * Löscht eine Szene und alle damit verknüpften Ressourcen.
 * 
 * @param scene die Szene, die gelöscht werden soll.
 */
void scene_deleteScene(Scene* scene);

#endif // SCENE_H
