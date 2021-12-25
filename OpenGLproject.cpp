#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <glm/glm/gtc/noise.hpp>
#include <sstream>
#include "shader.h"
#include "TimeSync.h"
#include "CustomNoise.h"
#include <memory>
#include <iostream>
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
unsigned int compress(uint8_t R, uint8_t G, uint8_t B, uint8_t A);
void APIENTRY glDebugOutput(GLenum source,
    GLenum type,
    unsigned int id,
    GLenum severity,
    GLsizei length,
    const char* message,
    const void* userParam);

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
//noise class
Noise random;
Noise smooth;

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
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,true);

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

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glDebugOutput, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    int max;
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &max);
    std::cout << max << "\n";

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

    //generating noise
    random.generate2d(60,60,0, 50);
    smooth.generate2d(100,100,0, 7);


    //this is a my voxel world specifications
    const int pwidth = 1000; const int pheight = 115; const int pdepth = 1000;
    unsigned int *voxlptr = new unsigned int[pwidth * pheight * pdepth];

    GLuint ssbo = 0;    //declaring my texture3D

            //collecting el garbage
            //---------------------
    glDeleteBuffers(1, &ssbo);
    ssbo = 0;
    int generated = 0;
    std::cout << " generating the world... \n this may take a while... \n";
    for (int i = 0; i < pwidth; i++)
        for (int j = 0; j < pheight; j++)
            for (int k = 0; k < pdepth; k++)
            {
                voxlptr[(k * pwidth * pheight * 1) + (j * pwidth * 1) + i] = compress(0, 0, 0, 0);
                ////this is the equation for a sphere
                //float condition = sqrtf(powf(pwidth / 2.0f - i, 2) + powf(pheight / 2.0f - j, 2) + powf(pdepth / 2.0f - k, 2)) + random.Get2D(i/20.0,j/20.0);
                float condition = j;
                if (condition < 55)
                {
                    voxlptr[(k * pwidth * pheight) + (j * pwidth) + i] = compress(10, 40, 120, 20);
                }
                if (condition < 2)
                {
                    voxlptr[(k * pwidth * pheight) + (j * pwidth) + i] = compress(40, 40, 40, 255);
                }
                condition = j + 0.3 * random.Get2D(i / 10.0, k / 10.0) - 2 * random.Get2D(i / 50.0, k / 50.0) + 3 * smooth.Get2D(i / 25.0, k / 25.0) + 3 * smooth.Get2D(i / 7.5, k / 7.5);
                if(condition < 25)
                {   //by adding the time variable in the equation we get a shape influenced by time
                    voxlptr[(k * pwidth * pheight) + (j * pwidth) + i] = compress(120,175,50, 255);
                }
            }
    std::cout << "world generating done! sending data to the GPU!\n";
    //this is how i transfer the contents of my array to my shader using a 3D texture
    //------------------------------------------------------------
    int arrSize = (4 * pwidth * pheight * pdepth);
    glGenTextures(1, &ssbo);
    glBindTexture(GL_TEXTURE_3D, ssbo);
    glTexStorage3D(GL_TEXTURE_3D,
        1,             // No mipmaps
        GL_R32UI,      // Internal format
        pwidth, pheight, pdepth);
    glTexSubImage3D(GL_TEXTURE_3D,
        0,                // Mipmap number
        0, 0, 0,          // xoffset, yoffset, zoffset
        pwidth, pheight, pdepth, // width, height, depth
        GL_RED_INTEGER,         // format
        GL_UNSIGNED_INT, // type
        voxlptr);           // pointer to data
    glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
    //------------------------------------------------------------
    std::cout << "done! \n";
    while (!glfwWindowShouldClose(window))
    {
        iTime = glfwGetTime()*1;
        
        if (Msync.Sync(0)) //this is not working
        {
            ssbo = 0;
            //int generated = 0;
            //for testing purposes this is updating the map dynamically every 0th of a second for now this works more than fine but soon i'll have to implement chunking to go alongside it.
            //-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
            for(int i = 0; i < 100; i++)
            {
                unsigned int r = ((rand() % pdepth) * pwidth * pheight) + ((rand() % pheight) * pwidth) + (rand() % pwidth);
                voxlptr[r] = compress(rand(), rand(), rand(), rand());

            }

            //this is how i transfer the contents of my array to my shader
            //------------------------------------------------------------
            glTexSubImage3D(GL_TEXTURE_3D,
                0,                // Mipmap number
                0, 0, 0,          // xoffset, yoffset, zoffset
                pwidth, pheight, pdepth, // width, height, depth
                GL_RED_INTEGER,         // format
                GL_UNSIGNED_INT, // type
                voxlptr);           // pointer to data
            glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
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
            float FOV = 90; //pretty obscure fonctionallity ;P
            //use shader
            ourShader.use();

            /*gets cursor position -->*/
            double x,y;
            glfwGetCursorPos(window, &x, &y);
            ourShader.setV2Float("Mouse", (float)(x - (screenX / 2)) * (1.0f / (screenX / 2)), (float)(y - (screenY / 2)) * (-1.0f / (screenY / 2)));

            ourShader.setV3Float("voxellist", (float)pwidth, (float)pheight, (float)pdepth);
            ourShader.setFloat("iTime", iTime);
            ourShader.setFloat("FOV", FOV);
            ourShader.setFloat("ElapsedTime", Vsync.ElapsedTime);
            ourShader.setV3Float("CameraPos", camX, camY, camZ);
            ourShader.setV3Float("CameraRot", rotX, rotY,rotZ);
            ourShader.setV2Float("iResolution", (float)screenX, (float)screenY);
            ourShader.setV2Float("Screen", screenX,screenY);
            

       

            // render the shader
            glBindImageTexture(0,
                ssbo,
                0,
                true,
                0,
                GL_READ_ONLY,
                GL_R32UI);
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
    random.destroy();
    smooth.destroy();
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

unsigned int compress(uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
    return unsigned int((256 * 256 * 256 * R) + (256 * 256 * G) + (256 * B) + (A));
}


void APIENTRY glDebugOutput(GLenum source,
    GLenum type,
    unsigned int id,
    GLenum severity,
    GLsizei length,
    const char* message,
    const void* userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}