#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

/* fragment shader 단계로 출력할 interface block 선언 (관련 설명 AdvancedGLSL 참고)  */
// 참고로, block Name (VS_OUT)은 다음 단계의 쉐이더와 일치시켜야 하고, instance Name (vs_out) 쉐이더 단계마다 임의로 다르게 지어줘도 됨.
out VS_OUT {
  vec3 FragPos; // 정점 위치를 world space 좌표계까지 변환하여 전송
  vec3 Normal; // 정점 노멀을 world space 좌표계까지 변환하여 전송
  vec2 TexCoords; // 텍스쳐 좌표를 보간하여 전송
  vec4 FragPosLightSpace; // 정점 위치를 light space 좌표계까지 변환하여 전송
} vs_out;

/* 변환 행렬을 전송받는 uniform 변수 선언 */ 

// 투영 행렬
uniform mat4 projection;

// 뷰 행렬
uniform mat4 view;

// 모델 행렬
uniform mat4 model;

// world space > light space 좌표계로의 변환 행렬
uniform mat4 lightSpaceMatrix;

void main() {
  // 정점 위치를 world space 좌표계까지 변환하여 프래그먼트 쉐이더 단계로 전송
  vs_out.FragPos = vec3(model * vec4(aPos, 1.0));

  // 정점 노멀을 world space 좌표계까지 변환하여 프래그먼트 쉐이더 단계로 전송 (모델행렬로부터 노말행렬 계산하여 적용)
  vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;

  // 텍스쳐 좌표를 보간하여 프래그먼트 쉐이더 단계로 전송
  vs_out.TexCoords = aTexCoords;

  // 정점의 world space 좌표를 light space 좌표계까지 한방에 변환 후, 프래그먼트 쉐이더 단계로 전송
  vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);

  // 오브젝트 공간 좌표에 모델 행렬 > 뷰 행렬 > 투영 행렬 순으로 곱해서 좌표계를 변환시킴.
  gl_Position = projection * view * model * vec4(aPos, 1.0);
}