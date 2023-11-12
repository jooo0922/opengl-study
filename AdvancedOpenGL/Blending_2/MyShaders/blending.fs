#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main() {
  // 샘플링한 텍스쳐의 texel 을 최종 색상으로 출력
  FragColor = texture2D(texture1, TexCoords);
}