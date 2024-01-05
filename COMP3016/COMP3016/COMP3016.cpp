// COMP3016.cpp 70% CMHILL

//Libraries
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace std;

//Camera parameters
glm::vec3 cameraPosition = glm::vec3(0.0f, 1.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

//Mouse Input
double lastX = 1280.0 / 2.0;
double lastY = 720.0 / 2.0;

float yaw = -90.0f;
float pitch = 0.0f;

//FOV
float fov = 45.0f;

//Time-related parameters
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//Number of particles for rain
const int numParticles = 100000;

//Particle structure
struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
};


//Set number of particles
Particle particles[numParticles];

//Functions
void initializeParticles();
void updateParticles(float deltaTime);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);


//Read shader file
std::string readShaderFile(const std::string& filePath) {
    std::ifstream shaderFile(filePath);
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    return shaderStream.str();
}

int main() {
    //Initialize GLFW
    if (!glfwInit()) {
        cout << "Failed to initialize GLFW" << endl;
        return -1;
    }


    //Creates GLFW Window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Rain Simulation", nullptr, nullptr);

    //Error handling
    if (window == nullptr) {
        cout << "GLFW Window did not instantiate" << endl;
        glfwTerminate();
        return -1;
    }


    //Set the window as current context
    glfwMakeContextCurrent(window);


    //Initialize GLEW
    GLenum glewInitResult = glewInit();

    //Error handling
    if (glewInitResult != GLEW_OK) {
        cout << "Failed to initialize GLEW: " << glewGetErrorString(glewInitResult) << endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }


    //Depth test for 3D rendering
    glEnable(GL_DEPTH_TEST);


    //Setup mouse input
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);


    //Load grass texture
    int width, height, nrChannels;
    unsigned char* data = stbi_load("Resources/grass.png", &width, &height, &nrChannels, 0);

    //Error handling
    if (!data) {
        std::cout << "Failed to load image: " << stbi_failure_reason() << std::endl;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    //Texture Parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //Load texture based on colour channels
    if (nrChannels == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else if (nrChannels == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }

    //Generate Mipmaps for texture
    glGenerateMipmap(GL_TEXTURE_2D);

    //Free image data
    stbi_image_free(data);

    //Loads shaders from .vert and .frag files
    const char* vertexShaderPath = "shader.vert";
    const char* fragmentShaderPath = "shader.frag";


    //Reads shader code from files
    std::string vertexShaderSource = readShaderFile(vertexShaderPath);
    std::string fragmentShaderSource = readShaderFile(fragmentShaderPath);

    //Convetrts shader source code to C strings
    const char* vertexShaderSourceCStr = vertexShaderSource.c_str();
    const char* fragmentShaderSourceCStr = fragmentShaderSource.c_str();

    //Compiles vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSourceCStr, NULL);
    glCompileShader(vertexShader);

    //Compiles frag shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSourceCStr, NULL);
    glCompileShader(fragmentShader);

    //Creates shader program and links vertex and frag
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    //Deletes shaders after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    //Ground vertices
    GLfloat planeVertices[] = {
        -10.0f, 0.0f, -10.0f,    0.0f, 0.0f,
         10.0f, 0.0f, -10.0f,    1.0f, 0.0f,
         10.0f, 0.0f,  10.0f,    1.0f, 1.0f,
        -10.0f, 0.0f,  10.0f,    0.0f, 1.0f,
    };

    //Creates vertex array and buffer for ground plane
    GLuint planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);

    glBindVertexArray(planeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    //Sets attribute pointers for ground plane
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //Cube vertices
    GLfloat quadVertices[] = {
        -0.5f,  0.0f, -0.5f,  0.0f, 0.0f,
         0.5f,  0.0f, -0.5f,  1.0f, 0.0f,
         0.5f,  1.0f, -0.5f,  1.0f, 1.0f,
         0.5f,  1.0f, -0.5f,  1.0f, 1.0f,
        -0.5f,  1.0f, -0.5f,  0.0f, 1.0f,
        -0.5f,  0.0f, -0.5f,  0.0f, 0.0f,

        -0.5f,  0.0f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.0f,  0.5f,  1.0f, 0.0f,
         0.5f,  1.0f,  0.5f,  1.0f, 1.0f,
         0.5f,  1.0f,  0.5f,  1.0f, 1.0f,
        -0.5f,  1.0f,  0.5f,  0.0f, 1.0f,
        -0.5f,  0.0f,  0.5f,  0.0f, 0.0f,

        -0.5f,  1.0f,  0.5f,  1.0f, 0.0f,
        -0.5f,  1.0f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.0f, -0.5f,  0.0f, 1.0f,
        -0.5f,  0.0f, -0.5f,  0.0f, 1.0f,
        -0.5f,  0.0f,  0.5f,  0.0f, 0.0f,
        -0.5f,  1.0f,  0.5f,  1.0f, 0.0f,

         0.5f,  1.0f,  0.5f,  1.0f, 0.0f,
         0.5f,  1.0f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.0f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.0f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.0f,  0.5f,  0.0f, 0.0f,
         0.5f,  1.0f,  0.5f,  1.0f, 0.0f,

        -0.5f,  0.0f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.0f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.0f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.0f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.0f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.0f, -0.5f,  0.0f, 1.0f,

        -0.5f,  1.0f, -0.5f,  0.0f, 1.0f,
         0.5f,  1.0f, -0.5f,  1.0f, 1.0f,
         0.5f,  1.0f,  0.5f,  1.0f, 0.0f,
         0.5f,  1.0f,  0.5f,  1.0f, 0.0f,
        -0.5f,  1.0f,  0.5f,  0.0f, 0.0f,
        -0.5f,  1.0f, -0.5f,  0.0f, 1.0f
    };

    //Creates vertex array and buffer for the cube
    GLuint quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    //Sets attribute pointers for the cube
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    //Projection matrix for perspective projection
    glm::mat4 projection = glm::perspective(glm::radians(fov), 1280.0f / 720.0f, 0.1f, 100.0f);

    //Initialize particles for rain
    initializeParticles();

    //Main loop
    while (!glfwWindowShouldClose(window)) {
        //Time tracking
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //Process user input
        processInput(window);

        //Clear colour and depth buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Update particles based on delta time
        updateParticles(deltaTime);

        //Use shader program
        glUseProgram(shaderProgram);
        //Set object type for particles (0)
        glUniform1i(glGetUniformLocation(shaderProgram, "objectType"), 0);

        //Set point size
        glPointSize(2.0f);

        //Render particles as points
        glBegin(GL_POINTS);
        for (int i = 0; i < numParticles; ++i) {
            glVertex3fv(glm::value_ptr(particles[i].position));
        }
        glEnd();

        //Set object type for plane (1)
        glUniform1i(glGetUniformLocation(shaderProgram, "objectType"), 1);

        //View and model matrics for plane
        glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
        glm::mat4 model = glm::mat4(1.0f);

        //Use shader program and update matrics for plane
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        //Attach texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glUniform1i(glGetUniformLocation(shaderProgram, "Texture"), 0);

        //Render the plane
        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        //Set object type for cube (2)
        glUniform1i(glGetUniformLocation(shaderProgram, "objectType"), 2);

        //Render the cube
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0,36);

        //Swap buffers
        glfwSwapBuffers(window);

        //Poll events
        glfwPollEvents();
    }

    //Clean up resources
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);

    //Close window
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

