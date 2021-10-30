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


layout (std430, binding=3) buffer shader_data
{ 
    uint voxels[];      
};

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
    //int col = int (colo);
    unsigned int R = (col /unsigned int(256 * 256 * 256));
    unsigned int G = ((col - (R * (256 * 256 * 256))) / (256 * 256));
    unsigned int B = ((col - (R * (256 * 256 * 256) + (G * 256 * 256))) / (256));
    unsigned int A = ((col - (R * (256 * 256 * 256) + (G * 256 * 256) + (B * 256))));
    float Rf = float(R)/256.;
    float Gf = float(G)/256.;
    float Bf = float(B)/256.;
    float Af = float(A)/256.;
    vec4 finalcolor = vec4(Rf,Gf,Bf,Af);
    return finalcolor;
}
//per ray calculations:
//  setp 1: find rotation of individual ray
//      -create an fov base should be 90                                                            - done
//      -translate fov from degrees to radians                                                      - done
//      -find camera angle, use that!                                                               - done
//      -use fragcoord (goes from -1 to 1) so at max a ray will have +/- -> fov/2 from the camera   - done 

//  step2: make the rays step until they touch something opaque
//      -find the ray position depending on the camera and their position on screen                 - done
//      -calculate the step of the ray for the x,y,z cartesian coordinates                          - done
//      -repeat 100 times unless until something is hit then return the color                       - done
/*
that will produce just such an artifact at the top and bottom, for the same reason the arctic and antarctic are stretched super wide in most maps
you want to use a camera projection which preserves straight lines, instead
here is the simplest one, which is the conventional perspective projection:
1. pick a direction you're looking, as a ray/vector.
2. pick four points/vectors that make a rectangle with that direction at the center — this is your "picture frame", if you will
3. now linearly interpolate between those four points, across the image, to get your raycast direction vectors for each pixel 
when you linearly interpolate the coordinates of the direction vectors, you get a perspective projection
interpolating angles as you are currently doing does not 
"make a rectangle" is a bit underspecified and it may be easiest to copy the perspective math that is used for setting up regular GPU drawing
you just use the same projection and view matrices, except backwards: take a screen-space pair of points with different Z, and XY corresponding to the pixel you want to compute, put them through the matrix to get world-space, and that's your ray to trace

use tan(fov) to compute the direction vectors at the edge of the image
then linearly interpolate between those two vectors
do not use an interpolated angle; that's what makes the distortion you have
*/

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
    //in case i want to add stuff
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
        //color = vec4(float(i)/times, 0.,0.,1.);
        i++;
    }
    if (touched == false){color = vec4(0.,0.,1.,1);} 
    return color;
}
vec4 raycast()
{
    vec3 RayStep = FindRayStep();
    vec3 rayPosition = FindRayPosition();
    vec4 finalcolor = CastRay(700,rayPosition,RayStep);
    return finalcolor;
}


void main()
{   
    FragColor = vec4(0.,0.,0.,0.);
    FragColor = raycast();
    //blend(vec4(0.5,1,1,0.05/dist(correction(FragCoord),correction(Mouse))));        //this isn't important

    dither(2);
}
