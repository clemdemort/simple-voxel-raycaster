//----------Made by clemdemort----------//
#version 430 core
out vec4 FragColor;
in vec3 ourColor;
in vec2 FragCoord;

uniform float iTime;
uniform float FOV;
uniform float ElapsedTime;
uniform vec2 Mouse;
uniform vec3 CameraPos;
uniform vec3 CameraRot;
uniform vec3 CameraDir;
uniform vec2 iResolution;
uniform vec2 Screen;
uniform vec3 voxellist;
const int MAX_RAY_STEPS = 100;
layout(r32ui, binding = 0) uniform uimage3D voxels;//gets the binded texture



//the life of
float PI = 3.142857;

//this dithering function removes banding
void dither(float power){
        FragColor += vec4(0.001*power*cos(FragCoord.y*1000), 0.001*power*sin(FragCoord.y*1000), 0.001*power*cos(FragCoord.x*1000), 0);
        FragColor -= vec4(0.001*power*cos(FragCoord.x*1000), 0.001*power*sin(FragCoord.x*1000), 0.001*power*cos(FragCoord.y*1000), 0);
}

vec4 DeCompressCol(uint col)
{   
    //this code decompresses the voxel Data into various colours, as of yet it support 256^4 RGBA colours 
    //which is a bit dumb so i'll reduce it to 128^4 and add different effects like reflection or light emmision
    //and use it in my Signed Distance Field eventually...
    unsigned int R = (col /	 (256 * 256 * 256));										//gets red hue
    unsigned int G = ((col - (R * (256 * 256 * 256))) / (256 * 256));                   //gets green hue
    unsigned int B = ((col - (R * (256 * 256 * 256) + (G * 256 * 256))) / (256));       //gets blue hue
    unsigned int A = ((col - (R * (256 * 256 * 256) + (G * 256 * 256) + (B * 256))));   //gets alpha hue
    float Rf = float(R)/256.;   //normalisation time yaay
    float Gf = float(G)/256.;
    float Bf = float(B)/256.;
    float Af = float(A)/256. ;	// divide by 256. 
    vec4 finalcolor = vec4(Rf,Gf,Bf,Af);    //and finaly return the colour
    return finalcolor;
}

vec4 getVoxel(ivec3 c) {
	return DeCompressCol(imageLoad(voxels,c-1).r);
}

vec2 rotate2d(vec2 v, float a) {
	float sinA = sin(a);
	float cosA = cos(a);
	return vec2(v.x * cosA - v.y * sinA, v.y * cosA + v.x * sinA);	
}

vec3 rotate3d(vec3 v, float a, float b, float c) {
	
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

void cursor(vec3 color)
{
	float ratio =  iResolution.y / iResolution.x;
	float d = distance (vec2(FragCoord.x/ratio,FragCoord.y),vec2(0,0));
	if (d < 0.01 && d > 0.0075)
		FragColor.xyz = vec3(1-color.x,1-color.y,1-color.z);
}

void main()
{
	vec2 screenPos = rotate2d(vec2(FragCoord.x,FragCoord.y * (iResolution.y / iResolution.x)),CameraRot.z);
	vec3 cameraDir = vec3(0.0, 0.0, (180/FOV)-1);
	vec3 cameraPlaneU = vec3(1.0, 0.0, 0.0);
	vec3 cameraPlaneV = vec3(0.0, 1.0, 0.0);
	vec3 rayDir = cameraDir + screenPos.x * cameraPlaneU + screenPos.y * cameraPlaneV;
	vec3 rayPos = CameraPos;
	
	rayDir = rotate3d(rayDir,CameraRot.y,CameraRot.x,CameraRot.z);

	ivec3 mapPos = ivec3(floor(rayPos + 0.));

	vec3 deltaDist = abs(vec3(length(rayDir)) / rayDir);
	
	ivec3 rayStep = ivec3(sign(rayDir));

	vec3 sideDist = (sign(rayDir) * (vec3(mapPos) - rayPos) + (sign(rayDir) * 0.5) + 0.5) * deltaDist; 
	
    vec3 BGcolor = vec3(0,0,0.25);	//this is currently black but it can change any colour and still work
    vec4 color = vec4(BGcolor,0);

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
		    if(mapPos.x > 0 && mapPos.y > 0 && mapPos.z > 0 && mapPos.x <= voxellist.x && mapPos.y <= voxellist.y*2/*for showcase purposes*/ && mapPos.z <= voxellist.z)
			{
				//BGcolor += vec3(0.0005,0.0005,0.0005);
				i--;
				vec4 voxel = getVoxel(mapPos);
				if(voxel.w >= 0.99){
					color.xyz = (voxel.xyz*(1-color.w)+color.xyz*(color.w));	//if something is touched average it's colour with the current alpha(color.w)
					touched = true;
				} 
				if(voxel.w < 0.99 && voxel.w > 0){
					color.xyz = (voxel.xyz*((1-color.w))+color.xyz*(color.w));	//averaging out the current color and the added voxel color
					color.w += (1-color.w)*(voxel.w); //higher = more opaque
					}
					
				 
			}
		i++;
	}
	
	
    if(touched == false){color.xyz = (BGcolor.xyz*((1-color.w))+color.xyz*(color.w));}	//if nothing is touched then set the color to that of the background
    if(touched == true)
	{		
	//if the ray finaly touches apply the normals which are themselves affected by the alpha 
		if (mask.x) {
			color.xyz *= vec3(0.5)*(1-color.w) + color.w;
		}
		if (mask.y) {
			color.xyz *= vec3(1.0)*(1-color.w) + color.w;
		}
		if (mask.z) {
			color.xyz *= vec3(0.75)*(1-color.w) + color.w;
		}
	}
	FragColor.xyz = color.xyz;
	cursor(color.xyz);
	dither(2);
}