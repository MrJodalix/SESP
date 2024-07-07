#version 410 core

/**
 * postProcess Shader.
 * 
 * Copyright (C) 2023, FH Wedel
 * Autor: Joshua-Scott Schoettke, Ilana Schmara
 */

 layout (location = 0) in vec3 position;

 void main()
 {
   gl_Position = vec4(position, 1.0);
 }