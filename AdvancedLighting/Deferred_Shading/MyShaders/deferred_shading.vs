#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

// fragment shader 단계로 전송할 텍스쳐 좌표 출력 변수 선언
vec2 TexCoords;

void main() {
  // 텍스쳐 좌표 보간 출력
  TexCoords = aTexCoords;

  // NDC 좌표계 기준으로 미리 정의된 QuadMesh 의 위치값을 별도의 변환 없이 출력 변수에 그대로 할당 
  gl_Position = vec4(aPos, 1.0);
}