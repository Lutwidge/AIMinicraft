#version 400

//Variables en entree
in vec3 normal;
in vec4 color;
in vec2 uv;
flat in int type;
flat in float specLevel;

uniform vec3 lightDir;
uniform vec3 sunColor;
uniform vec3 skyColor;
uniform vec3 camPos;
uniform float world_size;

in vec3 worldPos;

uniform float elapsed;
uniform float totalTime;

uniform sampler2D colTex;

out vec4 color_out;

#define CUBE_HERBE 0.0
#define CUBE_TERRE 1.0
#define CUBE_PIERRE 2.0
#define CUBE_EAU 3.0
#define CUBE_BRANCHES 4.0
#define CUBE_TRONC 5.0
#define CUBE_BRIQUES 14.0
#define CUBE_SABLE_01 17.0
#define CUBE_FRUIT 30.0

float noiseWater(vec3 surfPos, float time, float world_size)
{
	float dist = length(vec2(world_size/2, world_size/2)-surfPos.xy)/(world_size/2);
	float offset = - 1.5f;
	offset += 1.5f * sin(surfPos.x/5.0F + time)*pow(dist,3);
	offset += 1.5f * sin(dist*5+time)*pow(dist, 3);

	float offset2 = 0.5f * sin(surfPos.y*3+time*6)*pow(dist, 3);
	offset2 += 0.3f * sin(length(surfPos.xy)*5+time*4)*pow(dist, 3);
	offset2 *= sin(surfPos.x/5.0f + time);

	return max(offset, offset2);
}

void main()
{
	/* TEXTURES */
	vec3 up = vec3(0, 0, 1);
	vec4 colorTex;
	if (dot(normal, up) < 0.01f) // Side
	{
		colorTex = texture2D(colTex,vec2((uv.x+float(type))/32.f,uv.y/2 + 0.5)).rgba; // Mapping de la texture
	}
	else // Up / down
	{
		colorTex = texture2D(colTex,vec2((uv.x+float(type))/32.f,uv.y/2)).rgba; // Mapping de la texture
	}

	/* EAU AVEC CALCUL NOUVELLE NORMALE SI NOISE */
	vec3 normalFs = normal;
	vec4 albedo = colorTex;
	//albedo = colorTex;
	//if (type == CUBE_EAU)
	//{
//	//	// Seulement pour noise water
//	//	vec3 A = worldPos;
//	//	vec3 B = worldPos + vec3(0.2,0,0);
//	//	vec3 C = worldPos + vec3(0,0.2,0);
//	//
//	//	A.z += noiseWater(A, elapsed, world_size);
//	//	B.z += noiseWater(B, elapsed, world_size);
//	//	C.z += noiseWater(C, elapsed, world_size);
//	//
//	//	normalFs = normalize(cross(normalize(B-A), normalize(C-A)));
	//}
	//else
	//{
	//	albedo = colorTex;
	//}

	/* BLINN-PHONG */
	// Diffuse
	float diffuse = dot(normalize(lightDir), normalFs);
	diffuse = max (0, diffuse);
	vec3 colorShaded = diffuse * albedo.xyz;

	// Speculaire
	float spec = 0.0f;
	if (dot(normal, lightDir) >= 0.0)
	{
		vec3 halfVector = normalize(normalize(lightDir) + normalize(camPos - worldPos));
		spec = max(0, dot(normalFs, halfVector));
		spec = specLevel * pow(spec, 50);
	}
	if (type == CUBE_SABLE_01) // Traitement spécial du sable
	{
		if (albedo.r > 0.76)
			spec *= 2;
		else
			spec /= 2;
	}
	colorShaded += spec * sunColor;

	// Ambiante
	float ambientLevel = 0.0f;
	colorShaded += ambientLevel * pow(1-diffuse, 10) * skyColor * albedo.xyz;

	/* COULEUR FINALE */
	color_out = vec4(max(colorShaded, vec3(0.1)), albedo.a); // Pour Texture
}