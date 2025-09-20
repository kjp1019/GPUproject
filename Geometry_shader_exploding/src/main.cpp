#include <learnopengl/shader.h>     
#include <learnopengl/camera.h>     
#include <learnopengl/model.h>      
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <array>
#include <random>
#include <vector>

// settings
const unsigned int SCR_WIDTH = 1800;
const unsigned int SCR_HEIGHT = 1200;

// Initial positions
const glm::vec3 INITIAL_CUBE_POS(-8.0f, 0.25f, -8.0f);
const std::array<glm::vec3, 4> INITIAL_CAR_A_POS = {
    glm::vec3(4.0f, 0.0f,  4.0f),
    glm::vec3(4.0f, 0.0f, -4.0f),
    glm::vec3(-4.0f, 0.0f,  4.0f),
    glm::vec3(-4.0f, 0.0f, -4.0f)
};
const glm::vec3 INITIAL_CAR_B_POS(0.0f, 0.0f, -5.0f);

// cube (player) state
glm::vec3 cubePos = INITIAL_CUBE_POS;
bool       isJumping = false;
float      jumpVelocity = 0.0f;
const float gravity = -9.8f;
const float jumpPower = 5.0f;
const float groundY = 0.25f;
float      moveSpeed = 2.5f;

// dash parameters
const float dashDistance = 5.0f;
const float dashCooldown = 2.0f;
float      lastDashTime = -dashCooldown;

// explosion state for Car A's 4 cars
std::array<bool, 4>  exploded = { false,false,false,false };
std::array<float, 4> explosionStart = { -1,-1,-1,-1 };
const float explosionDuration = 2.0f;

// Car B (rideable) state
glm::vec3 carBPos = INITIAL_CAR_B_POS;
bool       inCar = false;
float      carBYaw = -90.0f;
float      carBSpeed = 5.0f;
float      lastExitTime = -1.0f;

// timing & camera
float deltaTime = 0.0f, lastFrame = 0.0f;
float yaw = -90.0f, pitch = 20.0f;
float lastX = SCR_WIDTH / 2.0f, lastY = SCR_HEIGHT / 2.0f;
bool  firstMouse = true;
float distanceToCube = 5.0f;

// particle system
struct Particle {
    glm::vec3 pos;
    glm::vec3 velocity;
    float     life;
    bool      active;
};
const int MAX_PARTICLES = 5000;
std::vector<Particle> particles;
GLuint particleVAO, particleVBO;
std::default_random_engine generator;
std::uniform_real_distribution<float> distrib(-1.0f, 1.0f);

// function prototypes
void framebuffer_size_callback(GLFWwindow* w, int width, int height);
void mouse_callback(GLFWwindow* w, double xpos, double ypos);
void processInput(GLFWwindow* w);
unsigned int loadTexture(const char* path);
void initParticles();
void emitParticles(glm::vec3 center);
void updateParticles(float dt);

