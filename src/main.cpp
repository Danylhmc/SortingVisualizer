#define STB_IMAGE_IMPLEMENTATION
#include <cassert>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include "stb_image.h"
#include <iostream>
#include <algorithm>
#include <array>
#include <memory>
#include <numeric>
#include <random>
#include <string>
#include "shader.hpp"
#include "world.hpp"
#include "benchMark.hpp"
#include "csv_reader.hpp"
#include "graph.hpp"

static int scr_width = 1000;
static int scr_height = 600;

static unsigned int sceneFramebuffer = 0;
static unsigned int sceneColorTexture = 0;
static unsigned int sceneDepthTexture = 0;


static bool running = false;
static bool done = false;
static int speed = 100;
static int algo = 0;
static int n = 10;

static size_t eventIndex = 0;
double lastStepTime = glfwGetTime();

std::vector<int> a;
std::vector<int> sorted;

World world(a);

const glm::vec4 normalColor = {0.9176f, 0.8863f, 0.7176f, 1.0f};
const glm::vec4 compareColor = {0.8392f, 0.1569f, 0.1569f, 1.0f};
const glm::vec4 sortedColor = {0.337255f, 0.811765f, 0.882353f, 1.0f};
const glm::vec4 swapColor = {0.9686f, 0.4980f, 0.0f, 1.0f};

enum class EvType{
  Cmp, Swp, Ok
};

struct Event{
  EvType t;
  int i,j;
};

std::vector<Event> events;

static void rec(EvType t, int i, int j = -1){
  events.push_back({t, i, j});
}

static void doSwap(std::vector<int>& w, int i, int j){
  std::swap(w[i], w[j]);
  rec(EvType::Swp, i, j);
};

void bubbleSort(std::vector<int>& w) {
  int N = (int)w.size();

  for (int i = N - 1; i > 0; --i) {
    for (int j = 0; j < i; ++j) {
      rec(EvType::Cmp, j, j + 1);
      if (w[j] > w[j + 1])
        doSwap(w, j, j + 1);
    }

    rec(EvType::Ok, i);
  }

  if (N > 0) {
    rec(EvType::Ok, 0);
  }
}

void selectionSort(std::vector<int>& w) {
  int N = (int)w.size();
  for (int i = 0; i < N - 1; i++) {
    int minimum = i;

    for (int j = i + 1; j < N; j++) {
      rec(EvType::Cmp, minimum, j);
      if (w[minimum] > w[j])
        minimum = j;
    }

    if (minimum != i)
      doSwap(w, minimum, i);

    rec(EvType::Ok, i);
  }

  if (N > 0)
    rec(EvType::Ok, N - 1);
}

void insertionSort(std::vector<int>& w) {
  int N = (int)w.size();
  for (int i = 1; i < N; ++i) {
    int j = i;

    while (j > 0) {
      rec(EvType::Cmp, j - 1, j);

      if (w[j - 1] <= w[j]) break;

      doSwap(w, j - 1, j);
      --j;
    }
  }

  for (int i = 0; i < N; ++i) {
    rec(EvType::Ok, i);
  }
}

void reset(){
  a.resize(n);
  sorted.resize(a.size());
  std::iota(a.begin(), a.end(), 1);
  std::default_random_engine engine(445);
  std::shuffle(a.begin(), a.end(), engine);

  std::vector<int> w = a;
  world.generate(w);

  events.clear();
  sorted.assign(a.size(), false);
  eventIndex = 0;
  running = false;
  done = false;

  switch (algo){
    case 0: bubbleSort(a);
      break;
    case 1: selectionSort(a);
      break;
    case 2: insertionSort(a);
      break; 

    default: bubbleSort(a);
  }

  done = false;
}

void step(){
  if(eventIndex >= events.size()){
    done = true;
    return;
  }
  const Event& event = events[eventIndex];

  switch (event.t) {
    case EvType::Cmp:
      for (int i = 0; i < (int)a.size(); ++i) {
        if (!sorted[i]) {
          world.setCubeColor(i, normalColor);
        }
      }
      world.setCubeColor(event.i, compareColor);
      world.setCubeColor(event.j, compareColor);
      break;

    case EvType::Swp:
      world.swap(event.i, event.j);
      for (int i = 0; i < (int)a.size(); ++i) {
        if (!sorted[i]) {
          world.setCubeColor(i, normalColor);
        }
      }
      world.setCubeColor(event.i, swapColor);
      world.setCubeColor(event.j, swapColor);
       break;

    case EvType::Ok:
      sorted[event.i] = true;
      world.setCubeColor(event.i, sortedColor);
      break;
  }

  ++eventIndex;
}

double delayMs(){
  return (5.0 + 495.0 * std::pow(1.0 - speed / 100.0, 2.0))/100.0;
}

