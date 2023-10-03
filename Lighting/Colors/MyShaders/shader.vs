#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord; // uv 데이터를 전송받는 attribute 입력 변수 추가

out vec3 ourColor;
out vec2 texCoord; // 프래그먼트 쉐이더와 linking 시 보간해서 넘겨줄 uv 데이터 출력 변수 추가

// glm::mat4 타입 좌표계 변환 행렬을 전송받는 uniform 변수 선언
uniform mat4 model; // 모델 행렬
uniform mat4 view; // 뷰 행렬
uniform mat4 projection; // 투영 행렬

void main() {
  gl_Position = projection * view * model * vec4(aPos, 1.0); // 오브젝트 공간 좌표에 모델 행렬 > 뷰 행렬 > 투영 행렬 순으로 곱해서 좌표계를 변환시킴.
  texCoord = aTexCoord; // 출력 변수에 텍스쳐 uv 데이터 할당
}