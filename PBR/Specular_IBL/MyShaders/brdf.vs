#version 330 core

layout(location = 0) in vec3 aPos;

/* fragment shader 단계로 출력할 출력 변수 선언 */
out vec3 WorldPos;

/* 변환 행렬을 전송받는 uniform 변수 선언 */ 

// 투영 행렬
uniform mat4 projection;

// 뷰 행렬
uniform mat4 view;

void main() {
  // 프래그먼트 쉐이더 단계로 보간하여 출력할 world space 위치값 할당
  // -> HDR 이미지를 적용할 단위 큐브의 position attribute 는 이미 world space 기준으로 초기화되어 전송받음!
  WorldPos = aPos;

  // World Space 좌표에 뷰 행렬 > 투영 행렬 순으로 곱해서 좌표계를 변환시킴.
  gl_Position = projection * view * vec4(WorldPos, 1.0);
}
