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

// settings
const unsigned int SCR_WIDTH = 700;
const unsigned int SCR_HEIGHT = 700;
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

    //this is a my voxel world specifications
    const int pwidth = 200; const int pheight = 200; const int pdepth = 200;
    unsigned int* voxlptr = new unsigned int[pwidth * pheight * pdepth * 1];      //4 at the end represents the vec4 for colour

    for (int i = 0; i < pwidth; i++)     // a quick way to populate the scene
        for (int j = 0; j < pheight; j++)
            for (int k = 0; k < pdepth; k++)
            {
                unsigned int t = -1;
                float condition = sqrt((100 - i) * (100 - i) + (100 - j) * (100 - j) + (100 - k) * (100 - k)) + 2 * sin(k / 5.0) + 2 * sin(j / 5.0) + 2 * sin(i / 5.0);
                if ( condition < 90 && condition > 60)
                {   
                    t = 25;     //sets the voxel to visible
                }
                int r = rand() % 10;
                voxlptr[(k * pwidth * pheight * 1) + (j * pwidth * 1) + i * 1 + (0)] = unsigned int((256 * 256 * 256 * int(128 * (1. + (sin(i / 24.6))))) + (256 * 256 * int(128 * (1. + (sin(j / 24.6))))) + (256 * int(128 * (1. + (cos(k / 24.6))))) + (t)); //256^1
                unsigned int col = voxlptr[(k * pwidth * pheight * 1) + (j * pwidth * 1) + i * 1 + (0)];

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
            ourShader.setV2Float("CameraRot", rotX, rotY);
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
    camY -= timer * speed * sin(rotY);
    camZ += timer * speed * cos(rotX) * cos(rotY);
    speed *= 0.92 * (timer / timer);
    
    
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        rotX -= timer;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        rotX += timer;

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        rotY -= timer;

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        rotY += timer;

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