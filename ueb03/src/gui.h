/**
 * Graphisches Nutzerinterface für die Software.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

#ifndef GUI_H
#define GUI_H

#include "common.h"

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Initialisiert die GUI. 
 * Der Programmkontext MUSS ein Handle auf ein existierendes 
 * GLFW Fenster enthalten.
 * 
 * @param ctx Programmkontext.
 */
void gui_init(ProgContext* ctx);

/**
 * Rendert die GUI. 
 * Muss als letztes vor dem Double-Buffer Swap aufgerufen werden.
 * 
 * @param ctx Programmkontext.
 */
void gui_render(ProgContext* ctx);

/**
 * Gibt die Ressourcen der GUI wieder frei.
 * 
 * @param ctx Programmkontext.
 */
void gui_cleanup(ProgContext* ctx);

#endif // GUI_H
