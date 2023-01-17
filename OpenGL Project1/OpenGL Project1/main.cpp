#define _CRT_SECURE_NO_WARNINGS
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp" //glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp" //glm extension for computing inverse matrices
#include "glm/gtc/type_ptr.hpp" //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"


// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::vec3 lampLightPosition(-197, 9, 24);
glm::vec3 lampLightColor(0.0f, 0.0f, 0.0f);
glm::vec3 purpleLampLightPosition(-258.0f, 9.15, 4.0f);
glm::vec3 purpleLampLightColor(0.0f, 0.0f, 0.0f);
glm::mat4 projection;
glm::mat3 normalMatrix; 
glm::vec3 ghostPosition(-196.0f, 8.0f, 11.0f);
glm::vec3 ghostCenterAnimation(-213.0f, 11.0f, -2.0f);
// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLuint modelLoc;
GLuint viewLoc;
GLuint projectionLoc;
GLuint normalMatrixLoc;
GLuint lightDirLoc;
GLuint lightColorLoc;
GLuint lapmLightPositionLoc;
GLuint lampLightColorLoc;
GLuint purpleLampLightPositionLoc;
GLuint purpleLampLightColorLoc;
GLuint opacityLoc;
GLuint fogDensityLoc;

float fogDensity = 0.0f;
float opacity = 1.0f;

bool moveGhost = false;

// camera
gps::Camera myCamera(
    glm::vec3(-88.0f, 22.0f,-2.50f),
    glm::vec3(-89.0f, 22.0f, -2.29),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.5;
GLfloat cameraRotationSpeed = 15.0f;

GLboolean pressedKeys[1024];

// models
gps::Model3D baseScene;
gps::Model3D ghost;
GLfloat angle;

float ghoastAngle = 0.0f;

// shaders
gps::Shader myBasicShader;
gps::Shader skyboxShader;

// skybox
std::vector<const GLchar*> faces;
gps::SkyBox mySkyBox;

bool mousePause = false;
bool presentationPressed = true;

FILE* file;

GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            error = "STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            error = "STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

/// Window variables and constants
#define PROJECTION_ANGLE 45.0f
#define RENDER_DISTANCE 1000.f
int window_width = 1024, window_height = 648;
void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
    glfwGetFramebufferSize(myWindow.getWindow(), &window_width, &window_height);

    projection = glm::perspective(
        glm::radians(PROJECTION_ANGLE), 
        (float)width / (float)height,
        0.1f, 
        RENDER_DISTANCE);

    myBasicShader.useShaderProgram();
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glViewport(0, 0, window_width, window_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}


// Mouse variables
double mouseSensitivity = 0.3f;
double xd, yd, lx = 400, ly = 300, yaw = 180.0f, pitch = 0;
bool hasCursorChange = true, isPresentationMode = true;
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (mousePause)
        return;

    if (hasCursorChange) {
        hasCursorChange = !hasCursorChange;
        xd = yd = 0;
    }
    else {
        xd = xpos - lx;
        yd = ly - ypos;
    }

    xd *= mouseSensitivity;
    yd *= mouseSensitivity;

    lx = xpos;
    ly = ypos;

    pitch += yd;
    yaw += xd;

    if (pitch > 89.0f)
        pitch = 89.0f;

    if (pitch < -89.0f) 
        pitch = -89.0f;

    myCamera.rotate(pitch, yaw);
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}