const std::array<std::string, 3> cases = {"best", "worst", "random"};
const std::array<std::string, 3> algorithms = {"Bubble", "Selection", "Insertion"};
const std::array<glm::vec3, 3> colors = {{{1.0f, 0.0f, 0.0f}, {0.0f, 0.65f, 0.0f}, {0.0f, 0.0f, 1.0f}}};

GLFWwindow* mainWindow = nullptr;
GLFWwindow* graphWindow = nullptr;
std::array<std::unique_ptr<Graph>, 3> graphs;
int selectedCase = 0;
int graphWidth = 1000;
int graphHeight = 600;
bool graphResultsLoaded = false;

void updateGraphWindowTitle(){
  const std::array<const char*, 3> caseTitles = {
    "meilleur cas (entree deja triee)",
    "pire cas (entree en ordre inverse)",
    "cas aleatoire"
  };
  std::string title = "SortingVisualizer - " + std::string(caseTitles[selectedCase]);
  title += " | rouge: Bubble, vert: Selection, bleu: Insertion | x: n, y: temps (us) | touches 1-3";
  glfwSetWindowTitle(graphWindow, title.c_str());
}

void loadResults(){
  if (graphResultsLoaded)
    return;

  loadBenchmark();
  const std::vector<BenchmarckResult> results = read_csv();
  std::array<std::array<std::vector<Point>, 3>, 3> pointsByCaseAndAlgorithm;

  for (const BenchmarckResult& result : results) {
    const auto algorithmIt = std::find(algorithms.begin(), algorithms.end(), result.algorithm);
    const auto caseIt = std::find(cases.begin(), cases.end(), result.testCase);
    if (algorithmIt == algorithms.end() || caseIt == cases.end())
      continue;

    const std::size_t algorithmIndex = static_cast<std::size_t>(algorithmIt - algorithms.begin());
    const std::size_t caseIndex = static_cast<std::size_t>(caseIt - cases.begin());
    pointsByCaseAndAlgorithm[caseIndex][algorithmIndex].push_back({
      static_cast<float>(result.n),
      static_cast<float>(result.timeUs),
      colors[algorithmIndex]
    });
  }

  if (results.empty()) {
    std::cerr << "Aucun resultat de benchmark n'a pu etre charge.\n";
    return;
  }

  glfwMakeContextCurrent(graphWindow);
  for (std::size_t caseIndex = 0; caseIndex < cases.size(); ++caseIndex) {
    std::vector<std::vector<Point>> series;
    series.reserve(algorithms.size());
    for (std::size_t algorithmIndex = 0; algorithmIndex < algorithms.size(); ++algorithmIndex) {
      series.push_back(std::move(pointsByCaseAndAlgorithm[caseIndex][algorithmIndex]));
    }
    graphs[caseIndex] = std::make_unique<Graph>(std::move(series), graphWidth, graphHeight);
  }
  graphResultsLoaded = true;
  selectedCase = 0;
  updateGraphWindowTitle();
  glfwMakeContextCurrent(mainWindow);
}

void toggleGraphWindow(){
  loadResults();
  if (!graphResultsLoaded)
    return;

  if (glfwGetWindowAttrib(graphWindow, GLFW_VISIBLE)) {
    glfwHideWindow(graphWindow);
  } else {
    updateGraphWindowTitle();
    glfwShowWindow(graphWindow);
    glfwFocusWindow(graphWindow);
  }
}

static void graphKeyCallback(GLFWwindow* window, int key, int, int action, int){
  if (action == GLFW_RELEASE)
    return;

  if (key >= GLFW_KEY_1 && key <= GLFW_KEY_3 && action == GLFW_PRESS) {
    selectedCase = key - GLFW_KEY_1;
    updateGraphWindowTitle();
  } else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwHideWindow(window);
  }
}

