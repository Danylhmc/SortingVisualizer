#include <glad/glad.h>
#include "shader.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

Shader::Shader(const char* vertex, const char* fragment){
  std::fstream vFile, fFile;
  vFile.open(vertex);
  fFile.open(fragment);

  if(!vFile.is_open())
    std::cerr << "Error vertex path \n";

  if(!fFile.is_open())
    std::cerr << "Error fragment path \n";

  std::stringstream vStream, fStream;
  vStream << vFile.rdbuf();
  fStream << fFile.rdbuf();

  std::string vString, fString;
  vString = vStream.str();
  fString = fStream.str();

  const char* vCode = vString.c_str();
  const char* fCode = fString.c_str();

  unsigned int Vertex, Fragment;

  Vertex = glCreateShader(GL_VERTEX_SHADER);
  Fragment = glCreateShader(GL_FRAGMENT_SHADER);

  glShaderSource(Vertex, 1, &vCode, NULL);
  glShaderSource(Fragment, 1, &fCode, NULL);

  glCompileShader(Vertex);
  checkForCompilationErrors(Vertex, "Vertex");
  glCompileShader(Fragment);
  checkForCompilationErrors(Fragment, "Fragment");

  ID = glCreateProgram();

  glAttachShader(ID, Vertex);
  glAttachShader(ID, Fragment);

  glLinkProgram(ID);

  int success;
  char infoLog[1024];

  glGetProgramiv(ID, GL_LINK_STATUS, &success);
  if(!success){
    glGetProgramInfoLog(ID, 1024,NULL , infoLog);
    std::cerr << "Error Linking Program: " << infoLog << "\n";
  }
}

void Shader::checkForCompilationErrors(unsigned int shader, const char *type){
  int success;
  char infoLog[1024];

  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if(!success){
    glGetShaderInfoLog(shader, 1024,NULL , infoLog);
    std::cerr << "Error compiling" << type << "Shader:" << infoLog << "\n";
  }
}

void Shader::use()const{
  glUseProgram(ID);
}

void Shader::setBool(const char* name, int value)const {
  glUniform1i(glGetUniformLocation(ID, name), value);
};

void Shader::setInt(const char* name, int value)const{
  glUniform1i(glGetUniformLocation(ID, name), value);
};

void Shader::setFloat(const char* name, float value)const{
  glUniform1f(glGetUniformLocation(ID, name), value);
};

void Shader::setVec3(const char* name, glm::vec3& vec)const{
  glUniform3fv(glGetUniformLocation(ID, name),1, glm::value_ptr(vec));
};

void Shader::setVec4(const char* name, glm::vec4& vec)const{
  glUniform4fv(glGetUniformLocation(ID, name), 1, glm::value_ptr(vec));
};

void Shader::setMat3(const char* name, glm::mat3& mat)const{
  glUniformMatrix3fv(glGetUniformLocation(ID, name), 1, false, glm::value_ptr(mat));
};

void Shader::setMat4(const char* name, glm::mat4& mat)const{
  glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, false, glm::value_ptr(mat));
};
