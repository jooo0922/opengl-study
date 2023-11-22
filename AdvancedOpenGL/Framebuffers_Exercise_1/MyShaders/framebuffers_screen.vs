#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main() {
  TexCoords = aTexCoords; // uv 좌표를 보간하여 프래그먼트 쉐이더로 전송

  // 버텍스 쉐이더로 전송된 스크린 평면 정점 위치값 aPos 는 x, y 값만 정의된 NDC 좌표계이므로,
  // 별도의 좌표계 변환 없이 z 값 0.0 만 추가하여 최종 좌표값으로 전달함.
  gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}