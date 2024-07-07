/**
 * Wichtige Datentypen, Funktionen und Includes die
 * im gesammten Programm gebraucht werden.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

#include "common.h"

#include <stdlib.h>
#include <string.h>

#include "utils.h"

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

ProgContext* common_createContext(void)
{
    ProgContext* ctx = malloc(sizeof(ProgContext));

    ctx->winData = malloc(sizeof(WindowData));
    memset(ctx->winData, 0, sizeof(WindowData));

    ctx->window = NULL;
    ctx->input = NULL;
    ctx->rendering = NULL;
    ctx->gui = NULL;

    return ctx;
}

void common_deleteContext(ProgContext* ctx)
{
    free(ctx->winData);
    free(ctx);
}

void common_labelObjectByType(GLenum type, GLuint name, const char* label)
{
    // Erst prüfen, ob Funktion existiert.
    if (!GLAD_GL_KHR_debug)
    {
        return;
    }

    // Je nach Typ wird ein anderes Präfix gewählt.
    char* prefix = "??";
    switch (type)
    {
    case GL_BUFFER:
        prefix = "Buffer";
        break;
    case GL_SHADER:
        prefix = "Shader";
        break;
    case GL_PROGRAM:
        prefix = "Program";
        break;
    case GL_VERTEX_ARRAY:
        prefix = "VAO";
        break;
    case GL_TEXTURE:
        prefix = "Texture";
        break;
    case GL_RENDERBUFFER:
        prefix = "RBO";
        break;
    case GL_FRAMEBUFFER:
        prefix = "FBO";
        break;
    }

    // String zusammen bauen.
    #define MAX_LABEL 255
    char internalLabel[MAX_LABEL] = "";
    snprintf(internalLabel, MAX_LABEL, "%s: %s", prefix, label);
    #undef MAX_LABEL

    // Label setzen.
    glObjectLabel(type, name, -1, internalLabel);
}

void common_labelObjectByFilename(GLenum type, GLuint name, const char* filepath)
{
    // Erst prüfen, ob Funktion existiert.
    // Theoretisch mit common_labelObjectByType gedoppelt, aber dadurch
    // sparen wir uns die Suche nach dem Dateinamen.
    if (!GLAD_GL_KHR_debug)
    {
        return;
    }

    char* filename = utils_getFilename(filepath);
    common_labelObjectByType(type, name, filename);
    free(filename);
}

void common_pushRenderScopeSource(const char* scope, GLenum source)
{
    // Erst prüfen, ob Funktion existiert.
    if (!GLAD_GL_KHR_debug)
    {
        return;
    }

    glPushDebugGroup(source, 0, -1, scope);
}

void common_popRenderScope()
{
    // Erst prüfen, ob Funktion existiert.
    if (!GLAD_GL_KHR_debug)
    {
        return;
    }

    glPopDebugGroup();
}