#pragma once 
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include <glm/glm.hpp>

struct BenchmarckResult{
  std::string algorithm;
  std::string testCase;
  int n;
  double timeUs;
};


struct Point{
  float x;
  float y;
  glm::vec3 color;
};

std::vector<BenchmarckResult> read_csv();
