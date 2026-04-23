#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

enum GameState { STATE_ACTIVE, STATE_LOSS };
GameState currentState = STATE_ACTIVE;

glm::vec3 cameraPos = glm::vec3(0.0f, 6.0f, 10.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 playerPos = glm::vec3(0.0f, 2.0f, 0.0f);
glm::vec3 playerVel = glm::vec3(0.0f);
float moveSpeed = 7.0f;
float jumpForce = 7.5f;
bool onGround = false;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct Platform { glm::vec3 pos; glm::vec3 size; };
struct Star { glm::vec3 pos; bool collected; };

std::vector<Platform> platforms;
std::vector<Star> stars;
int platformsCount = 0;

void addPlatform() {
    float x = (platformsCount % 2 == 0) ? 2.5f : -2.5f;
    if (platformsCount == 0) x = 0.0f;
    float y = platformsCount * 0.8f;
    float z = platformsCount * -5.5f;
    platforms.push_back({ glm::vec3(x, y, z), glm::vec3(3.5f, 0.4f, 3.5f) });
    stars.push_back({ glm::vec3(x, y + 1.3f, z), false });
    platformsCount++;
}

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 model; uniform mat4 view; uniform mat4 projection;\n"
"void main() { gl_Position = projection * view * model * vec4(aPos, 1.0); }";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor; uniform vec4 color;\n"
"void main() { FragColor = color; }";

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    if (currentState == STATE_LOSS) {
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            playerPos = glm::vec3(0.0f, 2.0f, 0.0f); playerVel = glm::vec3(0.0f);
            platforms.clear(); stars.clear(); platformsCount = 0;
            for (int i = 0; i < 10; i++) addPlatform();
            currentState = STATE_ACTIVE;
        }
        return;
    }
    float v = moveSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) playerPos.z -= v;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) playerPos.z += v;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) playerPos.x -= v;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) playerPos.x += v;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && onGround) { playerVel.y = jumpForce; onGround = false; }
}

float cubeVertices[] = {
    -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f, 0.5f, 0.5f,-0.5f, -0.5f, 0.5f,-0.5f, -0.5f,-0.5f,-0.5f,
    -0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f,-0.5f, 0.5f,
    -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f, -0.5f,-0.5f,-0.5f, -0.5f,-0.5f,-0.5f, -0.5f,-0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
     0.5f, 0.5f, 0.5f,  0.5f, 0.5f,-0.5f,  0.5f,-0.5f,-0.5f, 0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
    -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f, 0.5f,-0.5f, 0.5f, -0.5f,-0.5f, 0.5f, -0.5f,-0.5f,-0.5f,
    -0.5f, 0.5f,-0.5f,  0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f
};

