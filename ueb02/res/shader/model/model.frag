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
in TESE_OUT {
    vec2 TexCoords;
    vec3 Normal;
    vec3 FragPos;
    vec4 ioEyeSpacePosition;
    vec3 T;
    vec3 B;
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

// Aktives Material.
uniform Material u_material;

//-----------------------------------


// Ambienter Anteil 
uniform vec3 u_matAmbient;

// Diffuser Anteil
uniform vec3 u_matDiffuse;

// Spekularer Anteil
uniform vec3 u_matSpecular;

// Kameraposition
uniform vec3 u_viewPos;

// Lichtposition.
uniform vec3 u_lightPosVec;

// Spekularer Anteil
uniform vec3 u_lightColor;

// Shaderauswahl
uniform int u_shaderChoice;

// Nebel anzeigen
uniform bool u_useFog;

// Nebel Farbe
uniform vec3 u_fogColor;

// Nebel Dichte
uniform float u_density;

// Normalmap anzeigen
uniform bool u_showNormalMap;

// Model Matrix
uniform mat4 u_modelMatrix;

vec4 phong(vec3 norm)
{
    // ambient
    vec3 ambient = u_material.ambient * u_matAmbient.rgb;
    
    // diffuse 
    vec3 lightDir = normalize(u_lightPosVec);
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 diffuse = u_material.diffuse;

    if(u_material.useDiffuseMap)
    { 
        diffuse *= texture(u_material.diffuseMap, fs_in.TexCoords).rgb;
    }
    diffuse *= diff * u_matDiffuse.rgb * u_lightColor;

    ambient *= diffuse;

    // specular
    vec3 viewDir = normalize(u_viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, norm); 
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_material.shininess);

    vec3 specular = u_material.specular.rgb;
    
    if(u_material.useSpecularMap)
    { 
        specular *= vec3(texture(u_material.specularMap, fs_in.TexCoords).b);
    }
    
    specular *= u_matSpecular.rgb * u_lightColor * spec;
    
    
    vec3 emission = u_material.emission.rgb;

    if(u_material.useEmissionMap)
    {
       emission *= vec3(texture(u_material.emissionMap, fs_in.TexCoords).rgb);
    } 

    vec4 result = vec4((ambient + diffuse + specular + emission), 1.0);

    return result;
}

/**
 * Hauptfunktion des Fragment-Shaders.
 * Hier wird die Farbe des Fragmentes bestimmt.
 */
void main()
{
    fragColor = vec4(0.0);
    vec3 norm = normalize(fs_in.Normal);

    if (u_showNormalMap)
    {
        vec3 t = normalize(fs_in.T);
        vec3 b = normalize(fs_in.B);
        vec3 n = normalize(fs_in.Normal);

        // Handedness
        if (dot(cross(n, t), b) < 0.0)
        {
            t = t * -1.0;
        }

        mat3 TBN = mat3(t, b, n);

        // Korrektur Tangente
         if (inverse(TBN) != transpose(TBN))
         {
             t = normalize(t - n * dot(n, t));
            TBN = mat3(t, b, n);
         }

        // Normalmapping
        if(u_material.useNormalMap)
        {
            norm = texture(u_material.normalMap, fs_in.TexCoords).rgb;
            norm = normalize(norm * 2.0 - 1.0);

            // Fuer Bistro-Szene die 3. Komponente der Normalen berechnen
            norm.z = sqrt(1.0f - pow(norm.x , 2.0f) - pow(norm.y, 2.0f));

            norm = TBN * norm;
        }
     }


    // Phong-Beleuchtung
    if (u_shaderChoice == 0)
    {
        fragColor = phong(norm);
    } 
    // Geometrienormale
    else if (u_shaderChoice == 1) 
    {
        fragColor = vec4(fs_in.Normal, 1.0f);
    }
    // Cubemap
    else if (u_shaderChoice == 2)
    {
        fragColor = vec4(fs_in.FragPos, 1.0f);
    }
    // Texture UV
    else if (u_shaderChoice == 3)
    {
        fragColor = vec4(fs_in.TexCoords, 1.0f, 1.0f);
    }
    // Diffuse-Map
    else if (u_shaderChoice == 4)
    {
        if(u_material.useDiffuseMap)
        {
            fragColor = texture(u_material.diffuseMap, fs_in.TexCoords);
        } 
    }
    // Spekular
    else if (u_shaderChoice == 5)
    {
        if(u_material.useSpecularMap)
        {
            fragColor = vec4(texture(u_material.specularMap, fs_in.TexCoords).b);
        } 
    }
    // Emission
    else if (u_shaderChoice == 6)
    {
        if(u_material.useEmissionMap)
        {
            fragColor = texture(u_material.emissionMap, fs_in.TexCoords);
        } 
    }
    // Normalen
    else if (u_shaderChoice == 7)
    {        
        if(u_material.useNormalMap)
        {
            fragColor = vec4(norm, 1.0);
        } 
    }

    if (fragColor.a < 0.1) 
    {
        discard;
    }
    
    // Berechnung Exp2-Nebel
    if (u_useFog) 
    {   
        float fogCoordinate = abs(fs_in.ioEyeSpacePosition.z / fs_in.ioEyeSpacePosition.w);
        fragColor = mix(fragColor, vec4(u_fogColor, 1.0), 1.0 - clamp(exp(-pow(u_density * fogCoordinate, 2.0)), 0.0, 1.0));
    }
}
