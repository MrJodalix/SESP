/**
 * Modul zum Laden und Verwenden von Shadern.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

#include "shader.h"

#include <stdio.h>
#include <sesp/stb_ds.h>

#include "utils.h"

////////////////////////////// LOKALE DATENTYPEN ///////////////////////////////

// Implementierung der Datenstruktur, die einen Shader repräsentiert.
// Dadurch, dass das Struct erst hier vollständig definiert wird, sind die
// Eigenschaften eiens Shaders nur in dieser Datei sichtbar.
struct Shader
{
    GLuint id;
    bool linked;
    int fileCount;
    GLuint* shaderFiles;
    struct UniformHashmap {
        char* key;
        GLint value;
    } *uniforms;
};

////////////////////////////// LOKALE FUNKTIONEN ///////////////////////////////

/**
 * Hilfsfunktion zum Laden eines Shaders aus einer Datei.
 * Der Shader wird direkt kompiliert.
 * 
 * @param type die Art Shader, die erzeugt werden soll
 * @param file der Pfad zum Shader-Quellcode
 * @param success signalisiert, ob die erzeugung erfolgreich war
 * @return die ID des neu erzeugten Shaders
 */
static GLuint shader_createGLSLShader(GLenum type, const char* file, 
                                      bool* success)
{
    // Grundsätzlich gehen wir von einem Erfolg aus.
    *success = true;

    // Zuerst erstellen wir einen neuen, leeren Shader.
    GLuint shader = glCreateShader(type);

    // Danach laden wir den Quellcode des Shaders aus der 
    // angegebenen Datei. Dieser wird dem neuen Shader zugewiesen.
    const char* source = utils_readFile(file);
    glShaderSource(shader, 1, &source, NULL);

    // Als nächstes kann der Shader kompiliert werden.
    glCompileShader(shader);

    // Außerdem kann der Speicher für den Quellcode wieder freigegeben werden.
    free((void*) source);

    // Zum Schluss muss festgestellt werden, ob Fehler beim Kompilieren
    // aufgetreten sind.
    GLint successId;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &successId);
    if (!successId)
    {
        // Wenn es einen Fehler gab, geben wir eine Meldung auf der Konsole aus.
        // Dazu muss erst die Länge der Meldung abgerufen werden, bevor diese 
        // in den neu erstellten Buffer geladen werden kann.
        GLint logSize = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

        GLchar* buffer = (GLchar*) malloc(logSize);
        glGetShaderInfoLog(shader, logSize, &logSize, buffer);

        // Wir sollten immer den Dateinamen mit ausgeben, damit nachvollziehbar 
        // ist, was genau schief gegangen ist.
        fprintf(
            stderr, 
            "Error on shader compilation of file \"%s\":\n\t%s\n", 
            file, buffer
        );

        // Nach der Meldung geben wir die neu erstellten Ressourcen wieder frei.
        free(buffer);
        glDeleteShader(shader);

        // Den Misserfolg an die aufrufende Funktion melden.
        *success = false;
    }

    // Label setzen, damit der Shader in RenderDoc leichter erkennbar ist.
    common_labelObjectByFilename(GL_SHADER, shader, file);

    // Bei Erfolg geben wir die ID des neuen Shaders zurück.
    return shader;
}

/**
 * Hilfsfunktion zum Abrufen einer Uniform Location.
 * Im Hintergrund wird ein Cache verwendet, um die Zugriffe zu beschleunigen.
 * 
 * @param shader der Shader, in dem die Uniform Location gesucht werden soll
 * @param name der Name der Uniform Variable, dessen Location gesucht ist
 * @return die Uniform Location oder -1 wenn sie garnicht existiert
 */
static GLint shader_getUniformLocation(Shader* shader, const char* name)
{
    // Zuerst überprüfen wir, ob wir die Uniform Location bereits gecached
    // haben. Der Standardwert für nicht-gecachte Werte ist -2, da -1 bereits
    // für Uniforms benutzt wird, die nicht im Shader gefunden wurden.
    GLint location = stbds_shget(shader->uniforms, name);
    if (location < -1)
    {
        // Wenn wir keine Location gecached haben, rufen wir sie von OpenGL ab.
        location = glGetUniformLocation(shader->id, name);

        // Danach sichern wir sie auch gleich im Cache.
        stbds_shput(shader->uniforms, name, location);
    }

    return location;
}

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

