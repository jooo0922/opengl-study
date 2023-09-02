#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoord; // uv 데이터를 전송받는 attribute 입력 변수 추가

out vec3 ourColor;
out vec2 texCoord; // 프래그먼트 쉐이더와 linking 시 보간해서 넘겨줄 uv 데이터 출력 변수 추가

void main() {
  gl_Position = vec4(aPos, 1.0);
  ourColor = aColor;
  texCoord = aTexCoord; // 출력 변수에 텍스쳐 uv 데이터 할당
}