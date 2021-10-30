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
uniform vec2 ratio;         //twitter moment
uniform vec2 Screen;
uniform vec3 voxellist;

//here is my voxel data please be nice, it's very sensitive :p
layout (std430, binding=3) buffer shader_data
{ 
    uint voxels[];      
};
//the almighty one
float PI = 3.142857;


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
int IDX(int x,int y,int z){
    int id = int((z*voxellist.y*voxellist.x) + (y*voxellist.x) + x);
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

vec3 FindRayStep()
{
    //bi-linearly interpolates the ray steps
    float DistTo1 = ((1.0f+FragCoord.x)+(1.0f+FragCoord.y))/2;  //gets distance for each pixel to the screen corner
    float DistTo2 = ((1.0f-FragCoord.x)+(1.0f+FragCoord.y))/2;
    float DistTo3 = ((1.0f-FragCoord.x)+(1.0f-FragCoord.y))/2;
    float DistTo4 = ((1.0f+FragCoord.x)+(1.0f-FragCoord.y))/2;
     
    vec3 Step;  //based on that distance it will set it's direction
    Step = 0.25*((DistTo1) * rectRayUPRIGHT + (DistTo2) * rectRayUPLEFT + (DistTo3) * rectRayDOWNLEFT + (DistTo4) * rectRayDOWNRIGHT);
    return Step;
}
vec3 FindRayPosition()
{
    //in case i want to add stuff but yeah i know over engeneering blah blah...
     vec3 p = CameraPos;
     return p;
}
vec4 CastRay(int times, vec3 position, vec3 Step)
{
    bool touched = false;
    int i = 1;
    float xf, yf ,zf;
    int x , y , z;
    vec4 color = vec4(0.,0.,1.,1);
    while(touched == false && i < times)
    {   
        xf = position.x+(i*(Step.x*i/100));     x = int(xf);
        yf = position.y+(i*(Step.y*i/100));     y = int(yf);
        zf = position.z+(i*(Step.z*i/100));     z = int(zf);
        color       =    vec4(DeCompressCol(voxels[IDX(x,y,z)]));
        if(x > 0 && y > 0 && z > 0 && x <= voxellist.x-1 && y <= voxellist.y-1 && z <= voxellist.z-1 && color.w < 0.5f){
            float dist  =    sqrt(((xf-CameraPos.x)*(xf-CameraPos.x))+((yf-CameraPos.y)*(yf-CameraPos.y))+((zf-CameraPos.z)*(zf-CameraPos.z)))/5;
            touched     =    true;
            color       =    (vec4(DeCompressCol(voxels[IDX(x,y,z)])*50)+vec4(0,0,1,1)*(dist))/(50+dist);
        }
        //color = vec4(float(i)/times, 0.,0.,1.); //this is to see the cost of each ray, the more red the more impactfull on the performance
        i++;
    }
    if (touched == false){color = vec4(0.,0.,1.,1);} 
    return color;
}
vec4 raycast()
{
    vec3 RayStep = FindRayStep();                       //finds the step for each ray
    vec3 rayPosition = FindRayPosition();               //finds the ray position(pretty much the camera position but if i want to do some funky stuff i'll keep it)
    vec4 finalcolor = CastRay(700,rayPosition,RayStep); //casts the ray then the ray will return an RGBA colour
    return finalcolor;                                  //returns the colour the ray returned and sets it as the colour of the fragment
}


void main()
{   
    FragColor = vec4(0.,0.,0.,0.);
    FragColor = raycast();
    //blend(vec4(0.5,1,1,0.05/dist(correction(FragCoord),correction(Mouse))));        //this isn't important but i want to keep it

    dither(2);
}
