#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

/* 프래그먼트 쉐이더 단계로 보간하여 전송할 텍스쳐 좌표 출력 변수 선언 */
out vec2 TexCoords;

void main() {
  TexCoords = aTexCoords;

  // QuadMesh 의 position 값은 NDC 좌표를 기준으로 전송되므로, 별도의 변환 없이 gl_Position 에 바로 할당
  gl_Position = vec4(aPos, 1.0);
}