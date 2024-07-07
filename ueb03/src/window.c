/**
 * Modul zum Erstellen und Verwalten des Hauptfensters.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

#include "window.h"

#include <stdio.h>

#include "rendering.h"
#include "gui.h"
#include "input.h"
#include "utils.h"
#include "texture.h"

////////////////////////////////// KONSTANTEN //////////////////////////////////

#define SESP_OPENGL_MAJOR 4
#define SESP_OPENGL_MINOR 1

#define DEFAULT_WINDOW_WIDTH 1200
#define DEFAULT_WINDOW_HEIGHT 800

/////////////////////////////// LOKALE CALLBACKS ///////////////////////////////

/**
 * Callback Funktion für Fehler, die von GLFW erzeugt werden.
 * 
 * @param errorId die interne Fehlernummer
 * @param msg die für Menschen lesbare Fehlermeldung
 */
static void callback_glfwError(int errorId, const char* msg)
{
    fprintf(stderr, "GLFW Error %d: %s\n", errorId, msg);
}

/**
 * Callback Funktion, die nach der GLFW Initialiserung immer dann
 * aufgerufen wird, bevor das Programm beendet wird.
 * Dadurch wird sichergestellt, das glfwTerminate aufgerufen wird.
 */
static void callback_exit(void)
{
    glfwTerminate();
    printf("Program exited.\n");
}

/**
 * Callback Funktion, die bei einer Größenveränderung des Framebuffers 
 * aufgerufen wird.
 * 
 * @param window ein GLFW Fensterhandle.
 * @param width die neue Breite des Framebuffers.
 * @param height die neue Höhe des Framebuffers.
 */
static void callback_framebufferSize(GLFWwindow* window, int width, int height)
{
    ProgContext* context = (ProgContext*) glfwGetWindowUserPointer(window);
    context->winData->width = width;
    context->winData->height = height;
    glViewport(0, 0, width, height);
}

/**
 * Callback Funktion, die bei einer Größenveränderung des Fensters
 * aufgerufen wird.
 * 
 * @param window ein GLFW Fensterhandle.
 * @param width die neue Breite des Fensters.
 * @param height die neue Höhe des Fensters.
 */
static void callback_windowSize(GLFWwindow* window, int width, int height)
{
    ProgContext* context = (ProgContext*) glfwGetWindowUserPointer(window);
    context->winData->realWidth = width;
    context->winData->realHeight = height;
}

/**
 * Callback Funktion, die bei einem Tastaturevent aufgerufen wird.
 * Sie gibt das Event an das Input-Modul weiter.
 * 
 * @param window ein GLFW Fensterhandle.
 * @param key die gedrückte Taste.
 * @param scancode der Scancode der gedrückten Taste.
 * @param action Grund für das Event (GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT).
 * @param mods Modifikatoren, zum Beispiel STRG oder ALT.
 */
static void callback_keyboardEvent(GLFWwindow* window, int key, 
                                   int scancode, int action, int mods)
{
    // Der Scancode ist Gerätespezifisch und eignet sich für Tastatur-Mappings.
    // Allerdings brauchen wir ihn in diesem Programm nicht, deshalb ignorieren
    // wir ihn hier explizit.
    (void) scancode; 

    // Event and Input-Modul weiterleiten.
    ProgContext* context = (ProgContext*) glfwGetWindowUserPointer(window);
    input_event(context, key, action, mods);
}

/**
 * Callback Funktion, die bei einer Mausbewegung aufgerufen wird.
 * Sie gibt das Event an das Input-Modul weiter.
 * 
 * @param window ein GLFW Fensterhandle.
 * @param x die X Position der Maus.
 * @param y die Y Position der Maus.
 */
static void callback_mouseMovement(GLFWwindow* window, double x, double y)
{
    // Event and Input-Modul weiterleiten.
    ProgContext* context = (ProgContext*) glfwGetWindowUserPointer(window);
    input_mouseMove(context, x, y);
}

/**
 * Callback Funktion, die bei einem Drag & Drop Event aufgerufen wird.
 * Sie gibt das Event an das Input Modul weiter.
 * 
 * @param window ein GLFW Fensterhandle.
 * @param count die Anzahl der Dateien.
 * @param paths die Dateipfade.
 */
static void callback_dropPath(GLFWwindow* window, int count, const char** paths)
{
    // Wir beachten immer nur die erste Datei.
    if (count > 0)
    {
        // Event and Input-Modul weiterleiten.
        ProgContext* context = (ProgContext*) glfwGetWindowUserPointer(window);
        input_userSelectedFile(context, paths[0]);
    }
}

////////////////////////////// LOKALE FUNKTIONEN ///////////////////////////////

/**
 * Initialisiert GLFW und richtet ein Exit-Callback ein.
 */
static void window_initGlfw(void)
{
    // Handler für Fehlermeldungen möglichst früh setzen.
    glfwSetErrorCallback(callback_glfwError);

    // GLFW initialisieren
    if (!glfwInit())
    {
        fprintf(stderr, "Error: GLFW initialization failed!");
        exit(EXIT_FAILURE);
    }

    // Das Exit-Callback festlegen, um sicherzustellen, das GLFW immer korrekt
    // deinitialisiert wird.
    atexit(callback_exit);
}

