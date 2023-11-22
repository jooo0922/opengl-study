#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture; // off-screen framebuffer 에 attach 된 texture unit 위치값이 전달됨.

void main() {
  // framebuffer 에 attach 된 텍스쳐 객체를 샘플링하여 최종 색상 출력
  vec3 col = texture2D(screenTexture, TexCoords).rgb;
  FragColor = vec4(col, 1.0);
}
