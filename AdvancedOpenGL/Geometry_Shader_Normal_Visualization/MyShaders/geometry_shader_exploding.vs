#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aTexcoords;

// geometry shader 단계로 출력할 interface block 선언
out VS_OUT {
  vec2 texCoords;
} vs_out;

// glm::mat4 타입 좌표계 변환 행렬을 전송받는 uniform 변수 선언
uniform mat4 model; // 모델 행렬
uniform mat4 view; // 뷰 행렬
uniform mat4 projection; // 투영 행렬

void main() {
  // geometry shader 단계로 전달할 uv 데이터 할당
  vs_out.texCoords = aTexcoords;

  // 오브젝트 공간 좌표에 모델 행렬 > 뷰 행렬 > 투영 행렬 순으로 곱해서 좌표계를 변환시킴.
  gl_Position = projection * view * model * vec4(aPos, 1.0);
}