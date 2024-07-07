#version 410 core

/**
 * 3D Modell Shader.
 * 
 * Copyright (C) 2023, FH Wedel
 * Autor: Joshua-Scott Schoettke, Ilana Schmara
 */

// Nur die Farbe wird ausgegeben.
layout (location = 0) out vec3 fragPos;
layout (location = 1) out vec3 fragNorm;
layout (location = 2) out vec4 fragAlbedoSpec;
layout (location = 3) out vec3 fragEmission;
layout (location = 4) out vec2 texCoords;

// Eigenschaften, die von dem Vertextshader weitergegeben wurden.
in TESE_OUT {
    vec2 TexCoords;
    vec3 Normal;
    vec3 FragPos;
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

/*
// Nebel anzeigen
uniform bool u_useFog;

// Nebel Farbe
uniform vec3 u_fogColor;

// Nebel Dichte
uniform float u_density;
*/

// Normalmap anzeigen
uniform bool u_showNormalMap;

/**
 * berechnung der TBN Matrix und korrektur der Normalen
 * 
 * @param norm die Normalen
*/
vec3 tbnMatrix(vec3 norm)
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
    
    return norm;
}

/**
 * Hauptfunktion des Fragment-Shaders.
 * Hier wird die Farbe des Fragmentes bestimmt.
 */
void main()
{
    fragPos = fs_in.FragPos;

    fragNorm = normalize(fs_in.Normal);

    // Normalenberechnug
    if (u_showNormalMap)
    {
        fragNorm = tbnMatrix(fragNorm);
    } else {
        if(u_material.useNormalMap)
        {
            fragNorm = texture(u_material.normalMap, fs_in.TexCoords).rgb;
            fragNorm = normalize(fragNorm);
        }
    }
    
    // Diffuse und Ambientberechnug
    fragAlbedoSpec.rgb = u_material.diffuse;
    if(u_material.useDiffuseMap)
    {
        fragAlbedoSpec.rgb = texture(u_material.diffuseMap, fs_in.TexCoords).rgb;
    }
    
    // Spekularberechnug
    fragAlbedoSpec.a = u_material.specular.b;
    if(u_material.useSpecularMap)
    {
        fragAlbedoSpec.a = texture(u_material.specularMap, fs_in.TexCoords).b;
    }
    
    // Emissionsberechnug
    fragEmission = u_material.emission;
    if(u_material.useEmissionMap)
    {
        fragEmission = texture(u_material.emissionMap, fs_in.TexCoords).rgb;
    }


    texCoords = fs_in.TexCoords;

    /*  
    // Berechnung Exp2-Nebel
    if (u_useFog) 
    {   
        float fogCoordinate = abs(fs_in.ioEyeSpacePosition.z / fs_in.ioEyeSpacePosition.w);
        fragColor = mix(fragColor, vec4(u_fogColor, 1.0), 1.0 - clamp(exp(-pow(u_density * fogCoordinate, 2.0)), 0.0, 1.0));
    }
    */
}
