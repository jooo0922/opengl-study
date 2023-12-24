#version 330 core

// per vertex update 가 적용될 일반적인 vertex attribute 변수들
layout(location = 0) in vec2 aPos; // NDC 좌표계 기준으로 정의된 위치값
layout(location = 1) in vec3 aColor;

// per instance update 가 적용될 instanced array (로 사용할 vertex attribute)
layout(location = 2) in vec2 aOffset;

// 프래그먼트 쉐이더 단계로 보간하여 전송할 색상값 출력 변수
out vec3 fColor;

void main() {
  // vertex attribute 변수로 전송받은 색상값을 프래그먼트 쉐이더로 보간하여 전송
  fColor = aColor;

  // per vertex 단위로 업데이트되는 Quad 의 각 정점 좌표(aPos)에
  // per instance 단위로 업데이트되는 각 Quad 의 position offset(aOffset) 을 더해서
  // 최종적인 NDC 좌표계 기준 위치값 계산
  gl_Position = vec4(aPos + aOffset, 0.0, 1.0);
}
