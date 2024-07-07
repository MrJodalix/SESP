#version 410 core

/**
 * postProcess Shader.
 * Rechnet die Bildqualitaet von HDR auf LDR zurueck
 * und wendet die Gamma Korrektur an.
 * 
 * Copyright (C) 2023, FH Wedel
 * Autor: Joshua-Scott Schoettke, Ilana Schmara
 */

// Nur die Farbe wird ausgegeben
out vec4 FragColor;
 
/* 
 * Der Exposure Wert fuer das Tone Mapping
 * Kann ueber die Gui veraendert werden
 */
uniform float u_exposure;

/* 
 * Der Gamma Korrekturwert
 * Kann ueber die Gui veraendert werden
 */
uniform float u_gamma;

// HDR Buffer
uniform sampler2D u_final;

// Bildschirmgroesze
uniform vec2 u_screenSize;

vec2 CalcTexCoord()
{
   return gl_FragCoord.xy / u_screenSize;
} 

/*
 * Hauptfunktion
 */
 void main()
 {	
    vec2 TexCoord = CalcTexCoord();
    vec3 hdrColor = texture(u_final, TexCoord).rgb;

	// exposure tone mapping
	vec3 mapped = vec3(1.0) - exp(-hdrColor * u_exposure);
	// gamma korrektur
	mapped = pow(mapped, vec3(1.0 / u_gamma));

    FragColor = vec4(mapped, 1.0);	
 }