/**
 * Erzeugt ein neues Fenster und konfiguriert es.
 * Das neue Fenster wird dabei dem übergebenen Programmkontext
 * hinzugefügt.
 * 
 * @param ctx Programmkontext.
 * @param title der Titel des neuen Fensters.
 */
static void window_createWindow(ProgContext* ctx, const char* title)
{
    // Zusätzliche Einstellungen für die Fenstererzeugung an GLFW geben.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, SESP_OPENGL_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, SESP_OPENGL_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef __APPLE__
        // Diese Einstellung ist unter macOS notwenig und erlaubt
        // es alte Features zu verwenden. Wenn ihr unter macOS arbeitet
        // müsst ihr also besonders darauf auspassen, keine veralteten
        // Features zu nutzen.
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Hier wird ein neues Fenster mit GLFW erzeugt.
    ctx->window = glfwCreateWindow(
        DEFAULT_WINDOW_WIDTH, 
        DEFAULT_WINDOW_HEIGHT, 
        title, NULL, NULL);
    if (!ctx->window)
    {
        exit(EXIT_FAILURE);
    }

    // Hier setzen wir den UserPointer des Fensters auf den neuen Kontext.
    // Dadurch können in Callbacks auf den Kontext zugegriffen werden.
    glfwSetWindowUserPointer(ctx->window, ctx);

    // Um OpenGL verwenden zu können muss zuerst der OpenGL Kontext
    // des Fensters aktiviert werden.
    glfwMakeContextCurrent(ctx->window);

    // Als nächstes muss GLAD die OpenGL Funktionsaufrufe mit den 
    // Implementierungen verknüpfen.
    // Dazu nutzen wir die von GLFW bereitgestellte Lade-Funktion.
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
	{
        exit(EXIT_FAILURE);
    }

    // Callback für die Veränderung der Framebuffergröße setzen.
    glfwSetFramebufferSizeCallback(ctx->window, callback_framebufferSize);

    // Callback für die Veränderung der Fenstergröße setzen.
    glfwSetWindowSizeCallback(ctx->window, callback_windowSize);

    // Callback für Tastatureingaben.
    glfwSetKeyCallback(ctx->window, callback_keyboardEvent);

    // Callback für Mausbewegungen.
    glfwSetCursorPosCallback(ctx->window, callback_mouseMovement);

    // Callback für Drag & Drop.
    glfwSetDropCallback(ctx->window, callback_dropPath);
}

/**
 * Gibt den GLFW Monitor zurück, auf dem das Fenster gerade am meisten zu sehen
 * ist.
 * 
 * Basierend auf: https://stackoverflow.com/a/31526753
 * 
 * @param win das Fenster, dessen Monitor bestimmt werden soll.
 * @return der gefundene Monitor.
 */
static GLFWmonitor* window_getCurrentMonitor(GLFWwindow *win)
{
    // Alle benötigten Variablen anlegen.
    int winX, winY, winW, winH;
    int bestOverlap = 0;

    int numMonitors;
    GLFWmonitor* bestMonitor = NULL;
    GLFWmonitor** allMonitors;

    // Die Abmessungen des Fensters auslesen.
    glfwGetWindowPos(win, &winX, &winY);
    glfwGetWindowSize(win, &winW, &winH);

    // Danach benötigen wir eine Liste aller Monitore.
    allMonitors = glfwGetMonitors(&numMonitors);

    // Über diese Liste iterieren wir hier.
    for (int i = 0; i < numMonitors; i++)
    {
        // Eigenschaften des Monitors abrufen.
        const GLFWvidmode* mode = glfwGetVideoMode(allMonitors[i]);
        int monW = mode->width;
        int monH = mode->height;
        int monX, monY;
        glfwGetMonitorPos(allMonitors[i], &monX, &monY);

        // Die jeweils kleinere Kante vom Fenster bzw. Monitor bestimmen.
        int rightEdge = utils_minInt(winX + winW, monX + monW);
        int bottomEdge = utils_minInt(winY + winH, monY + monH);

        // Die Überlappung des Fensters mit dem Monitor berechnen.
        int overlap = (
            utils_maxInt(0, rightEdge - utils_maxInt(winX, monX)) *
            utils_maxInt(0, bottomEdge - utils_maxInt(winY, monY))
        );
        
        // Wenn der aktuelle Monitor mehr Pixel des Fensters enthält, als alle
        // zuvor, dann speichern wir ihn als den Besten ab.
        if (bestOverlap < overlap)
        {
            bestOverlap = overlap;
            bestMonitor = allMonitors[i];
        }
    }

    // Den Monitor mit der größten Überlappung geben wir zurück.
    return bestMonitor;
}

/**
 * Initialisiert den FPS Timer.
 * 
 * @param ctx der Kontext, dessen Timer initialisiert werden soll
 */
static void window_initFpsTimer(ProgContext* ctx)
{
    WindowData* win = ctx->winData;
    win->lastFPSUpdateTime = glfwGetTime();
    win->frameCounter = 0;
    win->fps = 0;
    win->deltaTime = 0.0f;
    win->lastFrameTime = glfwGetTime();
}

