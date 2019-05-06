#version 400

// Variables en entrée
uniform float elapsed;
uniform mat4 m; // Données par les sendmatricestoshader
uniform mat4 v;
uniform mat4 p;
uniform mat4 nmat;
uniform float world_size; // A fournir avec uniform variables dans engine_minicraft
uniform mat4x4 shadowV; // Shadow cam V
uniform mat4x4 shadowP; // Shadow cam P

layout(location=0) in vec3 vs_position_in;
layout(location=1) in vec3 vs_normal_in;
layout(location=2) in vec2 vs_uv_in;
layout(location=3) in float vs_type_in;

//Variables en sortie
out vec3 normal;
out vec4 color;
out vec2 uv;
flat out int type; // out : ce qu'on sort du vertex et ce que fragment va récupérer + flat pour pas d'interpolation !
out vec3 worldPos; // On le récupère dans fragment shader
flat out float specLevel;

#define PI 3.1415926535897932384626433832795
#define CUBE_HERBE 0.0
#define CUBE_TERRE 1.0
#define CUBE_PIERRE 2.0
#define CUBE_EAU 3.0
#define CUBE_BRANCHES 4.0
#define CUBE_TRONC 5.0
#define CUBE_BRIQUES 14.0
#define CUBE_SABLE_01 17.0
#define CUBE_FRUIT 30.0

const vec4 CubeColors[36] = vec4[36](
	vec4(0,1,0,1),
	vec4(0.2,0.1,0,1),
	vec4(0.2,0.1,0,1),
	vec4(0.2,0.2,1.0,0.9),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0,1,0,1),
	vec4(0.2,0.1,0,1),
	vec4(0.2,0.1,0,1),
	vec4(0.2,0.2,1.0,0.9),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1),
	vec4(0, 1, 0, 1)
);

const float SpecLevels[36] = float[36](
	0.1f,
	0.1f,
	0.1f,
	1.0f,
	0.1f,
	0.1f,
	0.1f,
	0.1f,
	0.1f,
	0.1f,
	0.1f,
	0.1f,
	0.1f,
	0.1f,
	0.1f,
	0.1f,
	0.1f,
	0.1f,
	0.1f,
	0.1f,
	0.1f,
	1.0f,
	0.1f,
	0.1f,
	0.1f,
	0.1f,
	0.1f,
	0.1f,
	0.1f,
	0.1f,
	0.1f,
	0.1f,
	0.1f,
	0.1f,
	0.1f,
	0.1f
);

float noiseWater(vec3 surfPos, float time, float world_size) // Hautes freqs seulement ici
{
	float dist = length(vec2(world_size/2, world_size/2)-surfPos.xy)/(world_size/2);
	float offset = - 1.5f;
	offset += 1.5f * sin(surfPos.x/5.0F + time)*pow(dist,3);
	offset += 1.5f * sin(dist*5+time)*pow(dist, 3);
	return offset;
}

vec3 gerstnerWater(vec3 surfPos, float time, float steepness, float wavelength, vec2 direction, inout vec3 tangent, inout vec3 binormal)
{
	float k = 2 * PI / wavelength;
	float c = sqrt(9.8 / k);
	vec2 d = normalize(direction);
	float f = k * (dot(d, surfPos.xy) - c * time);
	float a = steepness / k;

	tangent += vec3(
		 - d.x * d.x * steepness * sin(f),
		 d.x * steepness * cos(f),
		 -d.x * d.y * steepness * sin(f)
	);

	binormal += vec3(
		 - d.x * d.y * steepness * sin(f),
		 d.y * steepness * cos(f),
		 - d.y * d.y * steepness * sin(f)
	);

	return vec3(d.x * a * cos(f), d.y * a * cos(f), a * sin(f));
}

void main()
{
	vec4 vecIn = vec4(vs_position_in,1.0);
	vec4 vecInWorld = m * vecIn; // Coordonnées monde
	worldPos = vecInWorld.xyz;

	/* EAU */
	normal = (nmat * vec4(vs_normal_in, 1.0)).xyz;
	//if (vs_type_in == CUBE_EAU)
	//{
	//	// Noise water
	//	//vecInWorld.z += noiseWater(vecInWorld.xyz,elapsed,world_size);
	//
	//	// Gerstner water
	//	vec3 tangent = vec3(1, 0, 0);
	//	vec3 binormal = vec3(0, 0, 1);
	//	vec3 gerstnerWave1 = gerstnerWater(vecInWorld.xyz, elapsed, 0.02f, 1, vec2(1,1), tangent, binormal);
	//	vec3 gerstnerWave2 = gerstnerWater(vecInWorld.xyz, elapsed, 0.025f, 1, vec2(1,0), tangent, binormal);
	//	vec3 gerstnerWave3 = gerstnerWater(vecInWorld.xyz, elapsed, 0.005f, 1, vec2(0,1.4), tangent, binormal);
	//	vecInWorld.z -= 1.0f;
	//	vecInWorld.xyz += gerstnerWave1;
	//	vecInWorld.xyz += gerstnerWave2;
	//	vecInWorld.xyz += gerstnerWave3;
	//
	//	normal = normalize(cross(binormal, tangent));
	//}
	//else
	//{
	//	normal = (nmat * vec4(vs_normal_in, 1.0)).xyz; // Applique rotations aux normales, pas utile ici
	//}

	vec4 vecInView = v * vecInWorld; // Repère vue

	/* AUTRES SORTIES */
	uv = vs_uv_in;

	///* DISTORTION DU MONDE */
	//vecInView.y -= pow(length(vecInView.xyz)/100, 3);
	gl_Position = p * vecInView;

	/* COULEUR */
	color = vec4(1.0,1.0,0.0,1.0); // Couleur par défaut violet
	specLevel = SpecLevels[int(vs_type_in)];
	type = int(vs_type_in);
	color = CubeColors[int(vs_type_in)]; // Couleur fonction du type
}