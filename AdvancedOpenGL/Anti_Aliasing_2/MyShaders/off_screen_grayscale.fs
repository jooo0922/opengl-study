#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture; // off-screen framebuffer 에 attach 된 texture unit 위치값이 전달됨.

void main() {
    /* grayscale post-processing 적용 */
  FragColor = texture2D(screenTexture, TexCoords);

  // framebuffer 에 attach 된 텍스쳐 객체를 샘플링한 텍셀값의 r, g, b 채널의 평균값 계산
  // float average = (FragColor.r + FragColor.g + FragColor.b) / 3.0;

  // 사람의 눈은 실제로 초록색을 가장 민감하게 받아들이고, 파란색을 가장 덜 민감하게 받아들이므로,
  // 물리적으로 좀 더 정확한 grayscale 을 계산하려면 각 색상 채널마다 가중치를 다르게 적용하여 계산함!
  float average = FragColor.r * 0.2126 + FragColor.g * 0.7152 + FragColor.b * 0.0722;

  FragColor = vec4(vec3(average), 1.0);
}