#version 410 core

/**
 * Shader fuer die Schatten des Richtungslichts.
 * 
 * Copyright (C) 2023, FH Wedel
 * Autor: Joshua-Scott Schoettke, Ilana Schmara
 */

layout (location = 0) in vec3 position;

uniform mat4 u_modelMatrix;
uniform mat4 u_lightSpaceMatrix;

void main()
{
    gl_Position = u_lightSpaceMatrix * u_modelMatrix * vec4(position, 1.0);
}