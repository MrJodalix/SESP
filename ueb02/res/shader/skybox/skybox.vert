#version 410 core

/**
 * Skybox Shader.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Joshua-Scott Schoettke, Ilana Schmara
 */

 layout (location = 0) in vec3 position;

 out vec3 TexCoords;

 uniform mat4 u_skyProjection;
 uniform mat4 u_skyView;

 void main()
 {
   TexCoords = position;
   gl_Position = u_skyProjection * u_skyView * vec4(position, 1.0);
 }