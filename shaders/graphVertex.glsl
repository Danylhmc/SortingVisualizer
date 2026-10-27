#version 330 core 

layout(location = 0)in vec2 apos;
layout(location = 1)in vec3 aColor;

out vec4 color;

uniform mat4 model;

void main(){
  color = vec4(aColor, 1.0);
  gl_Position = model * vec4(apos, 0.0, 1.0);
}