void processMovement() {
    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        //fprintf(file, "%d\n", GLFW_KEY_W);
    }
    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        //fprintf(file, "%d\n", GLFW_KEY_S);
    }
    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        //fprintf(file, "%d\n", GLFW_KEY_A);
    }
    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        //fprintf(file, "%d\n", GLFW_KEY_D);
    }
    if (pressedKeys[GLFW_KEY_UP]) {
        myCamera.move(gps::MOVE_UP, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    if (pressedKeys[GLFW_KEY_DOWN]) {
        myCamera.move(gps::MOVE_DOWN, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for baseScene
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    if (pressedKeys[GLFW_KEY_Q]) {
        yaw -= cameraSpeed;
        // update model matrix for teapot
        myCamera.rotate(pitch, yaw);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        //fprintf(file, "%d\n", GLFW_KEY_Q);
    }
    if (pressedKeys[GLFW_KEY_E]) {
        yaw += cameraSpeed;
        // update model matrix for teapot
        myCamera.rotate(pitch, yaw);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        //fprintf(file, "%d\n", GLFW_KEY_E);
    }
    if (pressedKeys[GLFW_KEY_T]) {
        pitch += cameraSpeed;
        // update model matrix for teapot
        myCamera.rotate(pitch, yaw);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        //fprintf(file, "%d\n", GLFW_KEY_T);
    }
    if (pressedKeys[GLFW_KEY_G]) { 
        pitch -= cameraSpeed;
        // update model matrix for teapot
        myCamera.rotate(pitch, yaw);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        //fprintf(file, "%d\n", GLFW_KEY_G);
    }

    if (pressedKeys[GLFW_KEY_P]) {
        mousePause = true;
        myCamera = gps::Camera(
            glm::vec3(-88.0f, 22.0f, -2.50f),
            glm::vec3(-89.0f, 22.0f, -2.29),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        isPresentationMode = false;
        mousePause = false;
        //fprintf(file, "%d\n", GLFW_KEY_P);
    }
    if (pressedKeys[GLFW_KEY_L]) { // turn on the lamp light
        myBasicShader.useShaderProgram();
        lampLightColor = glm::vec3(1, 0, 0); 
        glUniform3fv(lampLightColorLoc, 1, glm::value_ptr(lampLightColor));
        //fprintf(file, "%d\n", GLFW_KEY_L);
    }
    if (pressedKeys[GLFW_KEY_O]) { // turn off the lamp light
        myBasicShader.useShaderProgram();
        lampLightColor = glm::vec3(0, 0, 0); // 0 0 0 for turn off
        glUniform3fv(lampLightColorLoc, 1, glm::value_ptr(lampLightColor));
        //fprintf(file, "%d\n", GLFW_KEY_O);
    }
    if (pressedKeys[GLFW_KEY_1]) { // turn on second lamp 
        myBasicShader.useShaderProgram();
        purpleLampLightColor = glm::vec3(0.4, 0.1, 0.8); 
        glUniform3fv(purpleLampLightColorLoc, 1, glm::value_ptr(purpleLampLightColor));
        //fprintf(file, "%d\n", GLFW_KEY_1);
    }
    if (pressedKeys[GLFW_KEY_2]) { // turn off second lamp 
        myBasicShader.useShaderProgram();
        purpleLampLightColor = glm::vec3(0, 0, 0);
        glUniform3fv(purpleLampLightColorLoc, 1, glm::value_ptr(purpleLampLightColor));
        //fprintf(file, "%d\n", GLFW_KEY_2);
    }
    //polygonal
    if (pressedKeys[GLFW_KEY_Z]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        //fprintf(file, "%d\n", GLFW_KEY_Z);
    }

    if (pressedKeys[GLFW_KEY_X]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        //fprintf(file, "%d\n", GLFW_KEY_X);
    }
    //wireframe
    if (pressedKeys[GLFW_KEY_Y]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //fprintf(file, "%d\n", GLFW_KEY_Y);
    }
    if (pressedKeys[GLFW_KEY_F]) { /// fog ON
        fogDensity = 0.0069;
        myBasicShader.useShaderProgram();
        glUniform1f(fogDensityLoc, fogDensity);
        //fprintf(file, "%d\n", GLFW_KEY_F);
    }
    if (pressedKeys[GLFW_KEY_H]) { // fog OFF
        fogDensity = 0.0;
        myBasicShader.useShaderProgram();
        glUniform1f(fogDensityLoc, fogDensity);
        //fprintf(file, "%d\n", GLFW_KEY_H);
    }
}

void initOpenGLWindow() {
    myWindow.Create(3440, 1337, "OpenGL Project Core");
}

void setWindowCallbacks() {
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    windowResizeCallback(myWindow.getWindow(), 3440, 1337);
}

void initOpenGLState() {
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void initModels() {
    baseScene.LoadModel("models/base-scene/base_scene.obj");
    ghost.LoadModel("models/ghost/ghost.obj");
}

void initShaders() {
    myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
    skyboxShader.loadShader(
        "shaders/skyboxShader.vert",
        "shaders/skyboxShader.frag");
}

void initUniforms() {
    myBasicShader.useShaderProgram();

    // create model matrix for baseScene
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

    // get view matrix for current camera
    myCamera.rotate(pitch, yaw);
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for baseScene
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

    // create projection matrix
    projection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 20.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    // send projection matrix to shader
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //set the light direction (direction towards the light)
    lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    // send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    //set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    // send light color to shader
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    //lamp 
    lampLightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lampLightColor");
    lapmLightPositionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lampLightPosition");

    glUniform3fv(lampLightColorLoc, 1, glm::value_ptr(lampLightColor));
    glUniform3fv(lapmLightPositionLoc, 1, glm::value_ptr(lampLightPosition));

    //purpel light
    purpleLampLightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "purpleLampLightColor");
    purpleLampLightPositionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "purpleLampLightPosition");

    glUniform3fv(purpleLampLightColorLoc, 1, glm::value_ptr(purpleLampLightColor));
    glUniform3fv(purpleLampLightPositionLoc, 1, glm::value_ptr(purpleLampLightPosition));

    // fog location uniform
    fogDensityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");
    glUniform1f(fogDensityLoc, fogDensity);

    opacityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "opacity");
    glUniform1f(opacityLoc, opacity);

    mySkyBox.Load(faces);
    skyboxShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1f(glGetUniformLocation(skyboxShader.shaderProgram, "ambientStrength"), 1.0f);
}

void renderGhost(gps::Shader shader) {
    ghoastAngle += 0.01f;
    glm::mat4 ghostModel(1);
    ghostModel = glm::translate(ghostModel, ghostCenterAnimation);
    ghostModel = glm::rotate(ghostModel, ghoastAngle, glm::vec3(0, 1, 0));
    ghostModel = glm::translate(ghostModel, -ghostCenterAnimation);

    ghostModel = glm::translate(ghostModel, ghostPosition);
    ghostModel = glm::scale(ghostModel, glm::vec3(0.3, 0.3, 0.3));
    ghostModel = glm::rotate(ghostModel, glm::radians(170.0f), glm::vec3(0, 1, 0));
    ghostModel = glm::rotate(ghostModel, glm::radians(20.0f), glm::vec3(1, 0, 0));
    
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)) * ghostModel;
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    opacity = 0.2f;
    shader.useShaderProgram();
    glUniform1f(opacityLoc, opacity);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    ghost.Draw(shader);
    opacity = 1.0;
    shader.useShaderProgram();
    glUniform1f(opacityLoc, opacity);
}

