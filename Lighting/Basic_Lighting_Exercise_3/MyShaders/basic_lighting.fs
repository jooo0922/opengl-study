#version 330 core

out vec4 FragColor;

// 버텍스 쉐이더에서 전송된 보간 변수
in vec3 LightingColor; // 버텍스 쉐이더에서 계산된 최종 조명 색상을 보간하여 전달받음 (Gouraud Shading)

// 조명 계산 관련 uniform 변수 선언
uniform vec3 objectColor;

void main() {
  FragColor = vec4(LightingColor * objectColor, 1.0);
}

/*
  본문에 따르면, 
  
  objectColor * lightColor 값이 
  물체가 반사하는 빛의 양이고, 

  lightColor - (objectColor * lightColor) 값이 
  물체가 흡수하는 빛의 양이 나온다고 함.
*/