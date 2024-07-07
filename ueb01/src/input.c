/**
 * Modul zum Abfragen von Benutzereingaben.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

#include "input.h"

#include "window.h"
#include "texture.h"
#include "utils.h"

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

void input_init(ProgContext* ctx)
{
    ctx->input = malloc(sizeof(InputData));
    InputData* data = ctx->input;

    // Werte initialisieren
    data->isFullscreen = false;
    data->showHelp = false;
    data->showMenu = true;
    data->showWireframe = false;
    data->showStats = true;

    // Shader neu laden
    data->reloadShader = false;

    //Shader auswahl
    data->shaderChoice = 0;

    //Nebel anzeigen
    data->showFog = false;

    //Nebel Farbe
    glm_vec4_zero(data->rendering.clearColor);
    data->rendering.clearColor[3] = 0.0f;
    data->rendering.userModel = NULL;

    //Nebel dichte
    data->density = 0.0f;


    glm_vec4_zero(data->rendering.lightComp);
    data->rendering.lightComp[0] = 0.5f;
    data->rendering.lightComp[1] = 1.0f;
    data->rendering.lightComp[2] = 0.3f;

    glm_vec4_zero(data->rendering.lightDir);
    data->rendering.lightDir[0] = 0.0f;
    data->rendering.lightDir[1] = 1.0f;
    data->rendering.lightDir[2] = 0.0f;

    glm_vec4_zero(data->rendering.lightColor);
    data->rendering.lightColor[0] = 1.0f;
    data->rendering.lightColor[1] = 1.0f;
    data->rendering.lightColor[2] = 1.0f;

    glm_vec3_zero(data->rendering.translate);
    data->rendering.translate[0] = 0.0;
    data->rendering.translate[1] = 0.0;
    data->rendering.translate[2] = 0.0;

    glm_vec3_zero(data->rendering.rotate);
    data->rendering.rotate[0] = 0.0;
    data->rendering.rotate[1] = 0.0;
    data->rendering.rotate[2] = 0.0;

    glm_vec3_zero(data->rendering.scale);
    data->rendering.scale[0] = 1.0;
    data->rendering.scale[1] = 1.0;
    data->rendering.scale[2] = 1.0;

    // Kamera initialisieren
    data->mainCamera = camera_createCamera();
    glfwGetCursorPos(ctx->window, &data->mouseLastX, &data->mouseLastY);
    data->mouseLooking = false;
}

void input_process(ProgContext* ctx)
{
    // Kamerabewegung verarbeiten
    Camera* mainCamera = ctx->input->mainCamera;
    float deltaTime = (float) ctx->winData->deltaTime;
    bool shift = glfwGetKey(ctx->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

    if (glfwGetKey(ctx->window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera_processKeyboardInput(mainCamera, CAMERA_FORWARD, shift, deltaTime);
    }
    if (glfwGetKey(ctx->window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera_processKeyboardInput(mainCamera, CAMERA_BACKWARD, shift, deltaTime);
    }
    if (glfwGetKey(ctx->window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera_processKeyboardInput(mainCamera, CAMERA_LEFT, shift, deltaTime);
    }
    if (glfwGetKey(ctx->window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera_processKeyboardInput(mainCamera, CAMERA_RIGHT, shift, deltaTime);
    }
    if (glfwGetKey(ctx->window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        camera_processKeyboardInput(mainCamera, CAMERA_DOWN, shift, deltaTime);
    }
    if (glfwGetKey(ctx->window, GLFW_KEY_E) == GLFW_PRESS)
    {
        camera_processKeyboardInput(mainCamera, CAMERA_UP, shift, deltaTime);
    }
}

void input_event(ProgContext* ctx, int key, int action, int mods)
{
    // Mods wird aktuell nicht benutzt.
    (void) mods;

    // Input-Events verarbeiten.
    InputData* data = ctx->input;
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        /* Programm beenden */
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(ctx->window, true);
            break;
        
        /* Hilfe anzeigen/ausblenden */
        case GLFW_KEY_F1:
            data->showHelp = !data->showHelp;
            break;

        /* Fullscreen umschalten */
        case GLFW_KEY_F2:
            data->isFullscreen = !data->isFullscreen;
            window_updateFullscreen(ctx);
            break;

        /* Wireframe anzeigen/ausblenden */
        case GLFW_KEY_F3:
            data->showWireframe = !data->showWireframe;
            break;

        /* Programm-Menü anzeigen/ausblenden */
        case GLFW_KEY_F4:
            data->showMenu = !data->showMenu;
            break;

        /* Programmstatistiken anzeigen/ausblenden */
        case GLFW_KEY_F5:
            data->showStats = !data->showStats;
            break;

        /* Screenshot anfertigen */
        case GLFW_KEY_F6:
            texture_saveScreenshot(ctx);
            break;
        
        default:
            break;
        }
    }
}

void input_mouseMove(ProgContext* ctx, double x, double y)
{
    InputData* data = ctx->input;

    // Wir rotieren die Kamera nur, wenn zuvor der Bewegungsmodus mit der
    // linken Maustaste aktiviert wurde.
    if (data->mouseLooking)
    {
        // Veränderung berechnen.
        double xoff = x - data->mouseLastX;
        double yoff = data->mouseLastY - y;

        data->mouseLastX = x;
        data->mouseLastY = y;

        // Kamera rotieren.
        camera_processMouseInput(data->mainCamera, (float) xoff, (float) yoff);

        // Nocheinmal prüfen, ob die Maustaste wirklich gedrückt ist.
        // Es kann passieren, dass das RELEASE Event verschluckt wurde, z.B.
        // druch die GUI.
        data->mouseLooking = (
            glfwGetMouseButton(ctx->window, GLFW_MOUSE_BUTTON_LEFT) == 
            GLFW_PRESS
        );
    }
}

void input_mouseAction(ProgContext* ctx, int button, int action, int mods)
{
    // Die Modifikatioren werden nicht benutzt.
    (void) mods;

    InputData* data = ctx->input;

    // Prüfen, ob die linke Maustaste verändert wurde.
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            // Mausmodus aktivieren.
            glfwGetCursorPos(ctx->window, &data->mouseLastX, &data->mouseLastY);
            data->mouseLooking = true;
        }
        else if (action == GLFW_RELEASE)
        {
            // Mausmodus deaktivieren.
            data->mouseLooking = false;
        }
    }
}

void input_scroll(ProgContext* ctx, double xoff, double yoff)
{
    // Horizontales scrollen wird nicht benötigt.
    (void) xoff;

    // Zoom der Kamera anpassen.
    camera_processMouseZoom(ctx->input->mainCamera, (float) yoff);
}

void input_userSelectedFile(ProgContext* ctx, const char* path)
{
    Model* newModel = NULL;
    newModel = model_loadModel(path);

    // Wir tauschen das Modell nur aus, wenn es erfolgreich geladen werden
    // konnte.
    if (newModel != NULL)
    {
        texture_deleteCache();
        // Sollte zuvor ein Modell geladen worden sein, muss es gelöscht werden.
        if (ctx->input->rendering.userModel != NULL)
        {
            model_deleteModel(ctx->input->rendering.userModel);
        }
        ctx->input->rendering.userModel = newModel;
    }
}

void input_cleanup(ProgContext* ctx)
{
    // Wenn eine Modelldatei geladen ist, muss diese gelöscht werden.
    if (ctx->input->rendering.userModel != NULL)
    {
        model_deleteModel(ctx->input->rendering.userModel);
    }

    camera_deleteCamera(ctx->input->mainCamera);

    free(ctx->input);
}
