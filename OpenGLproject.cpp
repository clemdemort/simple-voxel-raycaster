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
TimeSync Vsync; //video sync
TimeSync Msync; //map sync
TimeSync Titlesync; //map sync

//initialise the camera position
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
                voxlptr[(k * pwidth * pheight * 1) + (j * pwidth * 1) + i * 1 + (0)] = unsigned int((256 * 256 * 256 * int(1)) + (256 * 256 * int(1)) + (256 * int(1)) + (0/*opacity is 0 so it's air basically*/));
                //this is the equation for a funky sphere
                float condition = sqrt((32 - i) * (32 - i) + (32 - j) * (32 - j) + (32 - k) * (32 - k)) + 3 * cos((i + j) / 5.0) + 3 * cos((i + k) / 5.0) + 3 * cos((k + j) / 5.0);
                //float condition = rand() % 100;
                if (condition < 30 && condition > 26)
                {   
                    voxlptr[(k * pwidth * pheight) + (j * pwidth) + i] = unsigned int((256 * 256 * 256 * int(128 * (1. + (sin(i / 24.6))))) + (256 * 256 * int(128 * (1. + (sin(j / 24.6))))) + (256 * int(128 * (1. + (cos(k / 24.6))))) + (rand()%128)); //256^1
                }
                int r = rand() % 10;
                unsigned int col = voxlptr[(k * pwidth * pheight * 1) + (j * pwidth * 1) + i * 1 + (0)];

            }


    
    // render loop
    // -----------
    GLuint ssbo = 0;    //declaring my SSBO (has to be done somewhere...)

    

    while (!glfwWindowShouldClose(window))
    {
        iTime = glfwGetTime()*1;
        
        if (Msync.Sync(20))
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
                        voxlptr[(k * pwidth * pheight * 1) + (j * pwidth * 1) + i * 1 + (0)] = unsigned int((256 * 256 * 256 * int(1)) + (256 * 256 * int(1)) + (256 * int(1)) + (0/*opacity is 0 so it's air basically*/));
                        //this is the equation for a funky sphere
                        float condition = sqrt((32 - i) * (32 - i) + (32 - j) * (32 - j) + (32 - k) * (32 - k)) + 1.5 * sin((i + j + k + iTime) / 5.0) + 1.5 * cos((i + k + iTime) / 5.0) + 1.5 * cos((k + j + iTime) / 5.0);
                        if (condition < 28 && condition > 22)
                        {   //by adding the time variable in the equation we get a shape influenced by time
                            voxlptr[(k * pwidth * pheight) + (j * pwidth) + i] = unsigned int((256 * 256 * 256 * int(128 * (1. + (sin((i + (iTime * 1)) / 24.6))))) + (256 * 256 * int(128 * (1. + (sin((j + (iTime * 1)) / 24.6))))) + (256 * int(128)) + (20 * (1 + sin(((iTime * 5) + i) / 25.0)))); //256^1
                        }
                        if (condition < 15)
                        {   //by adding the time variable in the equation we get a shape influenced by time
                            voxlptr[(k * pwidth * pheight) + (j * pwidth) + i] = unsigned int((256 * 256 * 256 * int(228)) + (256 * 256 * int(228)) + (256 * int(228)) + (255)); //256^1
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
            if (Titlesync.Sync(4)) {
                //below you can set the window title
                std::string title = "Voxel Raycaster";
                std::stringstream ss;
                ss << int(1.0 / Vsync.ElapsedTime);
                std::string temp = ss.str();
                std::string temp2 = title + " -FPS:" + temp;
                char* FPS = (char*)temp2.c_str();
                glfwSetWindowTitle(window, FPS);
            }
            
            // input
            // -----
            processInput(window);
            
            /*gets cursor position -->*/
            double x,y;
            glfwGetCursorPos(window, &x, &y);
            ourShader.setV2Float("Mouse", (float)(x - (screenX / 2)) * (1.0f / (screenX / 2)), (float)(y - (screenY / 2)) * (-1.0f / (screenY / 2)));

            ourShader.setV3Float("voxellist", (float)pwidth, (float)pheight, (float)pdepth);
            ourShader.setFloat("iTime", iTime);
            ourShader.setFloat("ElapsedTime", Vsync.ElapsedTime);
            ourShader.setV3Float("CameraPos", camX, camY, camZ);
            ourShader.setV3Float("CameraRot", rotX, rotY,rotZ);
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
        speed += force * Vsync.ElapsedTime;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        float force = 30;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            force *= force/2.0f;
        speed += -force * Vsync.ElapsedTime;
    }
    
    camX += Vsync.ElapsedTime * speed * sin(rotX) * cos(rotY);
    camY -= Vsync.ElapsedTime * speed * sin(rotY);
    camZ += Vsync.ElapsedTime * speed * cos(rotX) * cos(rotY);

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        float force = 30;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            force *= force / 2.0f;
        latspeed -= force * Vsync.ElapsedTime;
        
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        float force = 30;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            force *= force/2.0f;
        latspeed += force * Vsync.ElapsedTime;
    }
    if(abs(latspeed) < 10)rotZ = -latspeed / 70;

    camX += Vsync.ElapsedTime * latspeed * sin(rotX + PI / 2);
    camZ += Vsync.ElapsedTime * latspeed * cos(rotX + PI / 2);

    
    speed -= Vsync.ElapsedTime * 2.0 * speed;
    latspeed -= Vsync.ElapsedTime * 2.0 * latspeed;
    

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