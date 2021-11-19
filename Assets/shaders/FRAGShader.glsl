//----------Made by clemdemort----------//
#version 430 core
out vec4 FragColor;
uniform int fov;
in vec3 ourColor;
in vec2 FragCoord;

uniform vec3 rectRayUPRIGHT;
uniform vec3 rectRayUPLEFT;
uniform vec3 rectRayDOWNLEFT;
uniform vec3 rectRayDOWNRIGHT;
uniform float iTime;
uniform float ElapsedTime;
uniform vec2 Mouse;
uniform vec3 CameraPos;
uniform vec2 CameraRot;
uniform vec3 CameraDir;
uniform vec2 iResolution;         //twitter moment
uniform vec2 Screen;
uniform vec3 voxellist;
const int MAX_RAY_STEPS = 300;
//here is my voxel data please be nice, it's very sensitive
layout (std430, binding=3) buffer shader_data
{ 
    uint voxels[];      
};
//the almighty one
float PI = 3.142857;

//this dithering function removes banding
void dither(float power){
        FragColor += vec4(0.001*power*cos(FragCoord.y*1000), 0.001*power*sin(FragCoord.y*1000), 0.001*power*cos(FragCoord.x*1000), 0);
        FragColor -= vec4(0.001*power*cos(FragCoord.x*1000), 0.001*power*sin(FragCoord.x*1000), 0.001*power*cos(FragCoord.y*1000), 0);
}

//the index function used to access an element in the "voxels" array
int IDX(ivec3 pos){
    int id = int((pos.z*voxellist.y*voxellist.x) + (pos.y*voxellist.x) + pos.x);
    return id;
}

vec4 DeCompressCol(uint col)
{   
    //this code decompresses the voxel Data into various colours, as of yet it support 256^4 RGBA colours 
    //which is a bit dumb so i'll reduce it to 128^4 and add different effects like reflection or light emmision
    //and use it in my Signed Distance Field!!!
    unsigned int R = (col /unsigned int(256 * 256 * 256));                              //gets red hue
    unsigned int G = ((col - (R * (256 * 256 * 256))) / (256 * 256));                   //gets green hue
    unsigned int B = ((col - (R * (256 * 256 * 256) + (G * 256 * 256))) / (256));       //gets blue hue
    unsigned int A = ((col - (R * (256 * 256 * 256) + (G * 256 * 256) + (B * 256))));   //gets alpha hue
    float Rf = float(R)/256.;   //normalisation time yaay
    float Gf = float(G)/256.;
    float Bf = float(B)/256.;
    float Af = float(A)/256.;
    vec4 finalcolor = vec4(Rf,Gf,Bf,Af);    //and finaly return the colour
    return finalcolor;
}

vec4 getVoxel(ivec3 c) {
	vec3 p = vec3(c) + vec3(0.5);
	return DeCompressCol(voxels[IDX(c)]);
}

vec2 rotate2d(vec2 v, float a) {
	float sinA = sin(a);
	float cosA = cos(a);
	return vec2(v.x * cosA - v.y * sinA, v.y * cosA + v.x * sinA);	
}

vec3 rotate3d(vec3 v, float a, float b) {
	
	float sinA = sin(a);
	float cosA = cos(a);
	float sinB = sin(b);
	float cosB = cos(b);
	mat3 rotationA = mat3(
	1               ,  0            ,    0                  ,
    0               ,  cosA         ,    -sinA              ,
    0               ,  sinA         ,    cosA	
	);
	mat3 rotationB = mat3(
	cosB               ,  0                ,    sinB      ,
    0                      ,  1                ,    0             ,
    -sinB              ,  0                ,    cosB	
	);
	return v*rotationA*rotationB;	
}

void main()
{
	vec2 screenPos = (FragCoord.xy / vec2(iResolution.x/iResolution.y,1.));
	vec3 cameraDir = vec3(0.0, 0.0, 0.8);
	vec3 cameraPlaneU = vec3(1.0, 0.0, 0.0);
	vec3 cameraPlaneV = vec3(0.0, 1.0, 0.0) * iResolution.y / iResolution.x;
	vec3 rayDir = cameraDir + screenPos.x * cameraPlaneU + screenPos.y * cameraPlaneV;
	vec3 rayPos = CameraPos;
	
	rayDir = rotate3d(rayDir,CameraRot.y,CameraRot.x);

	ivec3 mapPos = ivec3(floor(rayPos + 0.));

	vec3 deltaDist = abs(vec3(length(rayDir)) / rayDir);
	
	ivec3 rayStep = ivec3(sign(rayDir));

	vec3 sideDist = (sign(rayDir) * (vec3(mapPos) - rayPos) + (sign(rayDir) * 0.5) + 0.5) * deltaDist; 
	
    vec3 color;

	bvec3 mask;
	bool touched = false;
	int i;
	while(touched == false && i < MAX_RAY_STEPS)
	{
		if (sideDist.x < sideDist.y) {
				if (sideDist.x < sideDist.z) {
					sideDist.x += deltaDist.x;
					mapPos.x += rayStep.x;
					mask = bvec3(true, false, false);
				}
				else {
					sideDist.z += deltaDist.z;
					mapPos.z += rayStep.z;
					mask = bvec3(false, false, true);
				}
			}
			else {
				if (sideDist.y < sideDist.z) {
					sideDist.y += deltaDist.y;
					mapPos.y += rayStep.y;
					mask = bvec3(false, true, false);
				}
				else {
					sideDist.z += deltaDist.z;
					mapPos.z += rayStep.z;
					mask = bvec3(false, false, true);
				}
			}	
		    if(mapPos.x > 0 && mapPos.y > 0 && mapPos.z > 0 && mapPos.x <= voxellist.x-1 && mapPos.y <= voxellist.y-1 && mapPos.z <= voxellist.z-1)
			{
				//color += vec3(0.001); //this is for debbuging
				
				if(getVoxel(mapPos).w < 0.5f){touched = true;}
			}
		i++;
	}
	
	
    if(touched == true){color = getVoxel(mapPos).xyz;}
	if (mask.x) {
		color *= vec3(0.5);
	}
	if (mask.y) {
		color *= vec3(1.0);
	}
	if (mask.z) {
		color *= vec3(0.75);
	}

	const mat3 RGBtoCIEmat = mat3(0.412453, 0.212671, 0.019334,
                              0.357580, 0.715160, 0.119193,
                              0.180423, 0.072169, 0.950227);

	vec3 cieColor = color * RGBtoCIEmat;

	FragColor.xyz = color;
	dither(2);
}
