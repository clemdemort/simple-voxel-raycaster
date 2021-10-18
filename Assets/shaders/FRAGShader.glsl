//made by clemdemort
#version 430 core
out vec4 FragColor;
uniform int fov;
in vec3 ourColor;
in vec2 FragCoord;

//Defining uniforms
//-------------------------------------------
uniform float iTime;
uniform vec2 Mouse;
uniform vec3 CameraPos;
uniform vec2 CameraRot;
uniform vec2 ratio;         //twitter moment
uniform vec2 Screen;
uniform vec3 voxellist;
//this could probably be done another way but eh it works
uniform vec3 rectRayUPLEFT;
uniform vec3 rectRayUPRIGHT;
uniform vec3 rectRayDOWNRIGHT;
uniform vec3 rectRayDOWNLEFT;
//getting the array of voxels from the main file through an SSBO
//-------------------------------------------
layout (std430, binding=3) buffer shader_data
{ 
    float voxels[];      
};

float PI = 3.142857;	//PI in case we need it

//corrects distortion due to screen size
vec2 correction(vec2 coord){
    return vec2(coord*ratio);
}
//a basic distance calculator between two vec2
float dist(vec2 point,vec2 to){
    float dist = sqrt(((to.x-point.x)*(to.x-point.x))+((to.y-point.y)*(to.y-point.y)));
    return dist;
}
//blends colors together! use the w component of the color vec4 (alpha) to scale this effect
vec4 blend(vec4 color1, vec4 color2){

    return vec4(((color1*color1.w)+(color2*color2.w))/(color1.w+color2.w));
}
//this dithering function removes banding
void dither(float power){
        FragColor += vec4(0.001*power*cos(FragCoord.y*1000),0.001*power*sin(FragCoord.y*1000),0.001*power*cos(FragCoord.x*1000),0);
        FragColor -= vec4(0.001*power*cos(FragCoord.x*1000),0.001*power*sin(FragCoord.x*1000),0.001*power*cos(FragCoord.y*1000),0);
}
//the index function used to access an element in the "voxels" array
int IDX(int x,int y,int z, int c){
    int id = int((z*voxellist.y*voxellist.x*4) + (y*voxellist.x*4) + x*4 + c);

    return id;
}
//per ray calculations:
//-------------------------------------------------------
vec3 FindRayStep()
{
    float DistTo1 = ((1.0f+FragCoord.x)+(1.0f+FragCoord.y))/2;
    float DistTo2 = ((1.0f-FragCoord.x)+(1.0f+FragCoord.y))/2;
    float DistTo3 = ((1.0f-FragCoord.x)+(1.0f-FragCoord.y))/2;
    float DistTo4 = ((1.0f+FragCoord.x)+(1.0f-FragCoord.y))/2;
     
    vec3 Step;
    Step = 0.25*((DistTo1) * rectRayUPRIGHT + (DistTo2) * rectRayUPLEFT + (DistTo3) * rectRayDOWNLEFT + (DistTo4) * rectRayDOWNRIGHT);
    return Step;
}
vec3 FindRayPosition()
{
     float x,y,z;
     x = CameraPos.x;
     y = CameraPos.y;
     z = CameraPos.z;
    

     vec3 p = vec3(x,y,z);
     return p;
}
vec4 CastRay(int times, vec3 position, vec3 Step)
{
    bool touched = false;
    int i = 1;
    float xf, yf ,zf;
    int x , y , z;
    vec4 color = vec4(0.,0.3,0.8,1);
    while(touched == false && i < times)
    {   
        xf = position.x+(i*Step.x);     x = int(xf);
        yf = position.y+(i*Step.y);     y = int(yf);
        zf = position.z+(i*Step.z);     z = int(zf);
        if(x > 0 && y > 0 && z > 0 && x <= voxellist.x && y <= voxellist.y && z <= voxellist.z && voxels[IDX(x,y,z,3)] > 0.0f){
            float dist  =    sqrt(((xf-CameraPos.x)*(xf-CameraPos.x))+((yf-CameraPos.y)*(yf-CameraPos.y))+((zf-CameraPos.z)*(zf-CameraPos.z)))/5;
            color       =    vec4(voxels[IDX(x,y,z,0)]/dist,voxels[IDX(x,y,z,1)]/dist,voxels[IDX(x,y,z,2)]/dist,voxels[IDX(x,y,z,3)]);
            touched     =    true;
        }
        

        i++;
    }
    return color;
}
vec4 raycast()
{
    
    vec3 RayStep = FindRayStep();
    vec3 rayPosition = FindRayPosition();
    vec4 finalcolor = CastRay(600,rayPosition,RayStep);
    return finalcolor;
}


void main()
{   
    FragColor = vec4(0.,0.3,0.8,0.);
    FragColor = raycast();
    //blend(vec4(0.5,1,1,0.05/dist(correction(FragCoord),correction(Mouse))));        //this isn't important

    dither(2);
}
