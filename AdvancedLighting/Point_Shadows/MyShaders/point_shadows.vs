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
} vs_out;

/* 변환 행렬을 전송받는 uniform 변수 선언 */ 

// 투영 행렬
uniform mat4 projection;

// 뷰 행렬
uniform mat4 view;

// 모델 행렬
uniform mat4 model;

// 노멀벡터 방향을 뒤집어서 계산할 지 여부를 결정하는 상태값
uniform bool reverse_normals;

void main() {
  // 정점 위치를 world space 좌표계까지 변환하여 프래그먼트 쉐이더 단계로 전송
  vs_out.FragPos = vec3(model * vec4(aPos, 1.0));

  // 정점 노멀을 world space 좌표계까지 변환하여 프래그먼트 쉐이더 단계로 전송 (모델행렬로부터 노말행렬 계산하여 적용)
  /*
    이때, Room 큐브를 렌더링하려면,
    큐브의 바깥 쪽 방향으로 정의된 원본 노멀벡터 aNormal 의 방향을
    안쪽 방향 노멀벡터로 뒤집어줘야 하므로, 

    reverse_normal 상태값에 따라
    노멀벡터 방향을 다르게 계산해 줌.
  */
  if(reverse_normals) {
    vs_out.Normal = transpose(inverse(mat3(model))) * (-1.0 * aNormal);
  } else {
    vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
  }

  // 텍스쳐 좌표를 보간하여 프래그먼트 쉐이더 단계로 전송
  vs_out.TexCoords = aTexCoords;

  /*
    omnidirectional shadow mapping 에서는 
    각 프래그먼트의 light space 좌표계를 사용하지 않고,

    '월드공간 거리값'을 기준으로 shadow testing 을 하므로, 
    별도로 light space 좌표계로 변환하는 코드를 작성할 필요가 없음! 
  */

  // 오브젝트 공간 좌표에 모델 행렬 > 뷰 행렬 > 투영 행렬 순으로 곱해서 좌표계를 변환시킴.
  gl_Position = projection * view * model * vec4(aPos, 1.0);
}