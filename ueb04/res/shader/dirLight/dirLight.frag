#version 410 core

/**
 * Richtungslicht Shader.
 * 
 * Copyright (C) 2023, FH Wedel
 * Autor: Joshua-Scott Schoettke, Ilana Schmara
 */
 
// Nur die Farbe wird ausgegeben.
layout (location = 0) out vec4 fragColor;


const float maxBIAS = 0.0005;
const float minBIAS = 0.0005;

uniform sampler2D u_Position;
uniform sampler2D u_Normal;
uniform sampler2D u_AlbedoSpec;
uniform sampler2D u_Emission;
uniform sampler2D u_shadowMap;

uniform mat4 u_modelMatrix;
uniform mat4 u_lightSpaceMatrix;

// Bildschirmgroesze
uniform vec2 u_screenSize;

// Lichtposition.
uniform vec3 u_lightDirVec;

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

 // ambient
 const float ambientFactor = 0.1;


vec2 CalcTexCoord()
{
   return gl_FragCoord.xy / u_screenSize;
}

float shadowCalculationBiased(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
	// Perspective Devide
	// transform clip-space coordinates in the range [-w,w] to [-1, 1]
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// transform NDC coordinantes to the range [0,1]
	projCoords = projCoords * 0.5 +0.5;
	// Sample the depth map
	float closestDepth = texture(u_shadowMap, projCoords.xy).r;	//from lights point of view
	// get current depth at the fragment 
	float currentDepth = projCoords.z;
	//Shadow Bias (offsets the depth of the surface [or  the shadow map] by a small bias amount)
	//float bias = BIAS;
	float bias = max(maxBIAS * (1.0 - dot(normal,lightDir)), minBIAS); // change amount of bias based on the surface angle towards the light: something we can solve with the dot product
	
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	// force shadow when z coordinante is larger than 1.0
	if(projCoords.z > 1.0)
    {
        shadow = 0.0;
    }

	return shadow;
}

/**
 * Lichtberechnung nach Phong
 * 
 * @param norm die Normalen
 */
vec4 phong(vec3 norm, vec2 texCoord, vec3 color, float specular, vec3 emission)
{
    vec4 FragPosLightSpace; // world-space vertex position transformed to light space
    
   
    vec3 WorldPos = texture(u_Position, CalcTexCoord()).xyz;

    FragPosLightSpace = u_lightSpaceMatrix * vec4(WorldPos, 1.0);

    // diffuse 
    vec3 lightDir = normalize(u_lightDirVec);
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 diffuse = diff * color * u_lightColor * u_matDiffuse;

    vec3 ambient = ambientFactor * color * u_lightColor * u_matAmbient;

    // specular
    vec3 viewDir = normalize(u_viewPos - WorldPos);
    vec3 reflectDir = reflect(-lightDir, norm); 
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
    
    specular *= (u_lightColor * spec).b * u_matSpecular;

	float shadow =  shadowCalculationBiased(FragPosLightSpace, norm, lightDir);

    vec4 result = vec4((ambient + (1.0 - shadow) * (diffuse + specular) + emission), 1.0);

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