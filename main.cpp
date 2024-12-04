#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <random>
#include <vector>
#include <chrono>
#include <thread>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <unordered_map>

#define CRES 100

struct Object {
    float x, y, lifetime;
};

void processInput(GLFWwindow* window, int* isSonarTurnedOn, float* depth);
void convertNDCtoWindowCoords(float ndcX, float ndcY, int windowWidth, int windowHeight, float* windowX, float* windowY);
unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);
unsigned int createTexture(const char* tSource);
unsigned int createSonarTexture();
unsigned int createOffButtonTexture();
unsigned int createOnButtonTexture();
unsigned int createAchtungTexture();
unsigned int createStableTexture();
unsigned int createLogoTexture();
void limitFPS();
void generateObject(std::vector<Object>* objects);
std::unordered_map<int, GLuint> loadNumberTextures();

int main(void)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Submarine2D", primaryMonitor, NULL);
    if (window == NULL)
    {
        std::cout << "Window could not be created! :(\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW could not be loaded! :'(\n";
        return 3;
    }

    

    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

    unsigned int program = createShader("panel.vert", "panel.frag");
    unsigned int sonarShader = createShader("sonar.vert", "sonar.frag");
    unsigned int buttonShader = createShader("texturedRectangle.vert", "button.frag");
    unsigned int depthBarShader = createShader("progressBar.vert", "depthBar.frag");
    unsigned int oBarShader = createShader("progressBar.vert", "oBar.frag");
    unsigned int numberShader = createShader("texturedRectangle.vert", "number.frag");
    unsigned int achtungShader = createShader("texturedRectangle.vert", "achtung.frag");
    unsigned int texturedRectangleShader = createShader("texturedRectangle.vert", "texturedRectangle.frag");
    unsigned int warningLightShader = createShader("warningLight.vert", "warningLight.frag");

    unsigned int VBO[14], VAO[14], EBO[13];

    glGenVertexArrays(14, VAO);
    glGenBuffers(14, VBO);
    glGenBuffers(13, EBO);

    float vertices[] = {
         1.0f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 0.5f,    1.0,  1.0,    // top right
         1.0f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f, 1.0f,    1.0,  0.0,    // bottom right
        -1.0f, -1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f,    0.0,  0.0,    // bottom left
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.5f,    0.0,  1.0     // top left 
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); 

    float circle[(CRES + 2) * 2];
    float r = 0.6; //poluprecnik

    circle[0] = 0; //Centar X0
    circle[1] = 0; //Centar Y0
    int i;
    for (i = 0; i <= CRES; i++)
    {

        circle[2 + 2 * i] = r * cos((3.141592 / 180) * (i * 360 / CRES)); //Xi (Matematicke funkcije rade sa radijanima)
        circle[2 + 2 * i + 1] = r * sin((3.141592 / 180) * (i * 360 / CRES)); //Yi
    }


    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circle), circle, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float buttonVertices[] = {
        -0.430f, -0.635f, 0.0f,  1.0f,  1.0f,    // top right
        -0.430f, -0.885f, 0.0f,  1.0f,  0.0f,    // bottom right
        -0.570f, -0.885f, 0.0f,  0.0f,  0.0f,    // bottom left
        -0.570f, -0.635f, 0.0f,  0.0f,  1.0f     // top left 
    };
    unsigned int buttonIndices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    glBindVertexArray(VAO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(buttonVertices), buttonVertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(buttonIndices), buttonIndices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float depthBarVertices[] = {
        -0.2f,  0.8f, 0.0f,    // top right
        -0.2f, -0.2f, 0.0f,    // bottom right
        -0.8f, -0.2f, 0.0f,    // bottom left
        -0.8f,  0.8f, 0.0f     // top left 
    };
    unsigned int depthBarIndices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    glBindVertexArray(VAO[3]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(depthBarVertices), depthBarVertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(depthBarIndices), depthBarIndices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float oBarVertices[] = {
        0.55f,  0.8f, 0.0f,    // top right
        0.55f, -0.2f, 0.0f,    // bottom right
        -0.05f, -0.2f, 0.0f,    // bottom left
        -0.05f,  0.8f, 0.0f     // top left 
    };
    unsigned int oBarIndices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    glBindVertexArray(VAO[4]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(oBarVertices), oBarVertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(oBarIndices), oBarIndices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float depthHVertices[] = {
        -0.6f, -0.2f, 0.0f,  1.0f, 1.0f,  // top right
        -0.6f, -0.4f, 0.0f,  1.0f, 0.0f,  // bottom right
        -0.8f, -0.4f, 0.0f,  0.0f, 0.0f,  // bottom left
        -0.8f,  -0.2f, 0.0f,   0.0f, 1.0f  // top left 
    };
    unsigned int depthHIndices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    float depthTVertices[] = {
        -0.4f,  -0.2f, 0.0f,  1.0f, 1.0f,  // top right
        -0.4f, -0.4f, 0.0f,  1.0f, 0.0f,  // bottom right
        -0.6f, -0.4f, 0.0f,  0.0f, 0.0f,  // bottom left
        -0.6f,  -0.2f, 0.0f,  0.0f, 1.0f   // top left 
    };
    unsigned int depthTIndices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    float depthOVertices[] = {
        -0.2f,  -0.2f, 0.0f,  1.0f, 1.0f,  // top right
        -0.2f, -0.4f, 0.0f,  1.0f, 0.0f,  // bottom right
        -0.4f, -0.4f, 0.0f,  0.0f, 0.0f,  // bottom left
        -0.4f,  -0.2f, 0.0f,  0.0f, 1.0f     // top left 
    };
    unsigned int depthOIndices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    glBindVertexArray(VAO[5]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[5]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(depthHVertices), depthHVertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[4]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(depthBarIndices), depthBarIndices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glBindVertexArray(VAO[6]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[6]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(depthTVertices), depthTVertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[5]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(depthTIndices), depthTIndices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glBindVertexArray(VAO[7]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[7]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(depthOVertices), depthOVertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[6]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(depthOIndices), depthOIndices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float oHVertices[] = {
        0.15f, -0.2f, 0.0f,  1.0f, 1.0f,  // top right
        0.15f, -0.4f, 0.0f,  1.0f, 0.0f,  // bottom right
        -0.05f, -0.4f, 0.0f,  0.0f, 0.0f,  // bottom left
        -0.05f,  -0.2f, 0.0f,   0.0f, 1.0f  // top left 
    };
    unsigned int oHIndices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    float oTVertices[] = {
        0.35f,  -0.2f, 0.0f,  1.0f, 1.0f,  // top right
        0.35f, -0.4f, 0.0f,  1.0f, 0.0f,  // bottom right
        0.15f, -0.4f, 0.0f,  0.0f, 0.0f,  // bottom left
        0.15f,  -0.2f, 0.0f,  0.0f, 1.0f   // top left 
    };
    unsigned int oTIndices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    float oOVertices[] = {
        0.55f,  -0.2f, 0.0f,  1.0f, 1.0f,  // top right
        0.55f, -0.4f, 0.0f,  1.0f, 0.0f,  // bottom right
        0.35f, -0.4f, 0.0f,  0.0f, 0.0f,  // bottom left
        0.35f,  -0.2f, 0.0f,  0.0f, 1.0f     // top left 
    };
    unsigned int oOIndices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    }; 

    glBindVertexArray(VAO[8]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[8]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(oHVertices), oHVertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[7]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(oHIndices), oHIndices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glBindVertexArray(VAO[9]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[9]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(oTVertices), oTVertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[8]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(oTIndices), oTIndices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glBindVertexArray(VAO[10]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[10]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(oOVertices), oOVertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[9]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(oOIndices), oOIndices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float achtungVertices[] = {
    0.55f, -0.5f, 0.0f,  1.0f, 1.0f,  // top right
    0.55f, -0.8f, 0.0f,  1.0f, 0.0f,  // bottom right
    -0.5f, -0.8f, 0.0f,  0.0f, 0.0f,  // bottom left
    -0.5f,  -0.5f, 0.0f,   0.0f, 1.0f  // top left 
    };
    unsigned int achtungIndices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    glBindVertexArray(VAO[11]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[11]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(achtungVertices), achtungVertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[10]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(achtungIndices), achtungIndices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float logoVertices[] = {
        -0.6f, -0.6f, 0.0f,  1.0f, 1.0f,  // top right
        -0.6f, -1.0f, 0.0f,  1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,  // bottom left
        -1.0f,  -0.6f, 0.0f,   0.0f, 1.0f  // top left 
    };
    unsigned int logoIndices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    glBindVertexArray(VAO[12]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[12]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(logoVertices), logoVertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[11]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(logoIndices), logoIndices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float warningLightVertices[] = {
        0.5f,  0.0f, 0.0f,    // top right
        0.5f, -1.0f, 0.0f,    // bottom right
        -0.5f, -1.0f, 0.0f,    // bottom left
        -0.5f,  -0.0f, 0.0f     // top left 
    };
    unsigned int warningLightIndices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    glBindVertexArray(VAO[13]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[13]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(warningLightVertices), warningLightVertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[12]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(warningLightIndices), warningLightIndices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    unsigned int metalPanelTexture = createTexture("metal-panel.jpg");
    unsigned int sonarTexture = createSonarTexture();
    unsigned int buttonOffTexture = createOffButtonTexture();
    unsigned int buttonOnTexture = createOnButtonTexture();
    unsigned int achtungTexture = createAchtungTexture();
    unsigned int stableTexture = createStableTexture();
    unsigned int logoTexture = createLogoTexture();
    std::unordered_map<int, GLuint> numberTextures = loadNumberTextures();

    int isSonarTurnedOn = 0;
    std::srand(static_cast<unsigned>(std::time(0)));
    float currentTime;
    float lastObjectGeneratedTime = 0;
    float lastBreath = 0;
    std::vector<Object> objects;
    float depth = 0;
    float oxygen = 100;
    int showText = 0;
    int oxygenTimer = 0;
    int lastOState = 0;
    int depthHundreds, depthTens, depthOnes, oxygenHundreds, oxygenTens, oxygenOnes;

    while (!glfwWindowShouldClose(window))
    {
        processInput(window, &isSonarTurnedOn, &depth);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, metalPanelTexture);

        glViewport(0, 0, mode->width, mode->height);
        glUseProgram(program);
        glBindVertexArray(VAO[0]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindTexture(GL_TEXTURE_2D, logoTexture);
        glUseProgram(texturedRectangleShader);
        glBindVertexArray(VAO[12]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glDisable(GL_BLEND);

        glBindTexture(GL_TEXTURE_2D, sonarTexture);

        glViewport(0, 0, mode->width / 2, mode->height);  // Da crtamo na desnoj polovini ekrana
        int vertexIsSonarTurnedOnLocation = glGetUniformLocation(sonarShader, "isSonarTurnedOn");
        glUseProgram(sonarShader);
        glUniform1i(vertexIsSonarTurnedOnLocation, isSonarTurnedOn);
        float timeValue = glfwGetTime();
        float greenValue = 0.25f * sin(2.5f * timeValue) + 0.5f;
        int vertexGreenColorLocation = glGetUniformLocation(sonarShader, "greenColor");
        glUniform3f(vertexGreenColorLocation, 0.0f, greenValue, 0.0f);
        float sweepAngle = fmod(timeValue * 2.0f, 6.28318530718f);

        // Set the uniform for the sweep angle
        int sweepAngleLocation = glGetUniformLocation(sonarShader, "sweepAngle");

        auto it = objects.begin();
        while (it != objects.end()) {
            it->lifetime -= 0.01;
            if (it->lifetime <= 0.0f) {
                it = objects.erase(it);
            }
            else {
                ++it;
            }
        } 

        currentTime = glfwGetTime();
        if (currentTime - 3.0f > lastObjectGeneratedTime) {
            generateObject(&objects);
            lastObjectGeneratedTime = currentTime;
        }

        std::vector<GLfloat> objectPositions;
        for (const auto& obj : objects) {
            objectPositions.push_back(obj.x);
            objectPositions.push_back(obj.y);
        }

        glUniform1i(glGetUniformLocation(sonarShader, "objectCount"), objects.size());
        glUniform2fv(glGetUniformLocation(sonarShader, "objects"), objects.size(), objectPositions.data());

        glUniform1f(sweepAngleLocation, sweepAngle);
        glBindVertexArray(VAO[1]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(circle) / (2 * sizeof(float)));
        glUseProgram(0);
        glBindVertexArray(0);

        glViewport(0, 0, mode->width, mode->height);

        if (isSonarTurnedOn)
            glBindTexture(GL_TEXTURE_2D, buttonOffTexture);
        else
            glBindTexture(GL_TEXTURE_2D, buttonOnTexture);
        
        glUseProgram(buttonShader);
        glBindVertexArray(VAO[2]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glViewport(mode->width / 2, 0, mode->width / 2, mode->height);

        glUseProgram(depthBarShader);
        glUniform1f(glGetUniformLocation(depthBarShader, "depth"), depth / 250);
        glBindVertexArray(VAO[3]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        if (currentTime - 0.25f > lastBreath) {        
            if (depth != 0) {
                if (oxygen > 0)
                    oxygen --;
            }

            lastBreath = currentTime;
        }

        if (depth == 0) {
            if (oxygen < 100)
                oxygen += 0.5;
        }

        glUseProgram(oBarShader);
        glUniform1f(glGetUniformLocation(oBarShader, "oLevel"), oxygen / 100);
        glBindVertexArray(VAO[4]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        depthHundreds = (int)depth / 100;
        depthTens = ((int)depth / 10) % 10;
        depthOnes = (int)depth % 10;

        glUseProgram(numberShader);

        glBindTexture(GL_TEXTURE_2D, numberTextures[depthHundreds]);
        glUniform1i(glGetUniformLocation(numberShader, "h"), depthHundreds);
        glUniform1i(glGetUniformLocation(numberShader, "t"), depthTens);
        glUniform1i(glGetUniformLocation(numberShader, "o"), depthOnes);

        glUniform1i(glGetUniformLocation(numberShader, "positionIndicator"), 0);
        glBindVertexArray(VAO[5]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, numberTextures[depthTens]);
        glUniform1i(glGetUniformLocation(numberShader, "positionIndicator"), 1); 
        glBindVertexArray(VAO[6]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, numberTextures[depthOnes]);
        glUniform1i(glGetUniformLocation(numberShader, "positionIndicator"), 2); 
        glBindVertexArray(VAO[7]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        oxygenHundreds = (int)oxygen / 100;
        oxygenTens = ((int)oxygen / 10) % 10;
        oxygenOnes = (int)oxygen % 10;

        glUniform1i(glGetUniformLocation(numberShader, "h"), oxygenHundreds);
        glUniform1i(glGetUniformLocation(numberShader, "t"), oxygenTens);
        glUniform1i(glGetUniformLocation(numberShader, "o"), oxygenOnes);

        glBindTexture(GL_TEXTURE_2D, numberTextures[oxygenHundreds]);
        glUniform1i(glGetUniformLocation(numberShader, "positionIndicator"), 0);
        glBindVertexArray(VAO[8]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, numberTextures[oxygenTens]);
        glUniform1i(glGetUniformLocation(numberShader, "positionIndicator"), 1);
        glBindVertexArray(VAO[9]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, numberTextures[oxygenOnes]);
        glUniform1i(glGetUniformLocation(numberShader, "positionIndicator"), 2);
        glBindVertexArray(VAO[10]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glUseProgram(achtungShader);

        if (oxygen <= 25) {
            glBindTexture(GL_TEXTURE_2D, achtungTexture);
            if (currentTime - 0.5 > oxygenTimer)
                showText = 1;
            else
                showText = 0;
            oxygenTimer = currentTime;
            lastOState = 1;
        }
        else if (oxygen > 75) {
            glBindTexture(GL_TEXTURE_2D, stableTexture);
            showText = 1;
            lastOState = 0;
        }
        else
            showText = 0;
        
        glUniform1i(glGetUniformLocation(achtungShader, "showText"), showText);
        glBindVertexArray(VAO[11]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        
        glViewport(0, 0, mode->width, mode->height);
        glUseProgram(warningLightShader);
        glUniform1i(glGetUniformLocation(warningLightShader, "isLightOn"), lastOState);
        glBindVertexArray(VAO[13]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glDisable(GL_BLEND);
        
        limitFPS();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void generateObject(std::vector<Object>* objects) {
    Object newObject;
    newObject.x = static_cast<float>(std::rand()) / RAND_MAX * 0.5f - 0.25f;
    newObject.y = static_cast<float>(std::rand()) / RAND_MAX * 0.5f - 0.25f;
    newObject.lifetime = 1.0f + static_cast<float>(std::rand()) / RAND_MAX * 4.0f;

    objects->push_back(newObject);
}

void processInput(GLFWwindow* window, int* isSonarTurnedOn, float* depth) {
    static int lastMouseState = GLFW_RELEASE;
    int currentMouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

    int wWidth, wHeight;
    glfwGetWindowSize(window, &wWidth, &wHeight);

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    

    // Invert mouse Y for comparison with NDC
    mouseY = wHeight - mouseY;

    float buttonXMin, buttonXMax, buttonYMin, buttonYMax;
    // Convert button's NDC coordinates to window coordinates
    convertNDCtoWindowCoords(-0.57, -0.885, wWidth, wHeight, &buttonXMin, &buttonYMin);
    convertNDCtoWindowCoords(-0.43, -0.635, wWidth, wHeight, &buttonXMax, &buttonYMax);

    // Check if mouse click is inside the button's region
    if (currentMouseState == GLFW_PRESS && lastMouseState == GLFW_RELEASE && mouseX > buttonXMin && mouseX < buttonXMax && mouseY > buttonYMin && mouseY < buttonYMax) 
        *isSonarTurnedOn = (*isSonarTurnedOn) ? 0 : 1;

    lastMouseState = currentMouseState; // Update last mouse state

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        if (*depth < 250)
            (*depth)++;
    }
        

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        if (*depth > 0)
            (*depth)--;
        else
            (*depth) = 0;
    }
        

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void limitFPS() {
    int frameDelay = 1000 / 60;
    static auto lastFrameTime = std::chrono::high_resolution_clock::now();
    auto currentFrameTime = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentFrameTime - lastFrameTime).count();

    if (elapsedTime < (1000 / 60)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(frameDelay - elapsedTime));
    }
    lastFrameTime = std::chrono::high_resolution_clock::now();
}

void convertNDCtoWindowCoords(float ndcX, float ndcY, int windowWidth, int windowHeight, float* windowX, float* windowY) {
    *windowX = ((ndcX + 1.0f) / 2.0f) * windowWidth;
    *windowY = ((ndcY + 1.0f) / 2.0f) * windowHeight;
}


unsigned int compileShader(GLenum type, const char* source)
{
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;
    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        std::cout << "Successfully read file from \"" << source << "\"!" << std::endl;
    }
    else {
        ss << "";
        std::cout << "Error reading file from \"" << source << "\"!" << std::endl;
    }
    std::string temp = ss.str();
    const char* sourceCode = temp.c_str();

    int shader = glCreateShader(type);

    int success;
    char infoLog[512];
    glShaderSource(shader, 1, &sourceCode, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf(" shader compilation failed! Error: \n");
        printf(infoLog);
    }
    return shader;
}

unsigned int createShader(const char* vsSource, const char* fsSource)
{
    unsigned int program;
    unsigned int vertexShader;
    unsigned int fragmentShader;

    program = glCreateProgram();

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "Shader program linking failed! Error: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

std::unordered_map<int, GLuint> loadNumberTextures() {
    std::unordered_map<int, GLuint> textures;
    for (int i = 0; i <= 9; ++i) {
        std::string filename = std::to_string(i) + ".png";

        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Load the image using STB Image
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true); // Flip the image vertically
        unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);

        if (data) {
            GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA; // Determine format
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            // Texture parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            textures[i] = textureID; // Store the texture ID in the map
        }
        else {
            std::cerr << "Failed to load texture: " << filename << std::endl;
            glDeleteTextures(1, &textureID);
        }

        stbi_image_free(data);
    }

    // Unbind the texture
    glBindTexture(GL_TEXTURE_2D, 0);
    return textures;
}

unsigned int createTexture(const char* tSource) {
    unsigned int texture;

    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// set texture wrapping to GL_REPEAT (default wrapping method)

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set texture filtering parameters

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // load image, create texture and generate mipmaps
    int width, height, nrChannels;

    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    unsigned char* data = stbi_load(tSource, &width, &height, &nrChannels, 0);

    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << "Texture loaded successfully: " << width << "x" << height << " with " << nrChannels << " channels" << std::endl;
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    return texture;
}

unsigned int createOffButtonTexture() {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load image
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // Flip the texture vertically if needed
    unsigned char* data = stbi_load("off-button.png", &width, &height, &nrChannels, 0);
    if (data) {
        // Determine the format
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

        // Upload texture data
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind only after the texture is fully loaded

    return texture;
}

unsigned int createOnButtonTexture() {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load image
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // Flip the texture vertically if needed
    unsigned char* data = stbi_load("on-button.png", &width, &height, &nrChannels, 0);
    if (data) {
        // Determine the format
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

        // Upload texture data
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind only after the texture is fully loaded

    return texture;
}

unsigned int createSonarTexture() {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load image
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // Flip the texture vertically if needed
    unsigned char* data = stbi_load("sonar.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        // Determine the format
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind only after the texture is fully loaded

    return texture;
}

unsigned int createAchtungTexture() {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    // Texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load image
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // Flip the texture vertically if needed
    unsigned char* data = stbi_load("achtung.png", &width, &height, &nrChannels, 0);
    if (data) {
        // Determine the format
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

        // Upload texture data
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture achtung" << std::endl;
    }

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind only after the texture is fully loaded

    return texture;
}

unsigned int createStableTexture() {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    // Texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load image
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // Flip the texture vertically if needed
    unsigned char* data = stbi_load("stable.png", &width, &height, &nrChannels, 0);
    if (data) {
        // Determine the format
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

        // Upload texture data
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind only after the texture is fully loaded

    return texture;
}

unsigned int createLogoTexture() {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load image
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // Flip the texture vertically if needed
    unsigned char* data = stbi_load("logo.png", &width, &height, &nrChannels, 0);
    if (data) {
        // Determine the format
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

        // Upload texture data
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind only after the texture is fully loaded

    return texture;
}

