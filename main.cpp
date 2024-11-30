#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <random>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define CRES 100

struct Object {
    float x, y, lifetime;
};

void processInput(GLFWwindow* window, int* isSonarTurnedOn);
void convertNDCtoWindowCoords(float ndcX, float ndcY, int windowWidth, int windowHeight, float* windowX, float* windowY);
unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);
unsigned int createTexture(const char* tSource);
unsigned int createSonarTexture();
unsigned int createOffButtonTexture();
unsigned int createOnButtonTexture();
void generateObject(std::vector<Object>* objects);

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
    unsigned int buttonShader = createShader("button.vert", "button.frag");

    unsigned int VBO[3], VAO[3], EBO[2];

    glGenVertexArrays(3, VAO);
    glGenBuffers(3, VBO);
    glGenBuffers(2, EBO);

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

    unsigned int metalPanelTexture = createTexture("metal-panel.jpg");
    unsigned int sonarTexture = createSonarTexture();
    unsigned int buttonOffTexture = createOffButtonTexture();
    unsigned int buttonOnTexture = createOnButtonTexture();

    int isSonarTurnedOn = 0;
    std::srand(static_cast<unsigned>(std::time(0)));
    float currentTime;
    float lastObjectGeneratedTime = 0;
    std::vector<Object> objects;

    while (!glfwWindowShouldClose(window))
    {
        processInput(window, &isSonarTurnedOn);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, metalPanelTexture);

        glViewport(0, 0, mode->width, mode->height);
        glUseProgram(program);
        glBindVertexArray(VAO[0]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

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
            it->lifetime -= 0.001;
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

void processInput(GLFWwindow* window, int* isSonarTurnedOn) {
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

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
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
