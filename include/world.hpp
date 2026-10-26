#pragma once
#include <glm/glm.hpp>
#include <cube.hpp>
#include <vector>

class World{
public:
  std::vector<int>& w;
  World(std::vector<int>& w)
    :w(w){}

  void generate(std::vector<int>& w) {
    this->w = w;
    if (cubes.size() > w.size()) cubes.erase(cubes.begin() + w.size(), cubes.end());
    for (int i = 0; i < (int)w.size(); ++i) {
      float height = (float)w[i];
      const glm::vec3 position(1.0f, height, -3.0f * i - 5.0f);
      const glm::vec3 size(1.0f, height, 1.0f);
      const glm::vec4 color(10.9176f, 0.8863f, 0.7176f, 1.0f);

      if (i < (int)cubes.size()) {
        cubes[i].setPosition(position);
        cubes[i].setSize(size);
        cubes[i].setColor(color);
      } else {
        cubes.emplace_back(position, size, color);
      }
    }
  }

  void draw(glm::mat4& view, glm::mat4& projection){
    for(int i = 0; i < cubes.size(); ++i){
      cubes[i].draw(view, projection);
    }
  }

  void swap(int i, int j){
    glm::vec3 sizeI = cubes[i].getSize();
    glm::vec3 sizeJ = cubes[j].getSize();

    glm::vec3 posI = cubes[i].getPosition();
    glm::vec3 posJ = cubes[j].getPosition();

    cubes[i].setSize(sizeJ);
    cubes[j].setSize(sizeI);

    cubes[i].setPosition(glm::vec3(posI.x, posJ.y, posI.z));
    cubes[j].setPosition(glm::vec3(posJ.x, posI.y, posJ.z));
  }

  void setCubeColor(int index, glm::vec4 color) {
    if (index < 0 || index >= (int)cubes.size()) return;
    cubes[index].setColor(color);
  }

private:
  std::vector<Cube> cubes;
};