int main()
{
    // GLFW init
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Project", NULL, NULL);
    if (!window) { std::cerr << "Failed to create GLFW window\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLAD init
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n"; return -1;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE);
    stbi_set_flip_vertically_on_load(true);

    // load shaders
    Shader basicShader("shader/basic.vs", "shader/basic.fs");
    Shader explodeShader(
        "shader/9.2.geometry_shader.vs",
        "shader/9.2.geometry_shader.fs",
        "shader/9.2.geometry_shader.gs"
    );
    Shader floorShader("shader/basic.vs", "shader/basic.fs");
    Shader particleShader("shader/particle.vs", "shader/particle.fs");

    // load models
    Model cubeModel("resources/objects/cube/cube.obj");
    Model carModelA("resources/objects/sportscar/sportsCar.obj");
    Model carModelB("resources/objects/sportscar/sportsCar.obj");

    // floor setup
    float floorVerts[] = {
         10.0f,0.0f, 10.0f, 1.0f,0.0f,
        -10.0f,0.0f, 10.0f, 0.0f,0.0f,
        -10.0f,0.0f,-10.0f, 0.0f,1.0f,
         10.0f,0.0f, 10.0f, 1.0f,0.0f,
        -10.0f,0.0f,-10.0f, 0.0f,1.0f,
         10.0f,0.0f,-10.0f, 1.0f,1.0f
    };
    GLuint floorVAO, floorVBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVerts), floorVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // textures
    unsigned int floorTex = loadTexture("resources/objects/textures/metal.png");
    floorShader.use(); floorShader.setInt("texture1", 0);

    // init particle system
    initParticles();

    // render loop
    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            float now = glfwGetTime();

            // 1) 플레이어(큐브) 위치/점프 초기화
            cubePos = INITIAL_CUBE_POS;
            isJumping = false;
            jumpVelocity = 0.0f;

            // 2) 대시 쿨다운 초기화
            lastDashTime = now - dashCooldown;

            // 3) Car A 폭발 상태 초기화
            for (int i = 0; i < 4; ++i) {
                exploded[i] = false;
                explosionStart[i] = -1.0f;
            }

            // 4) Car B 상태 초기화
            inCar = false;
            carBPos = INITIAL_CAR_B_POS;
            carBYaw = -90.0f;
            lastExitTime = -1.0f;

            // 5) 카메라 초기화
            yaw = -90.0f;
            pitch = 20.0f;
            lastX = SCR_WIDTH / 2.0f;
            lastY = SCR_HEIGHT / 2.0f;
            firstMouse = true;

            // 6) 파티클 모두 비활성화
            for (auto& p : particles) {
                p.active = false;
            }
        }

        processInput(window);

        // jump physics
        if (isJumping) {
            jumpVelocity += gravity * deltaTime;
            cubePos.y += jumpVelocity * deltaTime;
            if (cubePos.y <= groundY) {
                cubePos = glm::vec3(cubePos.x, groundY, cubePos.z);
                isJumping = false; jumpVelocity = 0;
            }
        }

        // dash
        if (!inCar && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS
            && currentTime - lastDashTime >= dashCooldown) {
            glm::vec3 fwd = glm::normalize(glm::vec3(
                cos(glm::radians(yaw)), 0,
                sin(glm::radians(yaw))
            ));
            cubePos += fwd * dashDistance;
            lastDashTime = currentTime;
        }

        // enter/exit Car B
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && currentTime - lastExitTime > 0.5f) {
            if (!inCar && glm::length(cubePos - carBPos) < 2.0f) {
                inCar = true;
            }
            else if (inCar) {
                inCar = false;
                cubePos = carBPos + glm::vec3(0, 0, -2.0f);
            }
            lastExitTime = currentTime;
        }

        // Car A explosion triggers & emit particles
        for (int i = 0; i < 4; i++) {
            if (!exploded[i]) {
                glm::vec3 ref = inCar ? carBPos : cubePos;
                if (glm::length(ref - INITIAL_CAR_A_POS[i]) < 1.5f) {
                    exploded[i] = true;
                    explosionStart[i] = currentTime;
                    emitParticles(INITIAL_CAR_A_POS[i]);
                }
            }
        }

        // update particle physics
        updateParticles(deltaTime);

        // Car B controls
        if (inCar) {
            glm::vec3 forwardDir = glm::normalize(glm::vec3(
                cos(glm::radians(carBYaw)), 0,
                sin(glm::radians(carBYaw))
            ));
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                carBPos += forwardDir * carBSpeed * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                carBPos -= forwardDir * carBSpeed * deltaTime;
            float steer = 0;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) steer -= 1;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) steer += 1;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) steer = -steer;
            carBYaw += steer * 80.0f * deltaTime;
        }

        // camera setup
        glm::vec3 target = inCar ? carBPos : cubePos;
        glm::vec3 dirVec = {
            cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
            sin(glm::radians(pitch)),
            sin(glm::radians(yaw)) * cos(glm::radians(pitch))
        };
        glm::vec3 flat = glm::normalize(glm::vec3(dirVec.x, 0, dirVec.z));
        glm::vec3 offset = -flat * distanceToCube + glm::vec3(0, 2, 0);
        glm::vec3 camPos = target + offset;
        glm::mat4 view = glm::lookAt(camPos, target, glm::vec3(0, 1, 0));
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f
        );

        // clear
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw floor
        floorShader.use();
        floorShader.setMat4("view", view);
        floorShader.setMat4("projection", projection);
        floorShader.setMat4("model", glm::mat4(1.0f));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTex);
        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // draw cube
        if (!inCar) {
            basicShader.use();
            basicShader.setMat4("view", view);
            basicShader.setMat4("projection", projection);
            glm::mat4 mc = glm::translate(glm::mat4(1.0f), cubePos);
            mc = glm::rotate(mc, glm::radians(-yaw + 90.0f), glm::vec3(0, 1, 0));
            mc = glm::scale(mc, glm::vec3(0.5f));
            basicShader.setMat4("model", mc);
            cubeModel.Draw(basicShader);
        }

        // draw Car A & explosions
        for (int i = 0; i < 4; i++) {
            if (!exploded[i]) {
                basicShader.use();
                basicShader.setMat4("view", view);
                basicShader.setMat4("projection", projection);
                glm::mat4 ma = glm::translate(glm::mat4(1.0f), INITIAL_CAR_A_POS[i]);
                ma = glm::scale(ma, glm::vec3(0.7f));
                basicShader.setMat4("model", ma);
                carModelA.Draw(basicShader);
            }
            else {
                float t = currentTime - explosionStart[i];
                if (t < explosionDuration) {
                    explodeShader.use();
                    explodeShader.setMat4("view", view);
                    explodeShader.setMat4("projection", projection);
                    explodeShader.setMat4("model",
                        glm::translate(glm::mat4(1.0f), INITIAL_CAR_A_POS[i])
                    );
                    explodeShader.setFloat("explosionTime", t);
                    carModelA.Draw(explodeShader);
                }
            }
        }

        // draw particles
        {
            // collect active positions
            std::vector<glm::vec3> activePos;
            activePos.reserve(MAX_PARTICLES);
            for (auto& p : particles)
                if (p.active) activePos.push_back(p.pos);

            // update VBO
            glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
            glBufferData(GL_ARRAY_BUFFER,
                activePos.size() * sizeof(glm::vec3),
                activePos.data(), GL_STREAM_DRAW);

            // render points
            particleShader.use();
            particleShader.setMat4("view", view);
            particleShader.setMat4("projection", projection);
            particleShader.setFloat("pointSize", 40.0f);
            glBindVertexArray(particleVAO);
            glDrawArrays(GL_POINTS, 0, activePos.size());
        }

        // draw Car B
        basicShader.use();
        basicShader.setMat4("view", view);
        basicShader.setMat4("projection", projection);
        glm::mat4 mb = glm::translate(glm::mat4(1.0f), carBPos);
        mb = glm::rotate(mb, glm::radians(-carBYaw + 90.0f), glm::vec3(0, 1, 0));
        mb = glm::scale(mb, glm::vec3(0.7f));
        basicShader.setMat4("model", mb);
        carModelB.Draw(basicShader);

        // swap & poll
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    glDeleteVertexArrays(1, &floorVAO);
    glDeleteBuffers(1, &floorVBO);
    glDeleteVertexArrays(1, &particleVAO);
    glDeleteBuffers(1, &particleVBO);
    glfwTerminate();
    return 0;
}

