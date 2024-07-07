#version 410 core

/**
 * 3D Modell Shader.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

// Hier können verschiedene Unterteilungsmethoden (engl. spacing modes) gewählt 
// werden. "equal_spacing" kann durch "fractional_odd_spacing" oder 
// "fractional_even_spacing" ersetzt werden.
layout (triangles, equal_spacing, ccw) in;

in TESC_OUT {
    vec2 TexCoords;
    vec4 Position;
    vec3 Normal;
    vec3 FragPos;
    vec4 ioEyeSpacePosition;
    vec3 T;
    vec3 B;
} tese_in[];

out TESE_OUT {
    vec2 TexCoords;
    vec3 Normal;
    vec3 FragPos;
    vec4 ioEyeSpacePosition;
    vec3 T;
    vec3 B;
} tese_out;

// Uniforms
uniform mat4 u_mvpMatrix;

////////////////////////////////// FUNKTIONEN /////////////////////////////////

/**
 * Baryzentrische Interpolation von 3 4D-Vektoren
 *
 * Nutzt die vordefinierte Variable gl_TessCoord, um eine baryzentrische 
 * Interpolation der 3 übergebenen Vektoren durchzuführen.
 *
 * @param v0 Vektor, der dem Koeffizienten gl_TessCoord.x zugeordnet ist
 * @param v1 Vektor, der dem Koeffizienten gl_TessCoord.y zugeordnet ist
 * @param v2 Vektor, der dem Koeffizienten gl_TessCoord.z zugeordnet ist
 *
 * @return der interpolierter Vektor
 */
vec4 interpolate4D(vec4 v0, vec4 v1, vec4 v2) 
{
    return gl_TessCoord.x * v0 + gl_TessCoord.y * v1 + gl_TessCoord.z * v2;
}

/**
 * Baryzentrische Interpolation von 3 3D-Vektoren
 *
 * Nutzt die vordefinierte Variable gl_TessCoord, um eine baryzentrische 
 * Interpolation der 3 übergebenen Vektoren durchzuführen.
 *
 * @param v0 Vektor, der dem Koeffizienten gl_TessCoord.x zugeordnet ist
 * @param v1 Vektor, der dem Koeffizienten gl_TessCoord.y zugeordnet ist
 * @param v2 Vektor, der dem Koeffizienten gl_TessCoord.z zugeordnet ist
 *
 * @return der interpolierter Vektor
 */
vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2) 
{
    return gl_TessCoord.x * v0 + gl_TessCoord.y * v1 + gl_TessCoord.z * v2;
}

/**
 * Baryzentrische Interpolation von 3 2D-Vektoren
 *
 * Nutzt die vordefinierte Variable gl_TessCoord, um eine baryzentrische 
 * Interpolation der 3 übergebenen Vektoren durchzuführen.
 *
 * @param v0 Vektor, der dem Koeffizienten gl_TessCoord.x zugeordnet ist
 * @param v1 Vektor, der dem Koeffizienten gl_TessCoord.y zugeordnet ist
 * @param v2 Vektor, der dem Koeffizienten gl_TessCoord.z zugeordnet ist
 *
 * @return der interpolierter Vektor
 */
vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2) 
{
    return gl_TessCoord.x * v0 + gl_TessCoord.y * v1 + gl_TessCoord.z * v2;
}


/**
 * Einsprungpunkt für den Tessellation Evaluation Shader.
 */
void main() {

    tese_out.TexCoords = interpolate2D(tese_in[0].TexCoords, tese_in[1].TexCoords, tese_in[2].TexCoords);
    tese_out.Normal = interpolate3D(tese_in[0].Normal, tese_in[1].Normal, tese_in[2].Normal);
    tese_out.FragPos = interpolate3D(tese_in[0].FragPos, tese_in[1].FragPos, tese_in[2].FragPos);
    tese_out.ioEyeSpacePosition = interpolate4D(tese_in[0].ioEyeSpacePosition, tese_in[1].ioEyeSpacePosition, tese_in[2].ioEyeSpacePosition);
    tese_out.T = interpolate3D(tese_in[0].T, tese_in[1].T, tese_in[2].T);
    tese_out.B = interpolate3D(tese_in[0].B, tese_in[1].B, tese_in[2].B);

    vec4 oldPosition = interpolate4D(tese_in[0].Position, tese_in[1].Position, tese_in[2].Position);
    
    gl_Position = u_mvpMatrix * oldPosition;
}
