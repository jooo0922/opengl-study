#version 330 core

out vec4 FragColor;

void main() {
  // geometry shader 에서 binding 하여 생성한 line_strip primitive 를 
  // mono-color(노란색)으로 출력
  FragColor = vec4(1.0, 1.0, 0.0, 1.0);
}