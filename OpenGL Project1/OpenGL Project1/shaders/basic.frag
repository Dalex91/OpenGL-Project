#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
//lamp
uniform vec3 lampLightColor;
uniform vec3 lampLightPosition;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
//purple lamp
uniform vec3 purpleLampLightColor;
uniform vec3 purpleLampLightPosition;
// transparency and fog
uniform float fogDensity;
uniform float opacity;
//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;

//components
vec3 lampAmbient;
vec3 lampDiffuse;
vec3 lampSpecular;
float lampSpecularStrength = 0.5;

//components
vec3 purpleLampAmbient;
vec3 purpleLampDiffuse;
vec3 purpleLampSpecular;
float purpleLampSpecularStrength = 0.2;

//constants for computing light
float cnst = 0.05;
float linear = 0.1;
float quad = 0.1;

vec4 fogColor = vec4(0.5, 0.5, 0.5, 1);

void computeDirLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;
}

void computePointLight() {
    vec3 normalEye = normalize(normalMatrix * fNormal);	
    vec3 lightDirN = normalize(lampLightPosition - fPosition.xyz);    
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);

    float distanceToLight = length(lampLightPosition - fPosition.xyz);
    vec3 viewDir = normalize (-fPosEye.xyz);

    float atenuation = cnst + linear * distanceToLight + quad * distanceToLight * distanceToLight;

    lampAmbient = lampLightColor / atenuation;
   
    lampDiffuse = (max(dot(normalEye, lightDirN), 0.0f) * lampLightColor) / atenuation;
    vec3 reflection = reflect(-lightDirN, normalEye);
    float specCoefficient = pow(max(dot(viewDir, reflection), 0.0f), 0.32f);
    lampSpecular = (specularStrength * specCoefficient * lampLightColor) / atenuation;
}

void computePurplePointLight() {
    vec3 normalEye = normalize(normalMatrix * fNormal);	
    vec3 lightDirN = normalize(purpleLampLightPosition - fPosition.xyz);    
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);

    float distanceToLight = length(purpleLampLightPosition - fPosition.xyz);
    vec3 viewDir = normalize (-fPosEye.xyz);

    float atenuation = cnst + linear * distanceToLight + quad * distanceToLight * distanceToLight;

    purpleLampAmbient = purpleLampLightColor / atenuation;
   
    purpleLampDiffuse = (max(dot(normalEye, lightDirN), 0.0f) * purpleLampLightColor) / atenuation;
    vec3 reflection = reflect(-lightDirN, normalEye);
    float specCoefficient = pow(max(dot(viewDir, reflection), 0.0f), 0.32f);
    purpleLampSpecular = (specularStrength * specCoefficient * purpleLampLightColor) / atenuation;
}

float computeFog(){
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    float dist = length(fPosEye);
    float fogFactor = exp(-pow(dist * fogDensity, 2));
    return clamp(fogFactor, 0.0f, 1.0f);
}

void main() {
    computeDirLight();
    computePointLight();
	computePurplePointLight();
	float fogFactor = computeFog();
    //compute final vertex color
    vec3 color = min((ambient + diffuse + lampAmbient + lampDiffuse + purpleLampAmbient + purpleLampDiffuse) * texture(diffuseTexture, fTexCoords).rgb + (lampSpecular + specular + purpleLampSpecular) * texture(specularTexture, fTexCoords).rgb, 1.0f);
    fColor = vec4(color, 1.0f);
	fColor = mix(fogColor, fColor, fogFactor);
	fColor.w = opacity;
}
