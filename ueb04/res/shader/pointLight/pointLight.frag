#version 410 core

/**
 * Punktlicht Shader.
 * 
 * Copyright (C) 2023, FH Wedel
 * Autor: Joshua-Scott Schoettke, Ilana Schmara
 */
 
// Nur die Farbe wird ausgegeben.
layout (location = 0) out vec4 fragColor;

uniform sampler2D u_Position;
uniform sampler2D u_Normal;
uniform sampler2D u_AlbedoSpec;
uniform sampler2D u_Emission;
uniform sampler2D u_ShadowMap;

struct PointLight
{
    vec3 pos;
    vec3 color;
    
    vec3 amb;
    vec3 diff;
    vec3 spec;

    float constant;
    float linear;
    float quadratic;
};
uniform PointLight pointLight;

// Bildschirmgroesze
uniform vec2 u_screenSize;

// Lichtposition.
uniform vec3 u_lightPosVec;

// Lichtfarben Anteil
//uniform vec3 u_lightColor;

// Ambienter Anteil 
uniform float u_matAmbient;

// Spekularer Anteil
uniform float u_matSpecular;

// Diffuser Anteil
uniform float u_matDiffuse;

// Kameraposition
uniform vec3 u_viewPos;

 // ambient
 const float ambientFactor = 0.1;


vec2 CalcTexCoord()
{
   return gl_FragCoord.xy / u_screenSize;
} 

/**
 * Lichtberechnung nach Phong
 * 
 * @param norm die Normalen
 */
vec4 phong(vec3 norm, vec2 texCoord, vec3 color, float specular, vec3 emission)
{
    vec3 WorldPos = texture(u_Position, CalcTexCoord()).xyz;
    vec3 lightDirection = WorldPos - pointLight.pos;
    float Distance = length(lightDirection);
    lightDirection = normalize(lightDirection);

    vec3 ambient = ambientFactor * color * pointLight.color * u_matAmbient * pointLight.amb;

    // diffuse 
    //vec3 lightDir = vec3(abs(normalize(vec4(pointLight.pos, 1.0) - gl_FragCoord))).rgb;
    float diff = max(dot(norm, lightDirection), 0.0);

    vec3 diffuse = diff * color * pointLight.color * u_matDiffuse * pointLight.diff;

    // specular
    vec3 viewDir = normalize(u_viewPos - WorldPos);
    vec3 reflectDir = reflect(-lightDirection, norm); 
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
    
    specular *= (pointLight.color * spec).b * u_matSpecular * pointLight.spec.b;


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