#pragma once 
#include <cstddef>
#include <utility>
#include <vector>
#include "csv_reader.hpp"
#include "shader.hpp"

class Graph{
public:
  Graph(std::vector<std::vector<Point>> series, int width, int height);

  void buildBuffer(int windowWidth, int windowHeight);
  void setMesh();
  void draw();
private:
  std::vector<std::vector<Point>> series;
  unsigned int vao = 0, vbo = 0;
  std::vector<float> vertices;
  std::vector<std::pair<std::size_t, std::size_t>> drawRanges;
  std::pair<std::size_t, std::size_t> axesRange{0, 0};
  Shader shader;

  float graphLeft = 70.0f;
  float graphBottom = 55.0f;
  float graphRight = 25.0f;
  float graphTop = 30.0f;
};
