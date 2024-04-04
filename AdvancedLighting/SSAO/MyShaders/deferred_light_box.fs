#version 330 core

// default framebuffer 바인딩된 color attachment 에 저장할 출력 변수 선언
layout(location = 0) out vec4 FragColor;

// 광원 큐브 색상을 전송받을 uniform 변수 선언
uniform vec3 lightColor;

void main() {
  // FragColor 출력 변수에 씬에 렌더링되는 광원 큐브 색상을 할당함.
  FragColor = vec4(lightColor, 1.0);
}