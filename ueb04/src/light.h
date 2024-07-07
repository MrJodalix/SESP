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

/**
 *      Tabelle mit unterschiedlichen Attenuation Werten.
 *      Quelle: 
 *      http://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
 * 
 *      Distance   Constant   Linear      Quadratic
 *      =============================================
 *      7          1.0         0.7        1.8
 *      13         1.0         0.35       0.44
 *      20         1.0         0.22       0.2
 *      32         1.0         0.14       0.07
 *      50         1.0         0.09       0.032
 *      65         1.0         0.07       0.017
 *      100        1.0         0.045      0.0075
 *      160        1.0         0.027      0.0028
 *      200        1.0         0.022      0.0019
 *      325        1.0         0.014      0.0007
 *      600        1.0         0.007      0.0002
 *      3250       1.0         0.0014     0.000007
 */

#ifndef LIGHT_H
#define LIGHT_H

#include "common.h"
#include "shader.h"

//////////////////////////// ÖFFENTLICHE DATENTYPEN ////////////////////////////

// Ein Richtungslicht.
struct DirLight
{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
typedef struct DirLight DirLight;

// Eine Punktlichtquelle.
struct PointLight
{
    vec3 position;
    vec3 color;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};
typedef struct PointLight PointLight;

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Erzeugt ein neues Richtungslicht.
 * 
 * @param dir die Richtung
 * @param color die Farbe des Lichtes
 * @return ein neues Richtungslicht
 */
DirLight* light_createDirLight(vec3 dir, vec3 color);

/**
 * Erzeugt ein neues Punktlicht.
 * 
 * @param pos die Position des Lichtes
 * @param color die Farbe des Lichtes
 * @return ein neues Punktlicht
 */
PointLight* light_createPointLight(vec3 pos, vec3 color);

/**
 * Erzeugt ein neues Punktlicht mit erweiterten Einstellungen.
 * 
 * @param pos die Position des Lichtes
 * @param color die Farbe des Lichtes
 * @param constant der Konstante-Falloff Wert
 * @param linear der Lineare-Falloff Wert
 * @param quadratic der Quadratische-Falloff Wert
 * @return ein neues Punktlicht
 */
PointLight* light_createPointLightEx(vec3 pos, vec3 color, float constant, 
                                     float linear, float quadratic);

/**
 * Übergibt die Lichtparameter per Uniform an den übergebenen Shader.
 * Der Shader MUSS zuvor bereits aktiviert worden sein.
 * 
 * Für mehr Informationen siehe den Dateikopf.
 * 
 * @param light das zu aktivierende Licht
 * @param shader der Shader, an den das Licht übergeben werden soll
 */
void light_activateDirLight(DirLight* light, Shader* shader);

/**
 * Übergibt die Lichtparameter per Uniform an den übergebenen Shader.
 * Der Shader MUSS zuvor bereits aktiviert worden sein.
 * 
 * Für mehr Informationen siehe den Dateikopf.
 * 
 * @param light das zu aktivierende Licht
 * @param shader der Shader, an den das Licht übergeben werden soll
 */
void light_activatePointLight(PointLight* light, Shader* shader);

/**
 * Löscht ein Richtungslicht.
 * 
 * @param light das zu löschende Licht.
 */
void light_deleteDirLight(DirLight* light);

/**
 * Löscht ein Punktlicht.
 * 
 * @param light das zu löschende Licht.
 */
void light_deletePointLight(PointLight* light);

#endif // LIGHT_H
