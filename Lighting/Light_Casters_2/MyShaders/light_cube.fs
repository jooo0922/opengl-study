#version 330 core

out vec4 FragColor;

void main() {
  // 광원 색상은 빛을 받는 큐브의 조명 색상 계산에 영향을 받지 않도록, 별도의 쉐이더에 상수인 흰색(vec4(1.0))값을 할당함.
  FragColor = vec4(1.0);
}