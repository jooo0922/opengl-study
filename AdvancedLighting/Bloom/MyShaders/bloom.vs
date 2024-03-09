#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

/* fragment shader 단계로 출력할 interface block 선언 (관련 설명 AdvancedGLSL 참고)  */
// 참고로, block Name (VS_OUT)은 다음 단계의 쉐이더와 일치시켜야 하고, instance Name (vs_out) 쉐이더 단계마다 임의로 다르게 지어줘도 됨.
out VS_OUT {
  vec3 FragPos;
  vec3 Normal;
  vec2 TexCoords;
} vs_out;

/* 변환 행렬을 전송받는 uniform 변수 선언 */ 

// 투영 행렬
uniform mat4 projection;

// 뷰 행렬
uniform mat4 view;

// 모델 행렬
uniform mat4 model;

/* 기타 uniform 변수 선언 */

void main() {
  /* interface block 에 정의된 멤버변수들에 프래그먼트 쉐이더 단계로 보간하여 출력할 값들을 할당 */

  // 월드공간 정점 좌표로 변환하여 보간 출력
  vs_out.FragPos = vec3(model * vec4(aPos, 1.0));

  // 텍스쳐 좌표 보간 출력
  vs_out.TexCoords = aTexCoords;

  // 노멀벡터를 월드 공간 노멀벡터로 변환하기 위한 노멀행렬 계산
  mat3 normalMatrix = transpose(inverse(mat3(model)));
  vs_out.Normal = normalize(normalMatrix * aNormal);

  // 오브젝트 공간 좌표에 모델 행렬 > 뷰 행렬 > 투영 행렬 순으로 곱해서 좌표계를 변환시킴.
  gl_Position = projection * view * model * vec4(aPos, 1.0);
}