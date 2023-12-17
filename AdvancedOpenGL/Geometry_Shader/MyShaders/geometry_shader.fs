#version 330 core

out vec4 FragColor;

// 지오메트리 쉐이더 단계에서 전달받은 각 정점으로부터 보간된 색상 데이터
in vec3 fColor;

void main() {
  // 지오메트리 쉐이더에서 전달받아 보간된 색상을 최종 색상으로 출력
  FragColor = vec4(fColor, 1.0);
}