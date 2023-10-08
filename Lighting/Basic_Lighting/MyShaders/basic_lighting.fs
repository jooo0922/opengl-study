#version 330 core

out vec4 FragColor;

// OpenGL 에서 전송해 줄 uniform 변수들 선언
uniform vec3 objectColor; // 물체가 반사할 색상값(= 물체의 색상)
uniform vec3 lightColor; // 광원의 색상값 (= 조명 색상)

void main() {
  // 물체의 색상과 조명 색상을 component-wise 곱으로 계산하여 최종 색상 결정
  FragColor = vec4(lightColor * objectColor, 1.0);
}

/*
  본문에 따르면, 
  
  objectColor * lightColor 값이 
  물체가 반사하는 빛의 양이고, 

  lightColor - (objectColor * lightColor) 값이 
  물체가 흡수하는 빛의 양이 나온다고 함.
*/