#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aTexcoords;
layout(location = 3) in mat4 aInstanceMatrix; // 인스턴스 단위마다 업데이트될 모델 행렬 attribute 변수 (instanced array)

// 프래그먼트 쉐이더로 보간하여 전송할 uv 좌표 출력 변수
out vec2 TexCoords;

// glm::mat4 타입 좌표계 변환 행렬을 전송받는 uniform 변수 선언
uniform mat4 view; // 뷰 행렬
uniform mat4 projection; // 투영 행렬

void main() {
  TexCoords = aTexcoords;
  gl_Position = projection * view * aInstanceMatrix * vec4(aPos, 1.0); // 오브젝트 공간 좌표에 모델 행렬 > 뷰 행렬 > 투영 행렬 순으로 곱해서 좌표계를 변환시킴.
}