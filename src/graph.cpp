#include "graph.hpp"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

Graph::Graph(std::vector<std::vector<Point>> series, int width, int height)
  :series(std::move(series)),shader(Shader("shaders/graphVertex.glsl", "shaders/graphFragment.glsl")){
  buildBuffer(width, height);
  setMesh();
}

void Graph::buildBuffer(int windowWidth, int windowHeight){
  vertices.clear();
  drawRanges.clear();
  axesRange = {0, 0};
  if (windowWidth <= 0 || windowHeight <= 0)
    return;

  float maxX = 0.0f;
  float maxY = 0.0f;
  for (const auto& line : series) {
    for (const Point& point : line) {
      maxX = std::max(maxX, point.x);
      maxY = std::max(maxY, point.y);
    }
  }
  const float graphWidth = std::max(1.0f, windowWidth - graphLeft - graphRight);
  const float graphHeight = std::max(1.0f, windowHeight - graphBottom - graphTop);
  maxX = maxX > 0.0f ? maxX * 1.05f : 1.0f;
  maxY = maxY > 0.0f ? maxY * 1.05f : 1.0f;

  for (const auto& line : series) {
    const std::size_t firstVertex = vertices.size() / 5;
    for (const Point& point : line) {
      const float posX = graphLeft + (point.x / maxX) * graphWidth;
      const float posY = graphBottom + (point.y / maxY) * graphHeight;

      const float ndcX = (posX / windowWidth) * 2.0f - 1.0f;
      const float ndcY = (posY / windowHeight) * 2.0f - 1.0f;

      vertices.push_back(ndcX);
      vertices.push_back(ndcY);
      vertices.push_back(point.color.x);
      vertices.push_back(point.color.y);
      vertices.push_back(point.color.z);
    }
    const std::size_t vertexCount = vertices.size() / 5 - firstVertex;
    if (vertexCount > 1)
      drawRanges.emplace_back(firstVertex, vertexCount);
  }

  const std::size_t firstAxisVertex = vertices.size() / 5;
  const glm::vec3 axisColor(0.12f, 0.12f, 0.12f);
  const auto appendAxisVertex = [&](float pixelX, float pixelY) {
    vertices.push_back((pixelX / windowWidth) * 2.0f - 1.0f);
    vertices.push_back((pixelY / windowHeight) * 2.0f - 1.0f);
    vertices.push_back(axisColor.r);
    vertices.push_back(axisColor.g);
    vertices.push_back(axisColor.b);
  };

  // X axis: horizontal; Y axis: vertical. The origin is the lower-left plot corner.
  appendAxisVertex(graphLeft, graphBottom);
  appendAxisVertex(windowWidth - graphRight, graphBottom);
  appendAxisVertex(graphLeft, graphBottom);
  appendAxisVertex(graphLeft, windowHeight - graphTop);
  axesRange = {firstAxisVertex, 4};
}

void Graph::setMesh(){
  if (vao == 0)
    glGenVertexArrays(1, &vao);
  if (vbo == 0)
    glGenBuffers(1, &vbo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2*sizeof(float)));

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void Graph::draw() {
  if (axesRange.second == 0)
    return;

  shader.use();

  glm::mat4 model = glm::mat4(1.0f);
  shader.setMat4("model", model);

  glBindVertexArray(vao);
  glLineWidth(2.5f);
  for (const auto& [first, count] : drawRanges) {
    glDrawArrays(GL_LINE_STRIP, static_cast<GLint>(first), static_cast<GLsizei>(count));
  }
  glLineWidth(1.5f);
  glDrawArrays(GL_LINES, static_cast<GLint>(axesRange.first), static_cast<GLsizei>(axesRange.second));
}
