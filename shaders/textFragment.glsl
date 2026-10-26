#version 330 core 

out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D texture2;

void main(){
  FragColor = vec4(texture(texture2, texCoords));
}
