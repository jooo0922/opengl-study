#version 330 core

out vec4 FragColor;

void main() {
  // stencil testing 으로 그릴 Object outlining 에 적용할 색상 하드코딩
  FragColor = vec4(0.04, 0.28, 0.26, 1.0);
}