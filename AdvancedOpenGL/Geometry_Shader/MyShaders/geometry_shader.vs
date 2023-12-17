#version 330 core

layout(location = 0) in vec2 aPos; // 정점 위치 데이터 (NDC 좌표)
layout(location = 1) in vec3 aColor; // 정점 색상 데이터

// geometry shader 단계로 출력할 interface block 선언
out VS_OUT {
  vec3 color;
} vs_out;

void main() {
  // geometry shader 단계로 전달할 정점 색상 데이터 할당
  vs_out.color = aColor;

  // 좌표계 변환없이 전달받은 NDC 좌표를 geometry shader 단계로 직접 전송
  gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}