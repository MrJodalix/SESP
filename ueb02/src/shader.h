/**
 * Modul zum Laden und Verwenden von Shadern.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

#ifndef SHADER_H
#define SHADER_H

#include "common.h"

//////////////////////////// ÖFFENTLICHE DATENTYPEN ////////////////////////////

// Datenstruktur, die einen Shader repräsentiert.
struct Shader;
typedef struct Shader Shader;

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Erzeugt einen neuen, leeren Shader.
 * 
 * @return ein neuer Shader.
 */
Shader* shader_createShader();

/**
 * Hängt eine GLSL Datei an einen bestehenden Shader an.
 * Der Code wird dabei auch sofort übersetzt und nur bei erfolg an den
 * Shader gehängt.
 * 
 * Bei Misserfolg gibt die Funktion eine Fehlermeldung aus.
 * 
 * @param shader der Shader an den die Datei angehängt werden soll.
 * @param type der Shadertyp der Datei.
 * @param file der Pfad zur Datei.
 * @return true, wenn die operation erfolgreich war, false wenn nicht.
 */
bool shader_attachShaderFile(Shader* shader, GLenum type, const char* file);

/**
 * Baut einen Shader zusammen (linken) nachdem mehrere Dateien an ihn
 * gehängt wurden.
 * 
 * Bei Misserfolg gibt die Funktion eine Fehlermeldung aus.
 * 
 * @param shader der Shader, der gebaut werden soll.
 * @return true, wenn die operation erfolgreich war, false wenn nicht.
 */
bool shader_buildShader(Shader* shader);

/**
 * Aktiviert einen Shader für die Benutzung.
 * Der Shader muss bereits gebaut worden sein.
 * 
 * @param shader der Shader, der genutzt werden soll.
 */
void shader_useShader(Shader* shader);

/**
 * Löscht einen bestehenden Shader und gibt alle Ressourcen wieder frei.
 * Dabei ist es egal, ob der Shader bereits gebaut wurde oder nicht.
 * Auch bereits angehängte Codes werden wieder freigegeben.
 * 
 * @param shader der Shader, der gelöscht werden soll.
 */
void shader_deleteShader(Shader* shader);

/**
 * Hilfsfunktion zum Anlegen eines Shaders, der aus einem Vertex- und
 * einem Fragmentshader besteht.
 * 
 * Bei Misserfolg gibt die Funktion eine Fehlermeldung aus.
 * 
 * @return ein Shader, der aus den übergebenen Dateien gebaut wurde oder NULL
 *         wenn etwas schief gegangen ist.
 */
Shader* shader_createVeFrShader(const char* label, const char* vert, const char* frag);

/**
 * Hilfsfunktion zum Anlegen eines Shaders, der aus einem Vertex- ,
 * Tessellation- und einem Fragmentshader besteht.
 *
 * Bei Misserfolg gibt die Funktion eine Fehlermeldung aus.
 *
 * @return ein Shader, der aus den übergebenen Dateien gebaut wurde oder NULL
 *         wenn etwas schief gegangen ist.
 */
Shader* shader_createVeTessFrShader(const char* label,
    const char* vert, const char* tesc,
    const char* tese, const char* frag);

/**
 * Übergibt eine 4x4 Matrix an einen Shader über eine Uniform-Variable.
 * Der Shader muss zuvor mit shader_useShader aktiviert worden sein!
 * 
 * @param shader der Shader, bei dem die Uniform Variable gesetzt werden soll
 * @param name der Name der Uniform Variable
 * @param mat die 4x4 Matrix
 */
void shader_setMat4(Shader* shader, char* name, mat4* mat);

/**
 * Übergibt einen 2D Vektor an einen Shader über eine Uniform-Variable.
 * Der Shader muss zuvor mit shader_useShader aktiviert worden sein!
 * 
 * @param shader der Shader, bei dem die Uniform Variable gesetzt werden soll
 * @param name der Name der Uniform Variable
 * @param vec2 der 2D Vektor
 */
void shader_setVec2(Shader* shader, char* name, vec2* vec2);

/**
 * Übergibt einen 3D Vektor an einen Shader über eine Uniform-Variable.
 * Der Shader muss zuvor mit shader_useShader aktiviert worden sein!
 * 
 * @param shader der Shader, bei dem die Uniform Variable gesetzt werden soll
 * @param name der Name der Uniform Variable
 * @param vec3 der 3D Vektor
 */
void shader_setVec3(Shader* shader, char* name, vec3* vec3);

/**
 * Übergibt einen Integer an einen Shader über eine Uniform-Variable.
 * Der Shader muss zuvor mit shader_useShader aktiviert worden sein!
 * 
 * @param shader der Shader, bei dem die Uniform Variable gesetzt werden soll
 * @param name der Name der Uniform Variable
 * @param val der zu setzende Wert
 */
void shader_setInt(Shader* shader, char* name, int val);

/**
 * Übergibt einen Float an einen Shader über eine Uniform-Variable.
 * Der Shader muss zuvor mit shader_useShader aktiviert worden sein!
 * 
 * @param shader der Shader, bei dem die Uniform Variable gesetzt werden soll
 * @param name der Name der Uniform Variable
 * @param val der zu setzende Wert
 */
void shader_setFloat(Shader* shader, char* name, float val);

/**
 * Übergibt einen Boolean an einen Shader über eine Uniform-Variable.
 * Der Shader muss zuvor mit shader_useShader aktiviert worden sein!
 * 
 * @param shader der Shader, bei dem die Uniform Variable gesetzt werden soll
 * @param name der Name der Uniform Variable
 * @param val der zu setzende Wert
 */
void shader_setBool(Shader* shader, char* name, bool val);

#endif // SHADER_H