Shader* shader_createShader()
{
    // Speicher allozieren und Shader initialisieren.
    Shader* shader = malloc(sizeof(Shader));
    shader->id = 0;
    shader->linked = false;
    shader->fileCount = 0;
    shader->shaderFiles = NULL;
    shader->uniforms = NULL;
    stbds_sh_new_arena(shader->uniforms);
    stbds_shdefault(shader->uniforms, -2);

    return shader;
}

bool shader_attachShaderFile(Shader* shader, GLenum type, const char* file)
{
    // Wenn der Shader bereits gelinkt wurde, darf keine neue Datei 
    // hinzugefügt werden.
    if (shader->linked)
    {
        fprintf(stderr, "Cannot attach a file to an already linked shader!\n");
        return false;
    }

    // Zuerst laden und kompilieren wir die angegebene Datei.
    bool success;
    GLuint glslShader = shader_createGLSLShader(type, file, &success);

    // Danach wird geprüft, ob die Operation erfolgreich war.
    if (success)
    {
        // Wenn ja fügen wir die neue Shader ID zum Array mit allen IDs hinzu.
        shader->fileCount++;
        shader->shaderFiles = realloc(
            shader->shaderFiles, 
            sizeof(GLuint) * shader->fileCount
        );
        shader->shaderFiles[shader->fileCount - 1] = glslShader;
    }

    return success;
}

bool shader_buildShader(Shader* shader)
{
    // Der Shader darf nicht bereits gelinkt sein.
    if (shader->linked)
    {
        fprintf(stderr, "Cannot build a shader that is already linked!\n");
        return false;
    }

    // Dem Shader muss mindestens eine Datei hinzugefügt worden sein.
    if (!shader->shaderFiles)
    {
        fprintf(stderr, "Cannot build a shader with no attached files!\n");
        return false;
    }

    // Benötigte Variablen und ein neues Shader-Programm anlegen.
    bool success = true;
    int i;
    GLuint newProgram = glCreateProgram();
    
    // Wir iterieren über alle Shader-Dateien und hängen sie an das neue
    // Programm mit an.
    for (i = 0; i < shader->fileCount; i++)
    {
        glAttachShader(newProgram, shader->shaderFiles[i]);
    }

    // Dannach kann das Programm gelinkt werden.
    glLinkProgram(newProgram);

    // Zum Schluss muss festgestellt werden, ob Fehler beim Linken
    // aufgetreten sind.
    GLint isLinked;
    glGetProgramiv(newProgram, GL_LINK_STATUS, &isLinked);
    if (!isLinked)
    {
        // Wenn es einen Fehler gab, geben wir eine Meldung auf der Konsole aus.
        // Dazu muss erst die Länge der Meldung abgerufen werden, bevor diese 
        // in den neu erstellten Buffer geladen werden kann.
        GLint logSize = 0;
        glGetProgramiv(newProgram, GL_INFO_LOG_LENGTH, &logSize);

        GLchar* buffer = (GLchar*) malloc(logSize);
        glGetProgramInfoLog(newProgram, logSize, &logSize, buffer);
        fprintf(stderr, "Error on shader linking:\n\t%s\n", buffer);

        // Nach der Meldung geben wir die neu erstellten Ressourcen wieder frei.
        free(buffer);
        glDeleteProgram(newProgram);

        // Der Fehlschlag wird dem Aufrufer mitgeteilt.
        success = false;
    }
    else
    {
        // Nach dem Linken sollten immer alle Shader 
        // vom Programm getrennt werden.
        // Außerdem werden bach dem Linken die einzelnen Bestandteile nicht mehr 
        // benötigt und müssen frei gegeben werden. Das Löschen wird jedoch 
        // solange aufgeschoben, bis die Shader von allen Programmen getrennt 
        // wurden.  Deswegen haben wir sie bereits im letzten Schritt getrennt.
        for (i = 0; i < shader->fileCount; i++)
        {
            glDetachShader(newProgram, shader->shaderFiles[i]);
            glDeleteShader(shader->shaderFiles[i]);
        }

        // Jetzt kann das Shaderobjekt vollständig gefüllt werden.
        shader->linked = true;
        shader->id = newProgram;
        free(shader->shaderFiles);
        shader->shaderFiles = NULL;
        shader->fileCount = 0;
    }

    return success;
}

void shader_useShader(Shader* shader)
{
    // Der Shader muss gelinkt sein, bevor er verwendet werden kann.
    if (!shader->linked)
    {
        fprintf(stderr, "Cannot use a shader that is not linked!\n");
        return;
    }
    // Zum Verwenden reicht ein einfacher Aufruf der folgenden Funktion:
    glUseProgram(shader->id);
}