static void resizeSceneFramebuffer(int width, int height) {
  if (sceneFramebuffer == 0) return;
  glBindTexture(GL_TEXTURE_2D, sceneColorTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  glBindTexture(GL_TEXTURE_2D, sceneDepthTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glBindTexture(GL_TEXTURE_2D, 0);
}

glm::vec3 cameraPos = {-50.0f, 2.0f, -20.0f};
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

static void key_callback(GLFWwindow* window, int key, int, int action, int) {
  if (action == GLFW_RELEASE) return;
  bool press = action == GLFW_PRESS;

  if (key >= GLFW_KEY_1 && key <= GLFW_KEY_3){
    if (press){
      algo = key - GLFW_KEY_1;
      reset();
    }
    return;
  }

  switch (key) {
    case GLFW_KEY_ESCAPE:
      if (press) glfwSetWindowShouldClose(window, true);
      break;

    case GLFW_KEY_SPACE:
      if (press && !done){
        running = !running;
      }
      break;

    case GLFW_KEY_RIGHT:
      running = false;
      if(!done) step();
      break;

    case GLFW_KEY_R:
      if (press) reset();
      break;

    case GLFW_KEY_UP:
      speed = std::min(200, speed + 5);
      break;

    case GLFW_KEY_DOWN:
      speed = std::max(0,   speed - 5);
      break;

    case GLFW_KEY_EQUAL:
    case GLFW_KEY_KP_ADD:
      n = std::min(120, n + 1);
      reset();
      break;

    case GLFW_KEY_MINUS:
    case GLFW_KEY_KP_SUBTRACT:
      n = std::max(8,   n - 1);
      reset();
      break;

    case GLFW_KEY_L:
      if (press) toggleGraphWindow();
      break;
  }
}

bool firstMouse = true;
float yaw = 2.31f;
float pitch = -4.0f;

float lastX, lastY;
float sensitivity = 0.08f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos){
  if(firstMouse){
    lastX = (float)xpos;
    lastY = (float)ypos;

    firstMouse = false;
  }

  float xoffset = (float)xpos - lastX;
  float yoffset = lastY - (float)ypos;

  lastX = (float)xpos;
  lastY = (float)ypos;

  xoffset *= sensitivity;
  yoffset *= sensitivity;

  yaw += xoffset;
  pitch += yoffset;

  if(pitch > 89.9f) pitch = 89.9f;
  if(pitch < -89.9f) pitch = -89.9f;

  glm::vec3 direction;

  direction.x = glm::cos(glm::radians(yaw))* glm::cos(glm::radians(pitch));
  direction.y = glm::sin(glm::radians(pitch));
  direction.z = glm::cos(glm::radians(pitch))* glm::sin(glm::radians(yaw));

  cameraFront = glm::normalize(direction);
}

float fov = 45.0f;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
  fov -= (float)yoffset;
  if(fov < 10) fov = 10;
  if(fov > 90) fov = 90;
}

void frameBufferSize_callback(GLFWwindow* window, int width, int height){
  if (width <= 0 || height <= 0)
    return;
  glfwMakeContextCurrent(window);
  glViewport(0, 0, width, height);

  scr_width = width;
  scr_height = height;
  resizeSceneFramebuffer(width, height);
}

void graphFrameBufferSize_callback(GLFWwindow* window, int width, int height){
  if (width <= 0 || height <= 0)
    return;
  glfwMakeContextCurrent(window);
  glViewport(0, 0, width, height);
  graphWidth = width;
  graphHeight = height;
  for (auto& graph : graphs) {
    if (graph) {
      graph->buildBuffer(graphWidth, graphHeight);
      graph->setMesh();
    }
  }
}

void handleInputs(GLFWwindow* window){
  glm::vec3 right = glm::normalize(glm::cross(cameraFront, cameraUp));

  if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += glm::vec3(cameraFront.x, 0, cameraFront.z);
  if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= glm::vec3(cameraFront.x, 0, cameraFront.z);
  if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += right;
  if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= right;
  if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) cameraPos += cameraUp;
  if(glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) cameraPos -= cameraUp;

  if(glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS) glPolygonMode(GL_LINE, GL_FRONT_AND_BACK);
  if(glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS) glPolygonMode(GL_FILL, GL_FRONT_AND_BACK);

}

