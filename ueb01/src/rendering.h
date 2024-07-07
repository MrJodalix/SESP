/**
 * Modul zum Rendern der 3D Szene.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

#ifndef RENDERING_H
#define RENDERING_H

#include "common.h"

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Initialisiert das Rendering-Modul. 
 * 
 * @param ctx Programmkontext.
 */
void rendering_init(ProgContext* ctx);

/**
 * Rendert die 3D Szene. 
 * 
 * @param ctx Programmkontext.
 */
void rendering_draw(ProgContext* ctx);

/**
 * Gibt die Ressourcen des Rendering-Moduls wieder frei.
 * 
 * @param ctx Programmkontext.
 */
void rendering_cleanup(ProgContext* ctx);

#endif // RENDERING_H