//Function to initalize particle positions and velocity
void initializeParticles() {
    for (int i = 0; i < numParticles; ++i) {
        //Initialize particle positions randomly
        particles[i].position = glm::vec3(
            static_cast<float>(rand()) / RAND_MAX * 20.0f - 10.0f,
            static_cast<float>(rand()) / RAND_MAX * 5.0f + 5.0f,
            static_cast<float>(rand()) / RAND_MAX * 20.0f - 10.0f
        );
        //Initialize particle velocity with random value
        particles[i].velocity = glm::vec3(
            0.0f,
            -static_cast<float>(rand()) / RAND_MAX * 5.0f - 2.0f,
            0.0f
        );
    }
}

//Function to update particle positions
void updateParticles(float deltaTime) {
    for (int i = 0; i < numParticles; ++i) {
        //Update particle positions
        particles[i].position += particles[i].velocity * deltaTime;
        //Reset particles if it hits ground
        if (particles[i].position.y < 0.0f) {
            particles[i].position.y = 10.0f;
        }
    }
}

//Function for user input for camera
void processInput(GLFWwindow* window) {
    //Speed of camera
    float cameraSpeed = 2.5f * deltaTime;

    //Move camera based on user input
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPosition += cameraSpeed * glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPosition -= cameraSpeed * glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

//Function for mouse and camera movement
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    //Offset for mouse movement
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    //Apply sensitivity to mouse movement
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    //Update yaw and pitch based on mouse movement
    yaw += xoffset;
    pitch += yoffset;

    //Avoid gimbal lock
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    //Calculate new camera direction
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);

    //Update last mouse movement
    lastX = xpos;
    lastY = ypos;
}