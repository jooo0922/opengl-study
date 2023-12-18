#version 330 core

layout(location = 0) in vec3 aPos;

// 노멀벡터 시각화에 사용할 쉐이더에서는 Assimp 에서 로드한 정점 노멀 데이터를 사용할 것임!
layout(location = 1) in vec3 aNormal; 

// geometry shader 단계로 출력할 interface block 선언
out VS_OUT {
  vec3 normal;
} vs_out;

// glm::mat4 타입 좌표계 변환 행렬을 전송받는 uniform 변수 선언
uniform mat4 model; // 모델 행렬
uniform mat4 view; // 뷰 행렬

void main() {
  // 오브젝트 공간 좌표에 모델 행렬 > 뷰 행렬 순으로 곱해서 좌표계를 변환시킴.
  // 일단 vertex shader 단계에서는 뷰 좌표계까지만 변환한 뒤, geometry shader 로 전송 
  // (투영행렬 변환 미적용한 이유는 geometry shader 단계에서 설명할 예정!)
  gl_Position = view * model * vec4(aPos, 1.0);

  /* Assimp 에 정의된 오브젝트공간 노멀 벡터를 뷰 공간 노멀벡터 단계까지 변환 */

  // 노멀벡터를 뷰 좌표계 단계까지 변환시키기 위한 노멀행렬 계산
  // 노멀행렬 관련 https://github.com/jooo0922/opengl-study/blob/main/Lighting/Basic_Lighting/MyShaders/basic_lighting.vs 참고
  mat3 normalMatrix = mat3(transpose(inverse(view * model)));

  // 오브젝트공간 노멀벡터를 뷰 좌표계까지 변환한 후, 
  // interface block 을 통해 geometry shader 로 전송
  vs_out.normal = normalize(vec3(vec4(normalMatrix * aNormal, 0.0)));
}