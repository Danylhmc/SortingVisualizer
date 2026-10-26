#version 330 core 
layout (location = 0) in vec2 apos;
layout (location = 1) in vec2 aTexCoords;

out vec2 texCoords;

uniform mat4 model;

void main(){
  gl_Position = model * vec4(apos, 0.0, 1.0);
  texCoords = aTexCoords;
}


