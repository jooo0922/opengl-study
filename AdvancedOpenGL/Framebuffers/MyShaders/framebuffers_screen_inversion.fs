#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture; // off-screen framebuffer 에 attach 된 texture unit 위치값이 전달됨.

void main() {
  /* inversion post-processing 적용 */
  // framebuffer 에 attach 된 텍스쳐 객체를 샘플링한 텍셀값을 vec3(1.0) 에서 빼서 값을 뒤집음.
  FragColor = vec4(vec3(1.0 - texture2D(screenTexture, TexCoords)), 1.0);
}
