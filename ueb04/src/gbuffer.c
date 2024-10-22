/**
 * Modul für das Verwalten eines Geometry-Buffers (kurz GBuffer).
 * 
 * Copyright (C) 2023, FH Wedel
 * Autor: Joshua-Scott Schöttke, Ilana Schmara
 */

#include "gbuffer.h"

#include <string.h>

////////////////////////////// LOKALE DATENTYPEN ///////////////////////////////

// GBuffer Datentyp.
struct GBuffer
{
    // Frame-Buffer-Object => FBO
    GLuint fbo;
    GLuint textures[GBUFFER_NUM_COLORATTACH];
    GLuint rboDepth;
};

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

GBuffer* gbuffer_createGBuffer(int width, int height)
{
    // Wie immer muss zuerst der Speicher für den GBuffer angefordert werden.
    GBuffer* gbuffer = malloc(sizeof(GBuffer));
    memset(gbuffer, 0, sizeof(GBuffer));

    // Dann erstellen wir unser FBO (Framebuffer Object) und binden es direkt.
    glGenFramebuffers(1, &gbuffer->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, gbuffer->fbo);

    //Exposure Tone Mapping

    // Texturen anlegen
    glGenTextures(sizeof(gbuffer->textures) / sizeof(gbuffer->textures[0]), gbuffer->textures);

    glGenTextures(1, &gbuffer->textures[GBUFFER_COLORATTACH_FINAL]); // allocate output (final) texture

    for (unsigned int i = 0; i < GBUFFER_NUM_COLORATTACH; i++) {
        // creates the storage area of the texture (without initializing it)
        glBindTexture(GL_TEXTURE_2D, gbuffer->textures[i]);
        //if texture id is ALBEDOSPEC using RGBA
        glTexImage2D(GL_TEXTURE_2D, 0, (i == GBUFFER_COLORATTACH_ALBEDOSPEC) ? GL_RGBA32F : GL_RGB32F, width, height, 0, (i == GBUFFER_COLORATTACH_ALBEDOSPEC) ? GL_RGBA : GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // prevents unnecessary interpolation between the texels that might create some fine distortions
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // attaches the texture to the FBO as a target
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, gbuffer->textures[i], 0);
        switch (i)
        {
        case GBUFFER_COLORATTACH_POSITION:
            common_labelObjectByType(GL_TEXTURE, gbuffer->textures[i], "Position");
            break;
        case GBUFFER_COLORATTACH_NORMAL:
            common_labelObjectByType(GL_TEXTURE, gbuffer->textures[i], "Normal");
            break;
        case GBUFFER_COLORATTACH_ALBEDOSPEC:
            common_labelObjectByType(GL_TEXTURE, gbuffer->textures[i], "AlbedoSpec");
            break;
        case GBUFFER_COLORATTACH_EMISSION:
            common_labelObjectByType(GL_TEXTURE, gbuffer->textures[i], "Emission");
            break;
        case GBUFFER_COLORATTACH_TEXCOORD:
            common_labelObjectByType(GL_TEXTURE, gbuffer->textures[i], "TexCoords");
            break;
        }
    
    }
    // --- Tiefenbuffer ---

    // Textur für Depth & Stencil Buffer
    glGenRenderbuffers(1, &gbuffer->rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, gbuffer->rboDepth);
    common_labelObjectByType(GL_RENDERBUFFER, gbuffer->rboDepth, "GBuffer Depth");

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gbuffer->rboDepth);


    // --- Finales Ausgabebild ---
    glBindTexture(GL_TEXTURE_2D, gbuffer->textures[GBUFFER_COLORATTACH_FINAL]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    common_labelObjectByType(GL_TEXTURE, gbuffer->textures[GBUFFER_COLORATTACH_FINAL], "Final");

    // Die Textur an das FBO binden.
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, 
        GL_COLOR_ATTACHMENT0 + GBUFFER_COLORATTACH_FINAL, 
        GL_TEXTURE_2D, 
        gbuffer->textures[GBUFFER_COLORATTACH_FINAL], 
        0
    );

    // explicitly tell OpenGl which color attachments to be used (of. FB)
    // enable writing to all six textures
    GLenum drawBuffer[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
    glDrawBuffers(sizeof(drawBuffer) / sizeof(drawBuffer[0]), drawBuffer); // supplying array if attachment locations 


    // Wir prüfen dann noch, ob das FBO erfolgreich angelegt wurde.
    GLenum fboState = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboState != GL_FRAMEBUFFER_COMPLETE)
    {
        fprintf(stderr, "Error: FBO not complete because:\n");

        // Wir geben auch den Grund für den Fehlschlag aus. Hier werden jedoch nur
        // die häufigsten Fälle abgefragt. Für mehr Informationen, oder wenn ihr
        // "Unkown" erhaltet, schaut euch die Dokumentation unter 
        // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glCheckFramebufferStatus.xhtml
        // an oder fragt nach.
        switch (fboState)
        {
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                fprintf(stderr, "\tIncomplete Attachment.\n");
                break;
            
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                fprintf(stderr, "\tMissing Attachments.\n");
                break;
            
            case GL_FRAMEBUFFER_UNSUPPORTED:
                fprintf(stderr, "\tFramebuffer Unsupported.\n");
                break;
        
            default:
                fprintf(stderr, "\tUnkown. Please check gbuffer.c\n");
                break;
        }
    }

    // Wir geben dem Framebuffer noch einen Namen.
    common_labelObjectByType(GL_FRAMEBUFFER, gbuffer->fbo, "GBuffer");

    // Zum Schluss wechseln wir zurück zum Standard FBO und 
    // geben den GBuffer zurück.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return gbuffer;
}

