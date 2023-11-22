#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main() {
  // 전송받은 텍스쳐 객체를 샘플링하여 최종 색상 계산
  FragColor = texture2D(texture1, TexCoords);
}
