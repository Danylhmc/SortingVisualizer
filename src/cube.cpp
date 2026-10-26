#include "cube.hpp"

Cube::Cube(glm::vec3 position, glm::vec3 size, glm::vec3 color)
  :position(position),size(size),color(glm::vec4(color,1.0f)),shader(Shader(SORTING_VISUALIZER_SOURCE_DIR "/shaders/cubeVertex.glsl", SORTING_VISUALIZER_SOURCE_DIR "/shaders/cubeFragment.glsl")){
  setMesh();
}

glm::vec3 Cube::getPosition()const{
  return position;
}

glm::vec3 Cube::getSize()const{
  return size;
}

glm::vec4 Cube::getColor()const{
  return color;
}

void Cube::setPosition(const glm::vec3& newPos){
  position = newPos;
}

void Cube::setSize(const glm::vec3& newSize){
  size = newSize;
}

void Cube::setColor(const glm::vec4& newColor){
  color = newColor;
}

void Cube::setMesh(){
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, false, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
}

void Cube::draw(glm::mat4& view, glm::mat4& projection){
  shader.use();

  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, position);
  model = glm::scale(model, size);

  shader.setMat4("model", model);
  shader.setMat4("view", view);
  shader.setMat4("projection", projection);

  shader.setVec4("color", color);

  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 36);
}

