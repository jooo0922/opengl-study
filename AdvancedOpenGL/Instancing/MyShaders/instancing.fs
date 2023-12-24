#version 330 core

out vec4 FragColor;

// 버텍스 쉐이더 단계에서 보간되는 색상값 입력변수 
in vec3 fColor;

void main() {
  // 빨간색을 최종 색상으로 출력
  FragColor = vec4(fColor, 1.0);
}