void gbuffer_clearFinalTexture(GBuffer *gbuffer)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gbuffer->fbo);
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + GBUFFER_COLORATTACH_FINAL);
    glClear(GL_COLOR_BUFFER_BIT);
}

void gbuffer_bindGBufferForGeomPass(GBuffer *gbuffer)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gbuffer->fbo);

    // Die zu verwendenden Attribute konfigurieren.
    GLenum drawBuffer[] = {
        GL_COLOR_ATTACHMENT0 + GBUFFER_COLORATTACH_POSITION, // location 0
        GL_COLOR_ATTACHMENT0 + GBUFFER_COLORATTACH_NORMAL, // location 1
        GL_COLOR_ATTACHMENT0 + GBUFFER_COLORATTACH_ALBEDOSPEC, // location 2
        GL_COLOR_ATTACHMENT0 + GBUFFER_COLORATTACH_EMISSION, // location 3
        GL_COLOR_ATTACHMENT0 + GBUFFER_COLORATTACH_TEXCOORD, // location 4
        GL_COLOR_ATTACHMENT0 + GBUFFER_COLORATTACH_FINAL, // location 5
    };

    // Danach übergeben wir das Array mit den Color-Attachments.
    // ACHTUNG: Die Reihenfolge im Array gibt die Positionen im Shader an.
    glDrawBuffers(sizeof(drawBuffer) / sizeof(drawBuffer[0]), drawBuffer);
}

void gbuffer_bindGBufferForStencilPass(GBuffer* gbuffer)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gbuffer->fbo);
    glDrawBuffer(GL_NONE);
}

void gbuffer_bindGBufferForLightPass(GBuffer *gbuffer)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gbuffer->fbo);
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + GBUFFER_COLORATTACH_FINAL);
    for (unsigned int i = 0; i < sizeof(gbuffer->textures) / sizeof(gbuffer->textures[0]); i++) {
        glActiveTexture(GL_TEXTURE0 + i); 
        glBindTexture(GL_TEXTURE_2D, gbuffer->textures[GBUFFER_COLORATTACH_POSITION + i]);
    }
}

void gbuffer_bindGBufferForTextureRead(GBUFFER_TEXTURE_TYPE textureType)
{
    // Auswahl von welchem Attachment gelesen werden soll.
    glReadBuffer(GL_COLOR_ATTACHMENT0 + textureType); 
}

void gbuffer_bindGBufferForReadingTextures()
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); 
    for (unsigned int i = 0; i < GBUFFER_NUM_COLORATTACH; i++) {
        glActiveTexture(GL_TEXTURE0 + i); 
        glBindTexture(GL_TEXTURE_2D, GL_COLOR_ATTACHMENT0 + i);
    }
}


void gbuffer_bindGBufferForFinalPass(GBuffer* gbuffer)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gbuffer->fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0 + GBUFFER_COLORATTACH_FINAL);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void gbuffer_bindForReading(GBuffer* gbuffer) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gbuffer->fbo);
};

void gbuffer_deleteGBuffer(GBuffer *gbuffer)
{
    // FBO löschen.
    glDeleteFramebuffers(1, &gbuffer->fbo);

    // Die angehängten Texturen löschen.
    glDeleteTextures(1, &gbuffer->textures[GBUFFER_COLORATTACH_POSITION]);
    glDeleteTextures(1, &gbuffer->textures[GBUFFER_COLORATTACH_NORMAL]);
    glDeleteTextures(1, &gbuffer->textures[GBUFFER_COLORATTACH_ALBEDOSPEC]);
    glDeleteTextures(1, &gbuffer->textures[GBUFFER_COLORATTACH_EMISSION]);
    glDeleteTextures(1, &gbuffer->textures[GBUFFER_COLORATTACH_TEXCOORD]);
    glDeleteTextures(1, &gbuffer->textures[GBUFFER_COLORATTACH_FINAL]);
    glDeleteRenderbuffers(1, &gbuffer->rboDepth);

    free(gbuffer);
}
