#version 330 core

out vec4 FragColor;

in vec3 ourColor;
in vec3 ourPos;

void main() {
  FragColor = vec4(ourPos, 1.0);
}

/*
  삼각형 왼쪽 하단이 검정색으로 보이는 이유

  삼각형 왼쪽 하단의 위치 attribute 변수(aPos)는
  vec3(-0.5, -0.5, 0.0) 으로 넘어옴.

  이때, glsl 은 색상값이 음수일 때 0.0 으로 clamping(잘라내기) 함.

  따라서, FragColor 최종 출력 변수에 들어가는 색상값이
  vec3(0.0, 0.0, 0.0, 1.0) 이 되어서 검정색으로 보이는 것.
*/