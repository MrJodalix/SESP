#version 410 core

/**
 * Richtungslicht Shader.
 * 
 * Copyright (C) 2023, FH Wedel
 * Autor: Joshua-Scott Schoettke, Ilana Schmara
 */

layout (location = 0) in vec3 position;

/**
 * Hauptfunktion des Vertex-Shaders.
 * Hier werden die Daten weiter gereicht.
 */
void main()
{
    gl_Position = vec4(position, 1.0);
}