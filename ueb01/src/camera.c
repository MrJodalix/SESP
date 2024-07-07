/**
 * Modul für die Steuerung einer frei beweglichen 3D Kamera.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

#include "camera.h"

#include <math.h>

////////////////////////////////// KONSTANTEN //////////////////////////////////

#define CAMERA_SPEED 2.5f
#define CAMERA_FAST_SPEED 8.2f
#define MOUSE_SENSITIVITY 0.1f

////////////////////////////// LOKALE DATENTYPEN ///////////////////////////////

// Datenstruktur für die Repräsentation einer 3D Kamera.
struct Camera
{
    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;
    vec3 worldUp;

    float yaw;
    float pitch;
    float zoom;
};

////////////////////////////// LOKALE FUNKTIONEN ///////////////////////////////

/**
 * Aktualisiert die Kameravektoren.
 * 
 * @param camera die Kamera, die aktualisiert werden soll.
 */
static void camera_updateVectors(Camera* camera)
{
    // Zuerst berechnen wir den neuen front Vektor.
    vec3 front;
    front[0] = cosf(glm_rad(camera->yaw)) * cosf(glm_rad(camera->pitch));
    front[1] = sinf(glm_rad(camera->pitch));
    front[2] = sinf(glm_rad(camera->yaw)) * cosf(glm_rad(camera->pitch));
    glm_vec3_normalize_to(front, camera->front);

    // Danach wird der rechte Vektor aktualisiert.
    vec3 right;
    glm_vec3_cross(camera->front, camera->worldUp, right);
    glm_vec3_normalize_to(right, camera->right);

    // Und zum Schluss wird der up Vektor aktualisiert.
    vec3 up;
    glm_vec3_cross(camera->right, camera->front, up);
    glm_vec3_normalize_to(up, camera->up);
}

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

Camera* camera_createCamera(void)
{
    // Speicher reservieren und initialisieren.
    Camera* camera = malloc(sizeof(Camera));

    glm_vec3_zero(camera->position);
    glm_vec3_zero(camera->front);
    glm_vec3_zero(camera->up);
    glm_vec3_zero(camera->right);
    glm_vec3_zero(camera->worldUp);

    camera->front[2] = -1.0f;
    camera->worldUp[1] = 1.0f;
    camera->yaw = -90.0f;
    camera->pitch = 0.0f;
    camera->zoom = 45.0f;

    camera_updateVectors(camera);

    return camera;
}

void camera_getViewMatrix(Camera* camera, mat4 view)
{
    // Als erstes muss berechnet werden, wohin die Kamera sieht.
    vec3 target;
    glm_vec3_add(camera->position, camera->front, target);

    // Danach kann die View-Matrix errechnet werden.
    glm_lookat(
        camera->position, 
        target,
        camera->up,
        view
    );
}

float camera_getZoom(Camera* camera)
{
    return camera->zoom;
}

void camera_getPosition(Camera* camera, vec3 position)
{
    glm_vec3_copy(camera->position, position);
}

void camera_processKeyboardInput(Camera* camera, CameraMovement movement, 
                                 bool fast, float deltaTime)
{
    float velocity = (fast ? CAMERA_FAST_SPEED : CAMERA_SPEED) * deltaTime;
    vec3 movVec;
    
    switch (movement)
    {
    case CAMERA_FORWARD:
        glm_vec3_scale(camera->front, velocity, movVec);
        glm_vec3_add(camera->position, movVec, camera->position);
        break;
    
    case CAMERA_BACKWARD:
        glm_vec3_scale(camera->front, velocity, movVec);
        glm_vec3_sub(camera->position, movVec, camera->position);
        break;
    
    case CAMERA_LEFT:
        glm_vec3_scale(camera->right, velocity, movVec);
        glm_vec3_sub(camera->position, movVec, camera->position);
        break;
    
    case CAMERA_RIGHT:
        glm_vec3_scale(camera->right, velocity, movVec);
        glm_vec3_add(camera->position, movVec, camera->position);
        break;
    
    case CAMERA_UP:
        glm_vec3_scale(camera->up, velocity, movVec);
        glm_vec3_add(camera->position, movVec, camera->position);
        break;

    case CAMERA_DOWN:
        glm_vec3_scale(camera->up, velocity, movVec);
        glm_vec3_sub(camera->position, movVec, camera->position);
        break;
    }
}

void camera_processMouseInput(Camera* camera, float x, float y)
{
    x *= MOUSE_SENSITIVITY;
    y *= MOUSE_SENSITIVITY;

    camera->yaw += x;
    camera->pitch += y;

    // Die Rotation beschränken.
    if (camera->pitch > 89.0f)
    {
        camera->pitch = 89.0f;
    }

    if (camera->pitch < -89.0f)
    {
        camera->pitch = -89.0f;
    }

    camera_updateVectors(camera);
}

void camera_processMouseZoom(Camera* camera, float offset)
{
    camera->zoom -= offset;

    // Den Zoom beschränken.
    if (camera->zoom < 1.0f)
    {
        camera->zoom = 1.0f;
    }

    if (camera->zoom > 45.0f)
    {
        camera->zoom = 45.0f;
    }
}

void camera_deleteCamera(Camera* camera)
{
    free(camera);
}
