#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

/* fragment shader 단계로 출력할 출력 변수 선언 */
out vec2 TexCoords;

void main() {
  // 프래그먼트 쉐이더 단계로 보간하여 출력할 uv 좌표값 할당
  TexCoords = aTexCoord;

  // 이미 NDC 좌표계 기준으로 정의된 QuadMesh 의 position attribute 를 그대로 출력변수에 할당
  gl_Position = vec4(aPos, 1.0);
}
