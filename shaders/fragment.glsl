#version 330 core

in vec3 worldPosition;
out vec4 FragColor;

uniform sampler2D sceneColor;
uniform sampler2D sceneDepth;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 inverseProjection;

vec3 viewPositionFromDepth(vec2 uv, float depth) {
  vec4 clip = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
  vec4 position = inverseProjection * clip;
  return position.xyz / position.w;
}

void main() {
  vec3 floorColor = vec3(0.1, 0.1, 0.1);
  vec3 floorPosition = (view * vec4(worldPosition, 1.0)).xyz;
  vec3 normal = normalize(mat3(view) * vec3(0.0, 1.0, 0.0));
  vec3 incident = normalize(floorPosition);
  vec3 rayDirection = normalize(reflect(incident, normal));

  vec3 reflectedColor = vec3(0.0);
  float reflectionStrength = 0.0;
  vec3 rayPosition = floorPosition + normal * 0.025;

  for (int step = 0; step < 96; ++step) {
    rayPosition += rayDirection * (0.12 + float(step) * 0.008);
    vec4 clip = projection * vec4(rayPosition, 1.0);
    if (clip.w <= 0.0) break;

    vec3 ndc = clip.xyz / clip.w;
    vec2 uv = ndc.xy * 0.5 + 0.5;
    if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0))) ||
        ndc.z < -1.0 || ndc.z > 1.0) break;

    float depth = texture(sceneDepth, uv).r;
    if (depth < 0.99999) {
      vec3 scenePosition = viewPositionFromDepth(uv, depth);
      float separation = scenePosition.z - rayPosition.z;
      if (separation >= 0.0 && separation < 0.28) {
        reflectedColor = texture(sceneColor, uv).rgb;
        reflectionStrength = (1.0 - float(step) / 96.0) * 0.68;
        break;
      }
    }
  }

  float distanceFade = clamp(1.0 - length(worldPosition.xz) / 90.0, 0.25, 1.0);
  FragColor = vec4(mix(floorColor, reflectedColor, reflectionStrength * distanceFade), 1.0);
}
