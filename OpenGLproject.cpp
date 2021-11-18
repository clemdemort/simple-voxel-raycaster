#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include<sstream>
#include "shader.h"
#include <memory>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void getTime();
void setTitle(float Dspeed, GLFWwindow* window);
glm::vec3 cornerRaycalc(float FOV, float cornerX, float cornerY);

// settings
const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;
int screenX = SCR_WIDTH;
int screenY = SCR_HEIGHT;
float PI = 3.142857;
float speed;
float timeNow ;
float timeNow2 = glfwGetTime();
float timerguy;
float timer;
int data[16][16];

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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL shading project", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

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

    //this is what my code is currently doing(not voxels YET) so it's using the array to color pixels on the screen
    const int pwidth = 100; const int pheight = 100; const int pdepth = 100;
    unsigned int* voxlptr = new unsigned int[pwidth * pheight * pdepth * 1];      //4 at the end represents the vec4 for colour

    for (int i = 0; i < pwidth; i++)     // a quick way to populate the scene
        for (int j = 0; j < pheight; j++)
            for (int k = 0; k < pdepth; k++)
            {
                unsigned int t = -1;
                if (sqrt((50 - i) * (50 - i) + (50 - j) * (50 - j) + (50 - k) * (50 - k)) < 50)
                {   
                    t = 25;
                }
                int r = rand() % 10;
                voxlptr[(k * pwidth * pheight * 1) + (j * pwidth * 1) + i * 1 + (0)] = unsigned int((256 * 256 * 256 * int(128 * (1. + (sin(i / 12.8))))) + (256 * 256 * int(128 * (1. + (sin(j / 12.8))))) + (256 * int(128 * (1. + (cos(k / 12.8))))) + (t)); //256^1
                unsigned int col = voxlptr[(k * pwidth * pheight * 1) + (j * pwidth * 1) + i * 1 + (0)];

                /*unsigned int R = (col / unsigned int(256 * 256 * 256));
                unsigned int G = ((col - (R * (256 * 256 * 256))) / (256 * 256));
                unsigned int B = ((col - (R * (256 * 256 * 256) + (G * 256 * 256))) / (256));
                unsigned int A = ((col - (R * (256 * 256 * 256) + (G * 256 * 256) + (B * 256))));
                */
            }

    //this is how i transfer the contents of my array to my shader
    //------------------------------------------------------------
    int arrSize = (4 * pwidth * pheight * pdepth);
    GLuint ssbo = 0;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, arrSize, voxlptr, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    memcpy(p, voxlptr, arrSize);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        getTime();
        if(0.01 < timerguy )
        {
            setTitle(0.25,window);
            
            // input
            // -----
            processInput(window);
            
            /*gets cursor position -->*/
            double x,y;
            glfwGetCursorPos(window, &x, &y);
            ourShader.setV2Float("Mouse", (float)(x - (screenX / 2)) * (1.0f / (screenX / 2)), (float)(y - (screenY / 2)) * (-1.0f / (screenY / 2)));

            ourShader.setV3Float("voxellist", (float)pwidth, (float)pheight, (float)pdepth);
            ourShader.setFloat("iTime", glfwGetTime());
            ourShader.setFloat("ElapsedTime", timer);
            ourShader.setV3Float("CameraPos", camX, camY, camZ);
            ourShader.setV3Float("cameraDir", sin(rotX) * cos(rotY) , sin(rotY), cos(rotX) * cos(rotY));
            ourShader.setV3Float("CameraRot", rotX, rotY, rotZ);
            ourShader.setV2Float("iResolution", (float)screenX, (float)screenY);
            ourShader.setV2Float("Screen", screenX,screenY);
            
            float FOV = 70;
            float stepsize = 1.0f / 10;

       

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

    // optional: de-allocate all resources once they've outlived their purpose:<
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
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        speed += 3;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        speed -= 3;
    }

    camX += timer * speed * sin(rotX) * cos(rotY);
    camY += timer * speed * sin(rotY);
    camZ += timer * speed * cos(rotX) * cos(rotY);
    speed *= 0.92 * (timer / timer);
    
    
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        rotX -= timer;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        rotX += timer;

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        rotY += timer;

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        rotY -= timer;

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

void getTime()
{
    timeNow2 = glfwGetTime();
    float elapsedtime = timeNow2 - timeNow;
    timeNow = timeNow2;
    timer = elapsedtime;
    timerguy += elapsedtime;

}

void setTitle(float Dspeed, GLFWwindow* window)
{
    //will set the fps counter to 1/timerguy
    if (Dspeed < timerguy) {
        int fps = 1 / timer;
        timerguy = 0;
        std::stringstream ss;
        ss << fps;
        std::string temp = ss.str();
        std::string temp2 = "GLSL/C++ (GPU) -FPS:" + temp;
        char* FPS = (char*)temp2.c_str();
        glfwSetWindowTitle(window, FPS);
    }
}

//glm::vec3 cornerRaycalc(float FOV, float cornerX, float cornerY)
//{
//    float ratio = ((float)screenX / (float)screenY);        //gets screen width to height ratio
//    float rfov = (FOV / 180) * PI;                          //converts FOV from degrees to radians
//    float RAx = rotX + ((rfov / 2) * cornerX * ratio);      //rotation Angle for X
//    float RAy = rotY + ((rfov / 2) * cornerY);              //rotation Angle for Y
//
//    float rotmatX[] =                                       //this is my rotation matrix for the X rotaion
//    {
//    1               ,  0                ,    0                      ,
//    0               ,  cos(RAx)         ,    -sin(RAx)              ,
//    0               ,  sin(RAx)         ,    cos(RAx)
//    };
//    glm::mat3 rotationmatrixX = glm::make_mat3(rotmatX);
//
//    float rotmatY[] =                                       //this is my rotation matrix for the Y rotaion
//    {
//    cos(RAy)               ,  0                ,    sin(RAy)      ,
//    0                      ,  1                ,    0             ,
//    -sin(RAy)              ,  0                ,    cos(RAy)
//    };
//    glm::mat3 rotationmatrixY = glm::make_mat3(rotmatY);
//
//    //sends whatever abomination this function created to the main loop which then passes it as a uniform
//    //only done four times then these are linearly interpolated in the fragment shader to find the direction of each ray
//    glm::vec3 rotation = glm::vec3(1.0f, 1.0f, 1.0f);
//    return (rotation * rotationmatrixX) * rotationmatrixY;
//
//}