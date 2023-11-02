#version 330 core

out vec4 FragColor;

// 버텍스 쉐이더로부터 보간되어 전달된 uv 좌표값
in vec2 TexCoords;

/* 각 텍스쳐 타입별 변수명 컨벤션에 따라 uniform sampler2D 선언 */
uniform sampler2D texture_diffuse1;

void main() {
  // diffuse 텍스쳐를 샘플링하여 최종 색상으로 출력
  FragColor = texture2D(texture_diffuse1, TexCoords);
}