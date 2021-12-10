#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <sstream>
#include "shader.h"
#include "TimeSync.h"
#include <memory>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void setTitle(float Dspeed, GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 700;
const unsigned int SCR_HEIGHT = 700;
int screenX = SCR_WIDTH;
int screenY = SCR_HEIGHT;
float PI = 3.142857;
float speed;float latspeed;
//time variables
//--------------
float iTime;
float fps1;
float fps2;
float fpsclock;
float fpsTime;

//initialise the camera positionnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn
//------------------------------
float camX = 20, camY = 40, camZ = 20, rotX = 0, rotY = 0, rotZ = 0;

int main()
{
    
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Opengl project by clemdemort", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    Shader ourShader("Assets/shaders/VERTShader.glsl", "Assets/shaders/FRAGShader.glsl"); // you can name your shader files however you like

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // positions         // colors
         3.0f, -1.0f, 1.0f,  1.0f, 0.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  // bottom left
        -1.0f,  3.0f, 1.0f,  0.0f, 0.0f, 1.0f   // top 
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //this is a my voxel world specifications
    const int pwidth = 64; const int pheight = 64; const int pdepth = 64;
    unsigned int* voxlptr = new unsigned int[pwidth * pheight * pdepth * 1];      //4 at the end represents the vec4 for colour

    for (int i = 0; i < pwidth; i++)     // a quick way to populate the scene
        for (int j = 0; j < pheight; j++)
            for (int k = 0; k < pdepth; k++)
            {
                voxlptr[(k * pwidth * pheight * 1) + (j * pwidth * 1) + i * 1 + (0)] = unsigned int((256 * 256 * 256 * int(1)) + (256 * 256 * int(1)) + (256 * int(1)) + (-1));
                //this is the equation for a funky sphere
                float condition = sqrt((32 - i) * (32 - i) + (32 - j) * (32 - j) + (32 - k) * (32 - k)) + 3 * cos((i + j) / 5.0) + 3 * cos((i + k) / 5.0) + 3 * cos((k + j) / 5.0);
                //float condition = rand() % 100;
                if (condition < 30 && condition > 26)
                {   
                    voxlptr[(k * pwidth * pheight * 1) + (j * pwidth * 1) + i * 1 + (0)] = unsigned int((256 * 256 * 256 * int(128 * (1. + (sin(i / 24.6))))) + (256 * 256 * int(128 * (1. + (sin(j / 24.6))))) + (256 * int(128 * (1. + (cos(k / 24.6))))) + (25)); //256^1
                }
                int r = rand() % 10;
                unsigned int col = voxlptr[(k * pwidth * pheight * 1) + (j * pwidth * 1) + i * 1 + (0)];

            }

    TimeSync Vsync; //video sync
    TimeSync Msync; //map sync

    
    // render loop
    // -----------
    GLuint ssbo = 0;    //declaring my SSBO (has to be done somewhere...)
    while (!glfwWindowShouldClose(window))
    {
        iTime = glfwGetTime()*15;
        if (Msync.Sync(30))
        {
            //THIS LOOP IS MEANT FOR THE MAP UPDATE AND THE MAP UPDATES ONLY FUTURE ME PLEASE DONT SCREW IT UP!!!
            //collecting el garbage
            //---------------------
            glDeleteBuffers(1, &ssbo);
            ssbo = 0;

            //for testing purposes this is updating the map dynamically every 30th of a second for now this works more than fine but soon i'll have to implement chunking to go alongside it.
            //-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
            for (int i = 0; i < pwidth; i++)
                for (int j = 0; j < pheight; j++)
                    for (int k = 0; k < pdepth; k++)
                    {
                        voxlptr[(k * pwidth * pheight * 1) + (j * pwidth * 1) + i * 1 + (0)] = unsigned int((256 * 256 * 256 * int(1)) + (256 * 256 * int(1)) + (256 * int(1)) + (-1));
                        //this is the equation for a funky sphere
                        float condition = sqrt((32 - i) * (32 - i) + (32 - j) * (32 - j) + (32 - k) * (32 - k)) + 1.5 * sin((i + j + iTime) / 5.0) + 1.5 * cos((i + k + iTime) / 5.0) + 1.5 * cos((k + j + iTime) / 5.0);
                        if (condition < 28 && condition > 26)
                        {   //by adding the time variable in the equation we get a shape influenced by time
                            voxlptr[(k * pwidth * pheight * 1) + (j * pwidth * 1) + i * 1 + (0)] = unsigned int((256 * 256 * 256 * int(128 * (1. + (sin((i + iTime) / 24.6))))) + (256 * 256 * int(128 * (1. + (sin((j + iTime) / 24.6))))) + (256 * int(128 * (1. + (cos((k + iTime) / 24.6))))) + (25)); //256^1
                        }

                    }

            
            //this is how i transfer the contents of my array to my shader
            //------------------------------------------------------------
            int arrSize = (4 * pwidth * pheight * pdepth);
            glGenBuffers(1, &ssbo);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
            glBufferData(GL_SHADER_STORAGE_BUFFER, arrSize, voxlptr, GL_DYNAMIC_COPY);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
            GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
            memcpy(p, voxlptr, arrSize);
            //------------------------------------------------------------
        }
        if (Vsync.Sync(60))
        {   
            //this loop is meant for anything that has to be updated at the same time as the screen(60 times a second)
            
            setTitle(0.25,window);//this is a bad function, it will change soon(tm)
            
            // input
            // -----
            processInput(window);
            
            /*gets cursor position -->*/
            double x,y;
            glfwGetCursorPos(window, &x, &y);
            ourShader.setV2Float("Mouse", (float)(x - (screenX / 2)) * (1.0f / (screenX / 2)), (float)(y - (screenY / 2)) * (-1.0f / (screenY / 2)));

            ourShader.setV3Float("voxellist", (float)pwidth, (float)pheight, (float)pdepth);
            ourShader.setFloat("iTime", iTime);
            ourShader.setFloat("ElapsedTime", fpsTime);
            ourShader.setV3Float("CameraPos", camX, camY, camZ);
            ourShader.setV2Float("CameraRot", rotX, rotY);
            ourShader.setV2Float("iResolution", (float)screenX, (float)screenY);
            ourShader.setV2Float("Screen", screenX,screenY);
            
            float FOV = 70; //useless as of yet but ill give it a use in the future *maybe*

       

            // render the shader
            ourShader.use();
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(window);
            glfwPollEvents();
            
        }

    }

    // de-allocate all resources once they've outlived their purpose bye bye!
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &ssbo);
    delete[] voxlptr;
    std::cout << "program ran for: " << glfwGetTime() << " seconds" << std::endl;
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    rotX = x/screenX;
    rotY = y/screenY;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        float force = 30;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            force *= force/2.0f;
        speed += force * fpsTime;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        float force = 30;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            force *= force/2.0f;
        speed += -force * fpsTime;
    }
    
    camX += fpsTime * speed * sin(rotX) * cos(rotY);
    camY -= fpsTime * speed * sin(rotY);
    camZ += fpsTime * speed * cos(rotX) * cos(rotY);

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        float force = 30;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            force *= force / 2.0f;
        latspeed -= force * fpsTime;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        float force = 30;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            force *= force/2.0f;
        latspeed += force * fpsTime;
    }

    camX += fpsTime * latspeed * sin(rotX + PI / 2);
    camZ += fpsTime * latspeed * cos(rotX + PI / 2);

    
        
    
    speed -= fpsTime * 2.0 * speed;
    latspeed -= fpsTime * 2.0 * latspeed;
    

    if (!glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    {
        std::cout << "X:" << camX << std::endl;
        std::cout << "Y:" << camY << std::endl;
        std::cout << "Z:" << camZ << std::endl;    
        std::cout << "rotationX:" << rotX << std::endl;    
        std::cout << "rotationY:" << rotY << std::endl;    
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    screenX = width;screenY = height;
}

void setTitle(float Dspeed, GLFWwindow* window)
{
    //this function is sadly no longer accurate so it'll have to go sadly...
    fps2 = glfwGetTime();
    fpsTime = fps2 - fps1;
    fps1 = fps2;
    fpsclock += fpsTime;
    //will set the fps counter to 1/fpsclock
    if (Dspeed < fpsclock) {
        int fps = 1 / fpsTime;
        fpsclock = 0;
        std::stringstream ss;
        ss << fps;
        std::string temp = ss.str();
        std::string temp2 = "Simple Voxel Raycaster -FPS:" + temp;
        char* FPS = (char*)temp2.c_str();
        glfwSetWindowTitle(window, FPS);
    }
}