void renderBaseScene(gps::Shader shader) {
    shader.useShaderProgram();
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    baseScene.Draw(shader);
}

void renderScene() {
    myBasicShader.useShaderProgram();
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderBaseScene(myBasicShader);
    renderGhost(myBasicShader);

    skyboxShader.useShaderProgram();
    glUniform1f(glGetUniformLocation(skyboxShader.shaderProgram, "ambientStrength"), 1.0f);
    mySkyBox.Draw(skyboxShader, view, projection);
}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

void initSkyBox() {
    faces.push_back("models/skybox/nightsky_rt.tga");
    faces.push_back("models/skybox/nightsky_lf.tga");
    faces.push_back("models/skybox/nightsky_up.tga");
    faces.push_back("models/skybox/nightsky_dn.tga");
    faces.push_back("models/skybox/nightsky_bk.tga");
    faces.push_back("models/skybox/nightsky_ft.tga");
}

int baseFrameCounter = 0;

void clearButtonsState() {
    pressedKeys[GLFW_KEY_W] = false; 
    pressedKeys[GLFW_KEY_A] = false;
    pressedKeys[GLFW_KEY_S] = false;
    pressedKeys[GLFW_KEY_D] = false;

    pressedKeys[GLFW_KEY_Q] = false;
    pressedKeys[GLFW_KEY_E] = false;
    pressedKeys[GLFW_KEY_G] = false;
    pressedKeys[GLFW_KEY_T] = false;
}

void presentationAnimation() {
    int button;
    if (isPresentationMode) {
        if (fscanf(file, "%d", &button)) {
            clearButtonsState();
            pressedKeys[button] = true;
        }
    }
}

int main(int argc, const char* argv[]) {

    try {
        initOpenGLWindow();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
    initModels();
    initShaders();
    initSkyBox();
    initUniforms();
    setWindowCallbacks();

    glCheckError();
    // application loop
    file = fopen("presentation.in", "r");
    mousePause = true;
    while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
        presentationAnimation();
        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());

        glCheckError();
    }

    fclose(file);
    cleanup();

    return EXIT_SUCCESS;
}
