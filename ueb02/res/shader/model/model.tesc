#version 410 core

/**
 * 3D Modell Shader.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

layout (vertices = 3) out;

in VS_OUT {
    vec2 TexCoords;
    vec4 Position;
    vec3 Normal;
    vec3 FragPos;
    vec4 ioEyeSpacePosition;
    vec3 T;
    vec3 B;
} tesc_in[];

out TESC_OUT {
    vec2 TexCoords;
    vec4 Position;
    vec3 Normal;
    vec3 FragPos;
    vec4 ioEyeSpacePosition;
    vec3 T;
    vec3 B;
} tesc_out[];

// Uniforms
uniform float u_TessLevelInner;
uniform float u_TessLevelOuter;
uniform bool u_useTess;

/**
 * Einsprungpunkt für den Tessellation Control Shader.
 */
void main() 
{
    tesc_out[gl_InvocationID].TexCoords = tesc_in[gl_InvocationID].TexCoords;
    tesc_out[gl_InvocationID].Position = tesc_in[gl_InvocationID].Position;
    tesc_out[gl_InvocationID].Normal = tesc_in[gl_InvocationID].Normal;
    tesc_out[gl_InvocationID].FragPos = tesc_in[gl_InvocationID].FragPos;
    tesc_out[gl_InvocationID].ioEyeSpacePosition = tesc_in[gl_InvocationID].ioEyeSpacePosition;
    tesc_out[gl_InvocationID].T = tesc_in[gl_InvocationID].T;
    tesc_out[gl_InvocationID].B = tesc_in[gl_InvocationID].B;


    if (gl_InvocationID == 0) 
    {
        if(u_useTess)
        {
            gl_TessLevelInner[0] = u_TessLevelInner;
            gl_TessLevelOuter[0] = u_TessLevelOuter;
            gl_TessLevelOuter[1] = u_TessLevelOuter;
            gl_TessLevelOuter[2] = u_TessLevelOuter;
        }
        else
        {
            gl_TessLevelInner[0] = 1;
            gl_TessLevelOuter[0] = 1;
            gl_TessLevelOuter[1] = 1;
            gl_TessLevelOuter[2] = 1;
        }
    }
}

