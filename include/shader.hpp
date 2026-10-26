#pragma once 

#include <glm/glm.hpp>

class Shader{
public:
  Shader(const char* vertex, const char* fragment);

  void checkForCompilationErrors(unsigned int shader, const char* type);
  void use() const;

  void setBool(const char* name, int value)const;
  void setInt(const char* name, int value)const;
  void setFloat(const char* name, float value)const;
  
  void setVec3(const char* name, glm::vec3& vec)const;
  void setVec4(const char* name, glm::vec4& vec)const;

  void setMat3(const char* name, glm::mat3& mat)const;
  void setMat4(const char* name, glm::mat4& mat)const;
private:
  unsigned int ID;
};

