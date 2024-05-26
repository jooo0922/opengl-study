#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

/* fragment shader 단계로 출력할 출력 변수 선언 */
out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

/* 변환 행렬을 전송받는 uniform 변수 선언 */ 

// 투영 행렬
uniform mat4 projection;

// 뷰 행렬
uniform mat4 view;

// 모델 행렬
uniform mat4 model;

// 노멀 행렬
uniform mat3 normalMatrix;

void main() {
  // 프래그먼트 쉐이더 단계로 보간하여 출력할 값들을 World Space 로 변환하여 할당
  TexCoords = aTexCoords;
  WorldPos = vec3(model * vec4(aPos, 1.0));
  Normal = normalMatrix * aNormal;

  // World Space 좌표에 뷰 행렬 > 투영 행렬 순으로 곱해서 좌표계를 변환시킴.
  gl_Position = projection * view * vec4(WorldPos, 1.0);
}
