#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main() {
  // 전송받은 텍스쳐 객체를 샘플링하여 최종 색상 계산
  // FragColor = texture2D(texture1, TexCoords);

  // gl_FragCoord 에 들어있는 현재 프래그먼트의 깊이값을 최종 색상으로 출력하여 depth buffer 시각화
  FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
}