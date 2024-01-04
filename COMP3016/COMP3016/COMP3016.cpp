// COMP3016.cpp 70% CMHILL

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

glm::vec3 cameraPosition = glm::vec3(0.0f, 1.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

double lastX = 1280.0 / 2.0;
double lastY = 720.0 / 2.0;

float yaw = -90.0f;
float pitch = 0.0f;

float fov = 45.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

const int numParticles = 100000;

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
};

Particle particles[numParticles];

void initializeParticles();

void updateParticles(float deltaTime);

void processInput(GLFWwindow* window);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

std::string readShaderFile(const std::string& filePath) {
    std::ifstream shaderFile(filePath);
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    return shaderStream.str();
}

int main() {
    if (!glfwInit()) {
        cout << "Failed to initialize GLFW" << endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Rain Simulation", nullptr, nullptr);

    if (window == nullptr) {
        cout << "GLFW Window did not instantiate" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    GLenum glewInitResult = glewInit();
    if (glewInitResult != GLEW_OK) {
        cout << "Failed to initialize GLEW: " << glewGetErrorString(glewInitResult) << endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    int width, height, nrChannels;
    unsigned char* data = stbi_load("Resources/grass.png", &width, &height, &nrChannels, 0);

    if (!data) {
        std::cout << "Failed to load image: " << stbi_failure_reason() << std::endl;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (nrChannels == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else if (nrChannels == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }

    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    const char* vertexShaderPath = "shader.vert";
    const char* fragmentShaderPath = "shader.frag";

    std::string vertexShaderSource = readShaderFile(vertexShaderPath);
    std::string fragmentShaderSource = readShaderFile(fragmentShaderPath);

    const char* vertexShaderSourceCStr = vertexShaderSource.c_str();
    const char* fragmentShaderSourceCStr = fragmentShaderSource.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSourceCStr, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSourceCStr, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLfloat planeVertices[] = {
        -10.0f, 0.0f, -10.0f,    0.0f, 0.0f,
         10.0f, 0.0f, -10.0f,    1.0f, 0.0f,
         10.0f, 0.0f,  10.0f,    1.0f, 1.0f,
        -10.0f, 0.0f,  10.0f,    0.0f, 1.0f,
    };

    GLuint planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);

    glBindVertexArray(planeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    GLfloat quadVertices[] = {
    -0.5f,  0.4f, -0.5f,  0.0f, 0.0f,
     0.5f,  0.4f, -0.5f,  1.0f, 0.0f,
     0.5f,  1.4f, -0.5f,  1.0f, 1.0f,
     0.5f,  1.4f, -0.5f,  1.0f, 1.0f,
    -0.5f,  1.4f, -0.5f,  0.0f, 1.0f,
    -0.5f,  0.4f, -0.5f,  0.0f, 0.0f,

    -0.5f,  0.4f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.4f,  0.5f,  1.0f, 0.0f,
     0.5f,  1.4f,  0.5f,  1.0f, 1.0f,
     0.5f,  1.4f,  0.5f,  1.0f, 1.0f,
    -0.5f,  1.4f,  0.5f,  0.0f, 1.0f,
    -0.5f,  0.4f,  0.5f,  0.0f, 0.0f,

    -0.5f,  1.4f,  0.5f,  1.0f, 0.0f,
    -0.5f,  1.4f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.4f, -0.5f,  0.0f, 1.0f,
    -0.5f,  0.4f, -0.5f,  0.0f, 1.0f,
    -0.5f,  0.4f,  0.5f,  0.0f, 0.0f,
    -0.5f,  1.4f,  0.5f,  1.0f, 0.0f,

     0.5f,  1.4f,  0.5f,  1.0f, 0.0f,
     0.5f,  1.4f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.4f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.4f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.4f,  0.5f,  0.0f, 0.0f,
     0.5f,  1.4f,  0.5f,  1.0f, 0.0f,

    -0.5f,  0.4f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.4f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.4f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.4f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.4f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.4f, -0.5f,  0.0f, 1.0f,

    -0.5f,  1.4f, -0.5f,  0.0f, 1.0f,
     0.5f,  1.4f, -0.5f,  1.0f, 1.0f,
     0.5f,  1.4f,  0.5f,  1.0f, 0.0f,
     0.5f,  1.4f,  0.5f,  1.0f, 0.0f,
    -0.5f,  1.4f,  0.5f,  0.0f, 0.0f,
    -0.5f,  1.4f, -0.5f,  0.0f, 1.0f
    };


    GLuint quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

  

    glm::mat4 projection = glm::perspective(glm::radians(fov), 1280.0f / 720.0f, 0.1f, 100.0f);

    initializeParticles();

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        updateParticles(deltaTime);

        glUseProgram(shaderProgram);
        glUniform1i(glGetUniformLocation(shaderProgram, "objectType"), 0);

        glPointSize(2.0f);
        glBegin(GL_POINTS);
        for (int i = 0; i < numParticles; ++i) {
            glVertex3fv(glm::value_ptr(particles[i].position));
        }
        glEnd();

        glUniform1i(glGetUniformLocation(shaderProgram, "objectType"), 1);

        glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);

        glm::mat4 model = glm::mat4(1.0f);


        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glUniform1i(glGetUniformLocation(shaderProgram, "Texture"), 0);

        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glUniform1i(glGetUniformLocation(shaderProgram, "objectType"), 2);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0,36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void initializeParticles() {
    for (int i = 0; i < numParticles; ++i) {
        particles[i].position = glm::vec3(
            static_cast<float>(rand()) / RAND_MAX * 20.0f - 10.0f,
            static_cast<float>(rand()) / RAND_MAX * 5.0f + 5.0f,
            static_cast<float>(rand()) / RAND_MAX * 20.0f - 10.0f
        );

        particles[i].velocity = glm::vec3(
            0.0f,
            -static_cast<float>(rand()) / RAND_MAX * 5.0f - 2.0f,
            0.0f
        );
    }
}

void updateParticles(float deltaTime) {
    for (int i = 0; i < numParticles; ++i) {
        particles[i].position += particles[i].velocity * deltaTime;

        if (particles[i].position.y < 0.0f) {
            particles[i].position.y = 10.0f;
        }
    }
}

void processInput(GLFWwindow* window) {
    float cameraSpeed = 2.5f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPosition += cameraSpeed * glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPosition -= cameraSpeed * glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    float sensitivity = 0.1f;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);

    lastX = xpos;
    lastY = ypos;
}