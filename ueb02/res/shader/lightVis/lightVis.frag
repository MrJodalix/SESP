#version 410 core

/**
 * LightVis Shader.
 * 
 * Autor: Joshua-Scott Schoettke, Ilana Schmara
 */

 // Nur die Farbe wird ausgegeben.
layout (location = 0) out vec4 fragColor;

// Eigenschaften, die von dem Vertextshader weitergegeben wurden.
in VS_OUT {
    vec3 Normal;
    vec2 TexCoords;
    vec3 FragPos;
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

uniform vec3 u_lightColor;
uniform vec3 u_lightVisPos;
uniform vec3 u_viewVisPos;

 void main()
 {
    vec3 norm = normalize(fs_in.Normal);
     // ambient
    vec3 ambient = u_material.ambient;

    // diffuse 
    vec3 lightDir = abs(normalize(u_lightVisPos - fs_in.FragPos));
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 diffuse = diff * u_material.diffuse;

    // specular
    vec3 viewDir = normalize(u_viewVisPos - fs_in.FragPos);
    vec3 reflectDir = normalize(reflect(-lightDir, norm));  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_material.shininess);
    vec3 specular = spec * u_material.specular;

    fragColor = vec4(u_lightColor, 1.0);
 }