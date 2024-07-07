#version 410 core

/**
 * Direct Light Shader.
 * 
 * Copyright (C) 2023, FH Wedel
 * Autor: Joshua-Scott Schoettke, Ilana Schmara
 */
 
// Nur die Farbe wird ausgegeben.
layout (location = 0) out vec4 fragColor;

// Uniforms für die Color-Attachments
uniform sampler2D u_Position;
uniform sampler2D u_Normal;
uniform sampler2D u_AlbedoSpec;
uniform sampler2D u_Emission;

<<<<<<< .mine
// Bildschirmgroesze
||||||| .r46
=======
// Bildschirmgroesse
>>>>>>> .r78
uniform vec2 u_screenSize;

// Lichtrichtung
uniform vec3 u_lightPosVec;

// Spekularer Anteil
uniform vec3 u_lightColor;

// Ambienter Anteil 
uniform float u_matAmbient;

// Spekularer Anteil
uniform float u_matSpecular;

// Diffuser Anteil
uniform float u_matDiffuse;

// Kameraposition
uniform vec3 u_viewPos;

 // ambienter Faktor
 const float ambientFactor = 0.1;


vec2 CalcTexCoord()
{
   return gl_FragCoord.xy / u_screenSize;
} 

/**
 * Lichtberechnung nach Phong
 * 
 * @param norm die Normalen
 * @param color die Farbe 
 * @param specular spekularer Faktor
 * @param emission die Emission
 */
vec4 phong(vec3 norm, vec2 texCoord, vec3 color, float specular, vec3 emission)
{
    vec3 WorldPos = texture(u_Position, CalcTexCoord()).xyz;
 
    vec3 lightDir = normalize(u_lightPosVec);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * color * u_lightColor * u_matDiffuse;

    vec3 ambient = ambientFactor * color * u_lightColor * u_matAmbient;

    vec3 viewDir = normalize(u_viewPos - WorldPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
    specular *= (u_lightColor * spec).b * u_matSpecular;

    vec4 result = vec4((ambient + diffuse + specular + emission), 1.0);

    return result;
}

/**
 * Hauptfunktion des Fragment-Shaders.
 * Hier wird die Farbe des Fragmentes ueber die Lichtberechnung bestimmt.
 */
 void main()
 {
    vec2 TexCoord = CalcTexCoord();
    vec3 Color = texture(u_AlbedoSpec, TexCoord).xyz;
    vec3 Normal = texture(u_Normal, TexCoord).xyz;
	float Specular = texture(u_AlbedoSpec, TexCoord).a;
    vec3 Emission = texture(u_Emission, TexCoord).rgb;
    Normal = normalize(Normal);

    fragColor =  phong(Normal, TexCoord, Color, Specular, Emission);
}