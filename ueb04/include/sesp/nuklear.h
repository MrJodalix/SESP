/**
 * Diese Datei bindet eine Single-Header-Library (SHL) [1] ein und setzt dabei alle
 * nötigen Flags.
 * 
 * SHL: nuklear
 * 
 * DIE VERWENDUNG VON ZUSÄTZLICHEN BIBLIOTHEKEN MUSS IM VORRAUS ABGESPROCHEN WERDEN.
 * 
 * [1]: https://nicolashollmann.de/de/blog/single-header-libraries/
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

#ifndef SESP_NUKLEAR_H
#define SESP_NUKLEAR_H

// Nuklear benötigt Zugriff auf einige der GLFW
// und OpenGL Funktionen:
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Nuklear Flags setzen, die überall gebraucht werden:
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_KEYSTATE_BASED_INPUT

// Hier wird Nukler abschließend eingebunden:
#include <nuklear/nuklear.h>
#include <nuklear/nuklear_glfw_gl3.h>

#endif // SESP_NUKLEAR_H