void shader_deleteShader(Shader* shader)
{
    // Wenn kein Shader existiert muss nichts gelöscht werden.
    if (!shader)
    {
        return;
    }

    // Wenn der Shader gelinkt wurde, muss das Programm gelöscht werden.
    if (shader->linked)
    {
        glDeleteProgram(shader->id);
    }

    // Wenn noch Dateien angehängt sind, müssen diese gelöscht werden.
    if (shader->shaderFiles)
    {
        for (int i = 0; i < shader->fileCount; i++)
        {
            glDeleteShader(shader->shaderFiles[i]);
        }
        free(shader->shaderFiles);
    }

    // Uniform Hashmap freigeben.
    stbds_shfree(shader->uniforms);

    // Zum Schluss kann der Speicher wieder freigegeben werden.
    free(shader);
}

Shader* shader_createVeFrShader(const char* label, const char* vert, const char* frag)
{
    // Zuerst werden alle benötigten Bestandteile des Shaders angelegt,
    // egal ob einer Fehler verursacht.
    Shader* newShader = shader_createShader();
    bool vertOk = shader_attachShaderFile(newShader, GL_VERTEX_SHADER, vert);
    bool fragOk = shader_attachShaderFile(newShader, GL_FRAGMENT_SHADER, frag);

    // Danach wird auf mögliche Fehler geprüft.
    if (vertOk && fragOk)
    {
        // Wenn keine Fehler aufgetreten sind, kann der Shader gebaut werden.
        if (shader_buildShader(newShader))
        {
            // Wenn dies erfolgreich war, geben wir dem neuen Shader ein Label
            // und geben dann die ID zurück.
            common_labelObjectByType(GL_PROGRAM, newShader->id, label);
            return newShader;
        }
    }

    // Sollte ein Problem aufgetreten sein, wird der Shader wieder gelöscht und
    // NULL zurückgegeben.
    shader_deleteShader(newShader);
    return NULL;
}

Shader* shader_createVeTessFrShader(const char* label,
    const char* vert, const char* tesc,
    const char* tese, const char* frag)
{
    // Zuerst werden alle benötigten Bestandteile des Shaders angelegt,
    // egal ob einer Fehler verursacht.
    Shader* newShader = shader_createShader();
    bool vertOk = shader_attachShaderFile(newShader, GL_VERTEX_SHADER, vert);
    bool tescOk = shader_attachShaderFile(newShader, GL_TESS_CONTROL_SHADER, tesc);
    bool teseOk = shader_attachShaderFile(newShader, GL_TESS_EVALUATION_SHADER, tese);
    bool fragOk = shader_attachShaderFile(newShader, GL_FRAGMENT_SHADER, frag);

    // Danach wird auf mögliche Fehler geprüft.
    if (vertOk && tescOk && teseOk && fragOk)
    {
        // Wenn keine Fehler aufgetreten sind, kann der Shader gebaut werden.
        if (shader_buildShader(newShader))
        {
            // Wenn dies erfolgreich war, geben wir dem neuen Shader ein Label
            // und geben dann die ID zurück.
            common_labelObjectByType(GL_PROGRAM, newShader->id, label);
            return newShader;
        }
    }

    // Sollte ein Problem aufgetreten sein, wird der Shader wieder gelöscht und
    // NULL zurückgegeben.
    shader_deleteShader(newShader);
    return NULL;
}

void shader_setMat4(Shader* shader, char* name, mat4* mat)
{
    GLint location = shader_getUniformLocation(shader, name);
    glUniformMatrix4fv(location, 1, GL_FALSE, (float*) mat);
}

void shader_setVec2(Shader* shader, char* name, vec2* vec2)
{
    GLint location = shader_getUniformLocation(shader, name);
    glUniform2fv(location, 1, (float*) vec2);
}

void shader_setVec3(Shader* shader, char* name, vec3* vec3)
{
    GLint location = shader_getUniformLocation(shader, name);
    glUniform3fv(location, 1, (float*) vec3);
}

void shader_setInt(Shader* shader, char* name, int val)
{
    GLint location = shader_getUniformLocation(shader, name);
    glUniform1i(location, val);
}

void shader_setFloat(Shader* shader, char* name, float val)
{
    GLint location = shader_getUniformLocation(shader, name);
    glUniform1f(location, val);
}

void shader_setBool(Shader* shader, char* name, bool val)
{
    GLint location = shader_getUniformLocation(shader, name);
    glUniform1i(location, val);
}


