#version 410 core

/**
 * Direct Light Shader.
 * 
 * Copyright (C) 2023, FH Wedel
 * Autor: Joshua-Scott Schoettke, Ilana Schmara
 */
 
layout (location = 0) in vec3 position;

// Model Matrix
uniform mat4 u_modelMatrix;

// View Matrix
uniform mat4 u_viewMatrix;

// View Matrix
uniform mat4 u_projectionMatrix;

/**
 * Hauptfunktion des Vertex-Shaders.
 * Hier werden die Daten weiter gereicht.
 */
 void main()
 {
    gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * vec4(position, 1.0);
 }