// initialize particle pool & VAO/VBO
void initParticles() {
    particles.resize(MAX_PARTICLES);
    for (auto& p : particles) {
        p.active = false;
        p.life = 0;
    }
    glGenVertexArrays(1, &particleVAO);
    glGenBuffers(1, &particleVBO);
    glBindVertexArray(particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(glm::vec3), nullptr, GL_STREAM_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
}

// spawn particles at explosion center
void emitParticles(glm::vec3 center) {
    for (auto& p : particles) {
        p.active = true;
        p.pos = center;
        glm::vec3 d(distrib(generator),
            distrib(generator),
            distrib(generator));
            float speed = 2.0f + distrib(generator) * 3.0f;
        p.velocity = glm::normalize(d) * speed;
        p.life = 1.0f;
    }
}

// update particles each frame
void updateParticles(float dt) {
    for (auto& p : particles) {
        if (!p.active) continue;
        p.life -= dt;
        if (p.life <= 0.0f) {
            p.active = false;
            continue;
        }
        p.velocity += glm::vec3(0.0f, -9.8f, 0.0f) * dt;
        p.pos += p.velocity * dt;
    }
}
//
// 창 크기 변경 시 뷰포트 조정
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// 마우스 이동 처리
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.02f;
    yaw += xoffset * sensitivity;
    pitch += yoffset * sensitivity;

    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
}

// 키 입력 처리
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float velocity = moveSpeed * deltaTime;
    glm::vec3 forward = glm::normalize(glm::vec3(
        cos(glm::radians(yaw)), 0.0f, sin(glm::radians(yaw))));
    glm::vec3 right = glm::normalize(glm::cross(forward,
        glm::vec3(0.0f, 1.0f, 0.0f)));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cubePos += forward * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cubePos -= forward * velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cubePos -= right * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cubePos += right * velocity;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !isJumping) {
        isJumping = true;
        jumpVelocity = jumpPower;
    }
}

// 텍스처 로드 유틸리티
unsigned int loadTexture(const char* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    int width, height, nrComponents;
    unsigned char* data =
        stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = (nrComponents == 1 ? GL_RED
            : nrComponents == 3 ? GL_RGB
            : GL_RGBA);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format,
            width, height, 0, format,
            GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,
            GL_TEXTURE_MIN_FILTER,
            GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,
            GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    else {
        std::cerr << "Failed to load texture at path: "
            << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}