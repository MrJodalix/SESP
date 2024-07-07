/**
 * Modul für das Verwalten eines Geometry-Buffers (kurz GBuffer).
 * 
 * Dieses Modul dient nur der Orientierung und sollte 
 * umfangreich verändert werden. Viele Stellen sind mit einem TODO markiert,
 * diese müssen von euch gefüllt werden. Grundsätzlich dürft ihr auch die Struktur
 * komplett über Bord werfen und euer eigenes Modul aufsetzen.
 * Wenn ihr dieses Modul als Grundlage nehmt, denkt daran alle TODOs und diesen
 * Kommentar zu entfernen. Passt auch den Autor an!
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Jonas Sorgenfrei, Nicolas Hollmann
 */

#ifndef GBUFFER_H
#define GBUFFER_H

#include "common.h"

//////////////////////////// ÖFFENTLICHE DATENTYPEN ////////////////////////////

// Aufzählungstyp für die unterschiedlichen Color Attachments des GBuffers.
typedef enum {
    GBUFFER_COLORATTACH_POSITION,
    GBUFFER_COLORATTACH_NORMAL,
    GBUFFER_COLORATTACH_ALBEDOSPEC,
    GBUFFER_COLORATTACH_EMISSION,
    GBUFFER_COLORATTACH_TEXCOORD, 
    GBUFFER_COLORATTACH_FINAL,

    GBUFFER_NUM_COLORATTACH
} GBUFFER_TEXTURE_TYPE;

// GBuffer Datentyp.
struct GBuffer;
typedef struct GBuffer GBuffer;

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Erstellt einen neuen GBuffer mit der angegebenen Größe.
 * 
 * @param width die Breite des Buffers
 * @param height die Höhe des Buffers
 * @return der neue GBuffer
 */
GBuffer *gbuffer_createGBuffer(int width, int height);

/**
 * Bindet das GBuffer FBO, setzt die finale Render Ausgabe als DrawBuffer und 
 * cleart den Color-Buffer dieses Attachments.
 *
 * @param gbuffer der GBuffer
 */
void gbuffer_clearFinalTexture(GBuffer *gbuffer);

/**
 * Bindet das GBuffer FBO und setzt die entsprechenden Color_Attachments für 
 * das den Geometry Pass Render Vorgang. Der Farb und Tiefenbuffer wird geleert.
 *
 * @param gbuffer der GBuffer
 */
void gbuffer_bindGBufferForGeomPass(GBuffer *gbuffer);

/**
 * Bindet das GBuffer FBO und deaktiviert das schreiben auf einen
 * Draw-Buffer
 *
 * @param gbuffer der GBuffer
 */
void gbuffer_bindGBufferForStencilPass(GBuffer *gbuffer);

/**
 * Bindet das GBuffer FBO, setzt die finale Render Ausgabe als DrawBuffer und 
 * bindet die GBuffer Texturen als Texturen
 *
 * @param gbuffer der GBuffer
 */
void gbuffer_bindGBufferForLightPass(GBuffer *gbuffer);

/**
 * Setzt die Textur zum Lesen aus dem Framebuffer
 *
 * @param textureType welches Color Attachment fürs Lesen gebunden werden soll
 */
void gbuffer_bindGBufferForTextureRead(GBUFFER_TEXTURE_TYPE textureType);

/**
 * Setzt alle Texturen zum Lesen aus dem framebuffer
 */
void gbuffer_bindGBufferForReadingTextures();

/**
 * Bindet das default FBO als Draw FB und das GBuffer FBO als Read FB und 
 * bindet die final render Textur als ReadBuffer
 *
 * @param gbuffer der GBuffer
 */
void gbuffer_bindGBufferForFinalPass(GBuffer *gbuffer);

/**
 * Bindet das FBOs als Eingabe, sodass der Inhalt gezeigt werden kann
 * 
 * @param gbuffer der GBuffer
 */
void gbuffer_bindForReading(GBuffer* gbuffer);

/**
 * Löscht den übergebenen GBuffer wieder.
 * 
 * @param gbuffer der zu löschende GBuffer
 */
void gbuffer_deleteGBuffer(GBuffer* gbuffer);

#endif // GBUFFER_H
