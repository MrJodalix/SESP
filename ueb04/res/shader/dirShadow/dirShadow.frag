#version 410 core

/**
 * Shader fuer die Schatten des Richtungslichts.
 * 
 * Copyright (C) 2023, FH Wedel
 * Autor: Joshua-Scott Schoettke, Ilana Schmara
 */

 void main() 
 {
 gl_FragDepth = gl_FragCoord.z;
 }