int main() {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Sky Runner Final", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_DEPTH_TEST); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    unsigned int vs = glCreateShader(GL_VERTEX_SHADER); glShaderSource(vs, 1, &vertexShaderSource, NULL); glCompileShader(vs);
    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER); glShaderSource(fs, 1, &fragmentShaderSource, NULL); glCompileShader(fs);
    unsigned int prog = glCreateProgram(); glAttachShader(prog, vs); glAttachShader(prog, fs); glLinkProgram(prog);

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO);
    glBindVertexArray(VAO); glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);

    for (int i = 0; i < 10; i++) addPlatform();

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = (float)glfwGetTime(); deltaTime = currentFrame - lastFrame; lastFrame = currentFrame;
        processInput(window);

        if (currentState == STATE_ACTIVE) {
            playerVel.y -= 15.0f * deltaTime; playerPos += playerVel * deltaTime;
            onGround = false;
            for (int i = 0; i < platforms.size(); i++) {
                Platform& p = platforms[i];
                float minX = p.pos.x - p.size.x * 0.5f; float maxX = p.pos.x + p.size.x * 0.5f;
                float minZ = p.pos.z - p.size.z * 0.5f; float maxZ = p.pos.z + p.size.z * 0.5f;
                float maxY = p.pos.y + p.size.y * 0.5f;
                if (playerPos.x >= minX - 0.2f && playerPos.x <= maxX + 0.2f && playerPos.z >= minZ - 0.2f && playerPos.z <= maxZ + 0.2f) {
                    if (playerPos.y <= maxY + 0.5f && playerPos.y >= p.pos.y - 0.3f) {
                        playerPos.y = maxY + 0.5f; playerVel.y = 0; onGround = true;
                        if (i > (int)platforms.size() - 5) addPlatform(); break;
                    }
                }
            }
            if (playerPos.y < -15.0f) currentState = STATE_LOSS;
            for (auto& s : stars) { if (!s.collected && glm::distance(playerPos, s.pos) < 1.2f) s.collected = true; }

            // الرسم في حالة اللعب النشط
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glUseProgram(prog);

            glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 1000.0f);
            glm::mat4 view = glm::lookAt(cameraPos + glm::vec3(0, playerPos.y * 0.3f, playerPos.z), playerPos, cameraUp);
            glBindVertexArray(VAO);

            // رسم الخلفية الثابتة
            glDisable(GL_DEPTH_TEST);
            glm::mat4 fixedView = glm::mat4(1.0f);
            glUniformMatrix4fv(glGetUniformLocation(prog, "view"), 1, GL_FALSE, glm::value_ptr(fixedView));

            glm::mat4 skyModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, -1.0f));
            skyModel = glm::scale(skyModel, glm::vec3(4.0f, 2.0f, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(prog, "model"), 1, GL_FALSE, glm::value_ptr(skyModel));
            glUniform4f(glGetUniformLocation(prog, "color"), 0.5f, 0.7f, 1.0f, 1.0f); // سماوي
            glDrawArrays(GL_TRIANGLES, 0, 36);

            glm::mat4 grassModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.5f, -1.0f));
            grassModel = glm::scale(grassModel, glm::vec3(4.0f, 2.0f, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(prog, "model"), 1, GL_FALSE, glm::value_ptr(grassModel));
            glUniform4f(glGetUniformLocation(prog, "color"), 0.2f, 0.6f, 0.2f, 1.0f); // أخضر
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glEnable(GL_DEPTH_TEST);

            // رسم الكائنات
            glUniformMatrix4fv(glGetUniformLocation(prog, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(prog, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            for (auto& p : platforms) {
                glm::mat4 m = glm::translate(glm::mat4(1.0f), p.pos); m = glm::scale(m, p.size);
                glUniformMatrix4fv(glGetUniformLocation(prog, "model"), 1, GL_FALSE, glm::value_ptr(m));
                glUniform4f(glGetUniformLocation(prog, "color"), 0.5f, 0.3f, 0.1f, 1.0f);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
            glm::mat4 pM = glm::translate(glm::mat4(1.0f), playerPos); pM = glm::scale(pM, glm::vec3(0.6f, 1.0f, 0.6f));
            glUniformMatrix4fv(glGetUniformLocation(prog, "model"), 1, GL_FALSE, glm::value_ptr(pM));
            glUniform4f(glGetUniformLocation(prog, "color"), 1.0f, 0.2f, 0.2f, 1.0f);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            for (auto& s : stars) {
                if (s.collected) continue;
                glm::mat4 sM = glm::translate(glm::mat4(1.0f), s.pos);
                sM = glm::rotate(sM, (float)glfwGetTime() * 2.0f, glm::vec3(0, 1, 0)); sM = glm::scale(sM, glm::vec3(0.4f));
                glUniformMatrix4fv(glGetUniformLocation(prog, "model"), 1, GL_FALSE, glm::value_ptr(sM));
                glUniform4f(glGetUniformLocation(prog, "color"), 1.0f, 1.0f, 0.0f, 1.0f);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }
        else if (currentState == STATE_LOSS) {
            // شاشة الخسارة: حمراء بالكامل (بدون سماء أو عشب)
            glClearColor(0.8f, 0.0f, 0.0f, 1.0f); // لون أحمر غامق للخلفية
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        glfwSwapBuffers(window); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}