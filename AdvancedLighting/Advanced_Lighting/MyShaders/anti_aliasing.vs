#version 330 core

layout(location = 0) in vec3 aPos;

/* 변환 행렬을 전송받는 uniform 변수 선언 */ 

// 모델 행렬
uniform mat4 model;

// 뷰 행렬
uniform mat4 view;

// 투영 행렬
uniform mat4 projection;

void main() {
  // 오브젝트 공간 좌표에 모델 행렬 > 뷰 행렬 > 투영 행렬 순으로 곱해서 좌표계를 변환시킴.
  gl_Position = projection * view * model * vec4(aPos, 1.0);
}
