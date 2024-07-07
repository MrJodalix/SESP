/**
 * Modul zum Abfragen von Benutzereingaben.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

#ifndef INPUT_H
#define INPUT_H

#include "common.h"

#include "model.h"
#include "camera.h"

//////////////////////////// ÖFFENTLICHE DATENTYPEN ////////////////////////////

// Datenstruktur, die die Zustände des Programms enthält,
// die durch Benutzereingaben direkt beeinfluss werden können.
struct InputData
{
    bool isFullscreen;
    bool showWireframe;
    bool showHelp;
    bool showMenu;
    bool showStats;
    bool reloadShader;
    int shaderChoice;

    bool showFog;
    float density;

    struct {
        vec4 clearColor;
        vec4 lightComp;
        vec4 lightColor;
        vec4 lightDir;
        Model* userModel;
        vec3 translate;
        vec3 rotate;
        vec3 scale;
    } rendering;

    Camera* mainCamera;
    double mouseLastX;
    double mouseLastY;
    bool mouseLooking;
};
typedef struct InputData InputData;

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Initialisiert das Input-Modul. 
 * 
 * @param ctx Programmkontext.
 */
void input_init(ProgContext* ctx);

/**
 * Verarbeitet Benutzereingaben in jedem Frame.
 * Zum Auslesen wird "polling" verwendet, also jeden Frame geprüft,
 * ob z.B. eine Taste gedrückt wurde. Dieses Vorgehen eignet sich
 * gut für Tasten, die länger gedrückt werden sollen. Zum Beispiel
 * zum Steuern der Kamera.
 * 
 * @param ctx Programmkontext.
 */
void input_process(ProgContext* ctx);

/**
 * Verarbeitet Benutzereingaben wenn diese auftreten.
 * Diese Funktion soll nur aufgerufen werden, wenn auch ein Event
 * eingetreten ist. Dieses Vorgehen eignet sich gut für Tasten,
 * die einen Status direkt verändern, zum Beispiel zum Umschalten des
 * Wireframe-Modus.
 * 
 * @param ctx Programmkontext.
 * @param key die gedrückte Taste.
 * @param action Grund für das Event (GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT).
 * @param mods Modifikatoren, zum Beispiel STRG oder ALT.
 */
void input_event(ProgContext* ctx, int key, int action, int mods);

/**
 * Diese Funktion verarbeitet Mausbewegungen.
 * 
 * @param ctx Programmkontext.
 * @param x die X Position der Maus.
 * @param y die Y Position der Maus.
 */
void input_mouseMove(ProgContext* ctx, double x, double y);

/**
 * Verarbeitung von Maus-Klicks.
 * 
 * @param ctx Programmkontext.
 * @param button die gedrückte Taste.
 * @param action die Aktion, die das Event ausgelöst hat (z.B. loslassen).
 * @param mods aktivierte Modifikatoren.
 */
void input_mouseAction(ProgContext* ctx, int button, int action, int mods);

/**
 * Diese Funktion verarbeitet Scroll-Events.
 * 
 * @param ctx Programmkontext.
 * @param xoff die Veränderung in X-Richtung (z.B. bei Touchpads).
 * @param yoff die Veränderung in Y-Richtung (z.B. klassisches Mausrad).
 */
void input_scroll(ProgContext* ctx, double xoff, double yoff);

/**
 * Verarbeitet die Nutzeranweisung, dass eine bestimmte Datei geladen werden
 * soll.
 * 
 * @param ctx Programmkontext.
 * @param path der Pfad zur zu ladenden Datei.
 */
void input_userSelectedFile(ProgContext* ctx, const char* path);

/**
 * Gibt die Ressourcen des Input-Moduls wieder frei.
 * 
 * @param ctx Programmkontext.
 */
void input_cleanup(ProgContext* ctx);

#endif // INPUT_H
