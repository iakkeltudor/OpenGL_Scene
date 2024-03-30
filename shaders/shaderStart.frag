#version 410 core

in vec3 fragNormal;
in vec4 fragPosEye;
in vec2 fragTexCoords;
in vec4 fragPosLightSpace;

in vec3 fragPos;

out vec4 fColor;

uniform int fogOrNot;
uniform float fogDensity;

//lighting
uniform mat3 normalMatrix;
uniform mat3 lightDirMatrix;
uniform	vec3 lightDir;
uniform	vec3 lightColor;

//spot lighting
uniform int spotLighting;

float spotQuadratic = 0.02f;
float spotLinear = 0.09f;
float spotConstant = 1.0f;

vec3 spotLightAmbient = vec3(0.0f, 0.0f, 0.0f);
vec3 spotLightSpecular = vec3(1.0f, 1.0f, 1.0f);
vec3 spotLightColor = vec3(12,12,12);

uniform float spotlight;
uniform float spotlight2;

uniform vec3 spotLightDirection;
uniform vec3 spotLightPosition;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

vec3 computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(fragNormal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDir);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fragPosEye.xyz);
		
	//compute ambient light
	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;

	return (ambient + diffuse + specular);
}

vec3 computeSpotLightComponents() 
{
	vec3 cameraPosEye = vec3(0.0f);
	vec3 lightDir = normalize(spotLightPosition - fragPos);
	vec3 normalEye = normalize(normalMatrix * fragNormal);
	vec3 lightDirN = normalize(lightDirMatrix * lightDir);
	vec3 viewDirN = normalize(cameraPosEye - fragPosEye.xyz);
	vec3 halfVector = normalize(lightDirN + viewDirN);

	float diff = max(dot(fragNormal, lightDir), 0.0f);
	float spec = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
	float distance = length(spotLightPosition - fragPos);
	float attenuation = 1.0f / (spotConstant + spotLinear * distance + spotQuadratic * distance * distance);

	float theta = dot(lightDir, normalize(-spotLightDirection));
	float epsilon = spotlight - spotlight2;
	float intensity = clamp((theta - spotlight2)/epsilon, 0.0, 1.0);

	vec3 ambient = spotLightColor * spotLightAmbient * vec3(texture(diffuseTexture, fragTexCoords));
	vec3 diffuse = spotLightColor * spotLightSpecular * diff * vec3(texture(diffuseTexture, fragTexCoords));
	vec3 specular = spotLightColor * spotLightSpecular * spec * vec3(texture(specularTexture, fragTexCoords));
	ambient *= attenuation * intensity;
	diffuse *= attenuation * intensity;
	specular *= attenuation * intensity;
	
	return ambient + diffuse + specular;
}

float computeFog()
{
	//float fogDensity = 0.05f;
	float fragmentDistance = length(fragPosEye);
	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

	return clamp(fogFactor, 0.0f, 1.0f);

}

float computeShadow()
{	
	// perform perspective divide
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    
	// Transform to [0,1] range
    normalizedCoords = normalizedCoords * 0.5f + 0.5f;
   
   // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;    
   
   // Get depth of current fragment from light's perspective
    float currentDepth = normalizedCoords.z;
   
   // Check whether current frag pos is in shadow
    float bias = 0.005f;
    float shadow = currentDepth - bias > closestDepth  ? 1.0f : 0.0f;

	if(normalizedCoords.z > 1.0f) {
        return 0.0f;
	}

    return shadow;	
}

void main() 
{
	vec3 light = computeLightComponents();
	vec3 baseColor = vec3(0.9f, 0.35f, 0.0f);//orange
	
	float shadow = computeShadow();

	float fogFactor = computeFog();
	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);

	
	// modulate with diffuse map
	ambient *= texture(diffuseTexture, fragTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fragTexCoords).rgb;
	// modulate with specular map
	specular *= texture(specularTexture, fragTexCoords).rgb;

	if (spotLighting == 1) {
		light += computeSpotLightComponents();
	}
	
	
	// modulate with shadow
	vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow) * specular, 1.0f);

	vec4 colorWithShadow = vec4(color, 1.0f);

	if ( fogOrNot == 0)
	{

		fColor = min(colorWithShadow * vec4(light, 1.0f), 1.0f);
	}
	else
	{
 
		fColor = mix(fogColor, min(colorWithShadow * vec4(light, 1.0f), 1.0f), fogFactor);
	}

	
}