int main() {
  if (!glfwInit()){
    glfwTerminate();
    std::cerr << "Error glfwInit() \n";
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  mainWindow = glfwCreateWindow(scr_width, scr_height,"Sorting Visualizer",nullptr, nullptr);
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  graphWindow = glfwCreateWindow(scr_width, scr_height,"Graph",nullptr, nullptr);
  glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

  if (!mainWindow || !graphWindow) {
    glfwTerminate();
    std::cerr << "Error creating a window\n";
    return -1;
  }

  mouse_callback(mainWindow, 0, 0);

  glfwMakeContextCurrent(mainWindow);
  glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetKeyCallback(mainWindow, key_callback);
  glfwSetCursorPosCallback(mainWindow, mouse_callback);
  glfwSetScrollCallback(mainWindow, scroll_callback);
  glfwSetFramebufferSizeCallback(mainWindow, frameBufferSize_callback);
  glfwSetKeyCallback(graphWindow, graphKeyCallback);
  glfwSetFramebufferSizeCallback(graphWindow, graphFrameBufferSize_callback);

  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
    glfwTerminate();
    std::cerr << "Error glad\n";
    return -1;
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  float vertices[] = {
    -0.5f, 0.0f, -0.5f,
    -0.5f, 0.0f,  0.5f,
    0.5f, 0.0f,  0.5f,

    -0.5f, 0.0f, -0.5f,
    0.5f, 0.0f,  0.5f,
    0.5f, 0.0f, -0.5f
  };

  unsigned int vao, vbo;

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, false, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  Shader shader("./shaders/vertex.glsl", "./shaders/fragment.glsl");

  glGenFramebuffers(1, &sceneFramebuffer);
  glGenTextures(1, &sceneColorTexture);
  glBindTexture(GL_TEXTURE_2D, sceneColorTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glGenTextures(1, &sceneDepthTexture);
  glBindTexture(GL_TEXTURE_2D, sceneDepthTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  resizeSceneFramebuffer(scr_width, scr_height);
  glBindFramebuffer(GL_FRAMEBUFFER, sceneFramebuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneColorTexture, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, sceneDepthTexture, 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "Error: reflection framebuffer is incomplete\n";
    return -1;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  unsigned int textTexture;
  glGenTextures(1, &textTexture);
  glBindTexture(GL_TEXTURE_2D, textTexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  stbi_set_flip_vertically_on_load(true);
  unsigned char* data;
  int width, height, nbChannels;

  data = stbi_load("src/text-image.png", &width, &height, &nbChannels, 0);

  if(!data){
    std::cerr << "Error loading text texture\n";
    return -1;
  }else{
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  }

  stbi_image_free(data);
  glActiveTexture(GL_TEXTURE2);

  static float textVertices[] = {
    -0.9f,  0.9f,      0.0f, 1.0f,
    -0.9f,  0.5f,      0.0f, 0.0f,
    -0.3f,  0.5f,      1.0f, 0.0f,

    -0.9f,  0.9f,      0.0f, 1.0f,
    -0.3f,  0.5f,      1.0f, 0.0f,
    -0.3f,  0.9f,      1.0f, 1.0f
};

  unsigned int textVao, textVbo;

  glGenVertexArrays(1, &textVao);
  glGenBuffers(1, &textVbo);

  glBindVertexArray(textVao);

  glBindBuffer(GL_ARRAY_BUFFER, textVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(textVertices), textVertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, false, 4 * sizeof(float), (void*)0);
  glVertexAttribPointer(1, 2, GL_FLOAT, false, 4 * sizeof(float), (void*)(2 * sizeof(float)));

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  Shader textShader("./shaders/textVertex.glsl", "./shaders/textFragment.glsl");

  reset();

  glEnable(GL_DEPTH_TEST);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  while (!glfwWindowShouldClose(mainWindow)) {
    glfwPollEvents();
    if (glfwWindowShouldClose(graphWindow)) {
      glfwHideWindow(graphWindow);
      glfwSetWindowShouldClose(graphWindow, GLFW_FALSE);
    }
    glfwMakeContextCurrent(mainWindow);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    handleInputs(mainWindow);
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glm::mat4 projection = glm::perspective(glm::radians(fov), (float)scr_width/(float)scr_height, 0.1f, 1000.0f);

    double currentTime = glfwGetTime();

    if (currentTime - lastStepTime >= delayMs() && running) {
      step();
      lastStepTime = currentTime;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, sceneFramebuffer);
    glViewport(0, 0, scr_width, scr_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    world.draw(view, projection);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, scr_width, scr_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(1.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(1000.0f, 1.0f, 1000.0f));

    shader.use();

    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    glm::mat4 inverseProjection = glm::inverse(projection);
    shader.setMat4("inverseProjection", inverseProjection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneColorTexture);
    shader.setInt("sceneColor", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, sceneDepthTexture);
    shader.setInt("sceneDepth", 1);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    world.draw(view, projection);

    glm::mat4 textModel = glm::mat4(1.0f);
    textModel = glm::scale(textModel, glm::vec3(0.6f));
    textModel = glm::translate(textModel, glm::vec3(0.5f, -2.0f, 0.0f));

    textShader.use();
    textShader.setMat4("model", textModel);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textTexture);
    textShader.setInt("texture2", 2);

    glBindVertexArray(textVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glfwSwapBuffers(mainWindow);

    if (graphResultsLoaded && glfwGetWindowAttrib(graphWindow, GLFW_VISIBLE)) {
      glfwMakeContextCurrent(graphWindow);
      glViewport(0, 0, graphWidth, graphHeight);
      glClearColor(0.96f, 0.96f, 0.96f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      if (graphs[selectedCase])
        graphs[selectedCase]->draw();
      glfwSwapBuffers(graphWindow);
    }
  }

  glfwMakeContextCurrent(graphWindow);
  for (auto& graph : graphs)
    graph.reset();
  glfwSetFramebufferSizeCallback(mainWindow, nullptr);
  glfwSetFramebufferSizeCallback(graphWindow, nullptr);
  glfwDestroyWindow(graphWindow);
  glfwDestroyWindow(mainWindow);
  glfwTerminate();

  return 0;
}