/**
 * Aktualisiert den FPS Timer.
 * Nur einmal pro Sekunde wird das Update auch tatsächlich übernommen.
 * Das liegt daran, dass die Frames pro Sekunde gezählt werden.
 * 
 * @param ctx der Kontext, dessen Timer aktualisiert werden soll
 */
static void window_updateFpsTimer(ProgContext* ctx)
{
    WindowData* win = ctx->winData;

    // DeltaTime aktualisieren.
    double currentTime = glfwGetTime();
    win->deltaTime = currentTime - win->lastFrameTime;
    win->lastFrameTime = currentTime;

    // Wir zählen jeden Frame mit.
    win->frameCounter++;

    // Einmal pro Sekunde aktualisieren wir dann den Endwert.
    if (currentTime - win->lastFPSUpdateTime >= 1.0)
    {
        win->fps = win->frameCounter;
        win->frameCounter = 0;
        win->lastFPSUpdateTime += 1.0;
    }
}

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

ProgContext* window_init(const char* title)
{
    // Zuerst GLFW initialisieren.
    window_initGlfw();

    // Danach erzeugen wir unseren Programmkontext, in dem
    // alle relevanten Daten gespeichert werden.
    ProgContext* ctx = common_createContext();

    // Anschließend erzeugen wir unser Programmfenster.
    window_createWindow(ctx, title);

    // Wir geben einmal am Anfang des Programmes aus, welche OpenGL Version
    // tatsächlich geladen werden konnte. Damit könnt ihr für euch Überprüfen, 
    // ob soweit alles stimmt. 
    // Eine höhere Version als angefragt ist grundsätzlich ok.
    // Diese Ausgabe ist eine Allgemeine Anforderung von SESP.
    printf("OpenGL-Version: %s\n", glGetString(GL_VERSION));

    // Einmal zu Begin die Größe des Framebuffers bestimmen und setzen.
    // Bei Veränderungen wird das Callback aufgerufen.
    glfwGetFramebufferSize(
        ctx->window, 
        &ctx->winData->width, 
        &ctx->winData->height
    );
    glViewport(0, 0, ctx->winData->width, ctx->winData->height);

    // Für die GUI brauchen wir die echte Fenstergröße.
    // Auch sie wird über ein Callback aktualisiert.
    glfwGetWindowSize(
        ctx->window, 
        &ctx->winData->realWidth, 
        &ctx->winData->realHeight
    );

    // Module initialisieren.
    input_init(ctx);
    rendering_init(ctx);
    gui_init(ctx);

    return ctx;
}

void window_mainloop(ProgContext* ctx)
{
    // FPS Timer initialisieren.
    window_initFpsTimer(ctx);

    // Fullscreen-Einstellungen einmal am Anfang übernehmen.
    window_updateFullscreen(ctx);

    // Hauptschleife des Programmes. Sie wird erst beendet, wenn der Nutzer das
    // Fenster schließt.
    while (!glfwWindowShouldClose(ctx->window))
    {
        // Events abrufen und wenn nötig verarbeiten.
        glfwPollEvents();

        // Eingaben verarbeiten.
        input_process(ctx);

        // Szene zeichnen
        rendering_draw(ctx);

        // GUI Zeichnen
        gui_render(ctx);

        // Back- und Frontbuffer tauschen um den neuen Frame anzuzeigen.
        glfwSwapBuffers(ctx->window);

        // FPS Timer updaten.
        window_updateFpsTimer(ctx);
    }
}

void window_updateFullscreen(ProgContext* ctx)
{
    // Prüfen, ob sich überhaupt etwas ändern muss.
    if (ctx->input->isFullscreen != (glfwGetWindowMonitor(ctx->window) != NULL))
    {
        if (ctx->input->isFullscreen)
        {
            // Fenstereinstellungen sichern.
            glfwGetWindowPos(
                ctx->window,
                &ctx->winData->cachedPosX,
                &ctx->winData->cachedPosY
            );
            glfwGetWindowSize(
                ctx->window,
                &ctx->winData->cachedWidth,
                &ctx->winData->cachedHeight
            );

            // Fullscreenmodus aktivieren.
            GLFWmonitor* monitor = window_getCurrentMonitor(ctx->window);
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(
                ctx->window,
                monitor,
                0, 
                0,
                mode->width, 
                mode->height, 
                mode->refreshRate
            );
        }
        else
        {
            // Fenstermodus wiederherstellen.
            glfwSetWindowMonitor(
                ctx->window, 
                NULL, // Kein Monitor stellt den Fenstermodus wieder her.
                ctx->winData->cachedPosX, 
                ctx->winData->cachedPosY,
                ctx->winData->cachedWidth, 
                ctx->winData->cachedHeight,
                GLFW_DONT_CARE
            );
        }
    }
}

void window_cleanup(ProgContext* ctx)
{
    // Alle Module Stück für Stück löschen.
    texture_deleteCache();
    input_cleanup(ctx);
    rendering_cleanup(ctx);
    gui_cleanup(ctx);
    common_deleteContext(ctx);
}
