/**
 * Modul zum Erstellen und Verwalten des Hauptfensters.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

#ifndef WINDOW_H
#define WINDOW_H

#include "common.h"

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Erzeugt ein neues Fenster und initialisiert das gesamte Rendering-System.
 * 
 * @param title der Titel des Fensters.
 * @return ein neuer Programmkontext, der zu dem Fenster gehört.
 */
ProgContext* window_init(const char* title);

/**
 * Startet die Hauptschleife des Fensters.
 * Diese Funktion wird erst verlassen, wenn das Fenster geschlossen wird.
 * 
 * @param ctx Programmkontext.
 */
void window_mainloop(ProgContext* ctx);

/**
 * Aktualisiert die Fullscreen-Einstellung basierend auf dem Wert
 * von ctx->input->isFullscreen.
 * 
 * @param ctx Programmkontext.
 */
void window_updateFullscreen(ProgContext* ctx);

/**
 * Gibt alle Ressourcen, die durch das Fenster und dem dahinter
 * liegendem System belegt wurden, wieder frei.
 * 
 * @param ctx Programmkontext.
 */
void window_cleanup(ProgContext* ctx);

#endif // WINDOW_H
