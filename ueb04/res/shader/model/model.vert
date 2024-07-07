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
layout (location = 4) in vec3 tangent;
layout (location = 5) in vec3 biTangent;

// Kameraposition
uniform vec3 u_viewPos;

// Lichtposition.
uniform vec3 u_lightPosVec;

// Eigenschaften, die an den Fragmentshader weitergegeben werden sollen.
out VS_OUT  {
    vec2 TexCoords;
    vec4 Position;
    vec3 Normal;
    vec3 FragPos;
    //vec4 ioEyeSpacePosition;
    vec3 T;
    vec3 B;
} vs_out;

// Model Matrix
uniform mat4 u_modelMatrix;

// View Matrix
uniform mat4 u_viewMatrix;


/**
 * Hauptfunktion des Vertex-Shaders.
 * Hier werden die Daten weiter gereicht.
 */
void main()
{
    vs_out.TexCoords = texCoord;
    vs_out.FragPos = vec3(u_modelMatrix * vec4(position, 1.0));
    //vs_out.ioEyeSpacePosition = (u_viewMatrix * u_modelMatrix) * vec4(position, 1.0);

    mat3 normalMatrix = transpose(inverse(mat3(u_modelMatrix)));
    vs_out.T = normalize(normalMatrix * tangent);
    vs_out.B = normalize(normalMatrix * biTangent);
    vs_out.Normal = normalize(normalMatrix * normal);
    vs_out.Position = vec4(position, 1.0); 
}
