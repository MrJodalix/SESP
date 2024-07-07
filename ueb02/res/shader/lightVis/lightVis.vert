#version 410 core

/**
 * 3D Modell Shader.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 3) in vec2 texCoord;

// Eigenschaften, die an den Fragmentshader weitergegeben werden sollen.
out VS_OUT  {
    vec3 Normal;
    vec2 TexCoords;
    vec3 FragPos;
} vs_out;

// Model Matrix.
uniform mat4 u_mvpMatrix;

uniform mat4 u_modelMatrix;


/**
 * Hauptfunktion des Vertex-Shaders.
 * Hier werden die Daten weiter gereicht.
 */
void main()
{    
    vs_out.FragPos = vec3(u_modelMatrix * vec4(position, 1.0));
    gl_Position = u_mvpMatrix * vec4(position, 1.0);
}