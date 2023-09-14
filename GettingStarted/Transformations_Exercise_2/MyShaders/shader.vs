#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoord; // uv 데이터를 전송받는 attribute 입력 변수 추가

out vec3 ourColor;
out vec2 texCoord; // 프래그먼트 쉐이더와 linking 시 보간해서 넘겨줄 uv 데이터 출력 변수 추가

uniform mat4 transform; // glm 으로 만든 mat4 변환행렬을 전닳받는 uniform 변수

void main() {
  gl_Position = transform * vec4(aPos, 1.0); // gl_Position 에 할당하기 전에, 버텍스의 오브젝트 공간 좌표에 변환행렬을 곱해서 변환처리함.
  ourColor = aColor;
  texCoord = aTexCoord; // 출력 변수에 텍스쳐 uv 데이터 할당
}