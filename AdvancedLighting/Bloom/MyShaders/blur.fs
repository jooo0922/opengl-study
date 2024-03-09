#version 330 core

out vec4 FragColor;

// 버텍스 쉐이더에서 전송받은 텍스쳐 좌표 입력변수 선언
in vec2 TexCoords;

/* uniform 변수 선언 */

// image 텍스쳐 (blur 처리를 적용할 텍스쳐 객체(= color attachment) 색상 버퍼)
uniform sampler2D image;

// blur 처리 방향 변수
uniform bool horizontal;

// 각 blur 방향으로 샘플링된 texel 에 적용할 가중치 kernel (= convolution matrix)
uniform float weight[5] = float[](0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main() {
  // 각 blur 방향의 주변 texel 좌표 계산을 위해 더해줄 offset 값 -> 단일 texel 사이즈에 해당
  vec2 tex_offset = 1.0 / textureSize(image, 0);

  // 현재 texel 을 중심으로 가중치를 적용하여 누산할 변수 result 선언 및 초기화
  // blur 를 적용할 현재 texel (= kernel 의 중점) 에 첫 번째 가중치를 곱하여 누산값 초기화 
  vec3 result = texture(image, TexCoords).rgb * weight[0];

  // 현재 texel 을 중심으로, blur 처리 방향(= horizontal)을 따라 5개의 주변 texel 을 샘플링하여 가중치 적용 및 누산
  if(horizontal) {
    // 수평 방향 blur 처리
    for(int i = 0; i < 5; i++) {
      // 현재 texel 의 오른쪽 방향 주변 texel 들에 대해 가중치 적용 및 누산
      result += texture(image, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];

      // 현재 texel 의 왼쪽 방향 주변 texel 들에 대해 가중치 적용 및 누산
      result += texture(image, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
    }
  } else {
    // 수직 방향 blur 처리
    for(int i = 0; i < 5; i++) {
      // 현재 texel 의 위쪽 방향 주변 texel 들에 대해 가중치 적용 및 누산
      result += texture(image, TexCoords + vec2(tex_offset.y * i, 0.0)).rgb * weight[i];

      // 현재 texel 의 아래쪽 방향 주변 texel 들에 대해 가중치 적용 및 누산
      result += texture(image, TexCoords - vec2(tex_offset.y * i, 0.0)).rgb * weight[i];
    }
  }

  // 누산된 최종 색상값을 출력 변수에 저장
  FragColor = vec4(result, 1.0);
}
