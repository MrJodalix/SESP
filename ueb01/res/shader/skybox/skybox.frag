#version 410 core

/**
 * Skybox Shader.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Joshua-Scott Schoettke, Ilana Schmara
 */

 out vec4 FragColor;

 in vec3 TexCoords;

 uniform samplerCube skybox;

 void main()
 {
     FragColor = texture(skybox, TexCoords);
 }