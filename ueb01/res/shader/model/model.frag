#version 410 core

/**
 * 3D Modell Shader.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

// Nur die Farbe wird ausgegeben.
layout (location = 0) out vec4 fragColor;

// Eigenschaften, die von dem Vertextshader weitergegeben wurden.
in VS_OUT {
    vec2 TexCoords;
    vec3 Normal;
    vec3 FragPos;
    vec4 ioEyeSpacePosition;
} fs_in;

// Struktur für Materialeigenschaften.
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 emission;
    float shininess;

    bool useDiffuseMap;
    sampler2D diffuseMap;

    bool useSpecularMap;
    sampler2D specularMap;

    bool useNormalMap;
    sampler2D normalMap;

    bool useEmissionMap;
    sampler2D emissionMap;
};

// Aktives material.
uniform Material u_material;

//-----------------------------------

// Lichtposition.
uniform vec3 u_lightPosVec;

// Ambienter Anteil 
uniform vec3 u_matAmbient;

// Diffuser Anteil
uniform vec3 u_matDiffuse;

// Spekularer Anteil
uniform vec3 u_matSpecular;

// Spekularer Anteil
uniform vec3 u_lightColor;

// Kameraposition
uniform vec3 u_viewPos;

// Shader auswahl
uniform int u_shaderChoice;

// Nebel anzeigen
uniform bool u_useFog;

//Nebel Farbe
uniform vec3 u_fogColor;

//Nebel dichte
uniform float u_density;

/**
 * Hauptfunktion des Fragment-Shaders.
 * Hier wird die Farbe des Fragmentes bestimmt.
 */
void main()
{
    vec3 norm = normalize(fs_in.Normal);
 
    //Phong belichtung
    if (u_shaderChoice == 0){

        // ambient
        vec3 ambient = u_matAmbient.rgb * u_lightColor;

        // diffuse 
        vec3 lightDir = normalize(u_lightPosVec);
        float diff = max(dot(norm, u_lightPosVec), 0.0);

        vec3 diffuse = vec3(0.0);

        if(u_material.useDiffuseMap)
        {
            diffuse = diff * u_matDiffuse.rgb * u_lightColor;
        }

        // specular
        vec3 viewDir = normalize(u_viewPos - fs_in.FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 2.0);

        vec3 specular = vec3(0.0);

        if (u_material.useSpecularMap) 
        {
            specular = spec * u_matSpecular.rgb * u_lightColor;  
        }
        vec4 result = vec4((ambient + diffuse + specular), 1.0);

        fragColor = texture(u_material.diffuseMap, fs_in.TexCoords) * result;
    } 
    //Geometrie normale
    else if (u_shaderChoice == 1) 
    {
        fragColor = vec4(fs_in.Normal, 1.0f);
    }
    // Cubemap
    else if (u_shaderChoice == 2)
    {
        fragColor = vec4(fs_in.TexCoords, 1.0f, 1.0f);
    }
    //Texture UV
    else if (u_shaderChoice == 3)
    {
        fragColor = vec4(fs_in.TexCoords, 1.0f, 1.0f);
    }
    //Diffuse Map
    else if (u_shaderChoice == 4)
    {
        if(u_material.useDiffuseMap)
        {
            fragColor = texture(u_material.diffuseMap, fs_in.TexCoords);
        } 
        else
        {
            fragColor = vec4(0.0);
        }
    }
    //Spekular
    else if (u_shaderChoice == 5)
    {
        if(u_material.useSpecularMap)
        {
            fragColor = texture(u_material.specularMap, fs_in.TexCoords);
        } 
        else
        {
            fragColor = vec4(0.0);
        }
    }
    //Emission
    else if (u_shaderChoice == 6)
    {
        if(u_material.useEmissionMap)
        {
            fragColor = texture(u_material.emissionMap, fs_in.TexCoords);
        } 
        else
        {
            fragColor = vec4(0.0);
        }
    }
    //Normal
    else if (u_shaderChoice == 7)
    {        
        if(u_material.useNormalMap)
        {
            fragColor = texture(u_material.normalMap, fs_in.TexCoords);
        } 
        else
        {
            fragColor = vec4(0.0);
        }
    }

    if (fragColor.a < 0.1) 
    {
        discard;
    }
    
    if (u_useFog) 
    {
        //Berechnung exp2 Nebel
        float fogCoordinate = abs(fs_in.ioEyeSpacePosition.z / fs_in.ioEyeSpacePosition.w);
        fragColor = mix(fragColor, vec4(u_fogColor, 1.0), 1.0 - clamp(exp(-pow(u_density * fogCoordinate, 2.0)), 0.0, 1.0));
    }
}
