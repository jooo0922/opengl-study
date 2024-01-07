#version 330 core

out vec4 FragColor;

// vertex shader 단계에서 전달받는 입력 interface block 선언
in VS_OUT {
  vec3 FragPos;
  vec3 Normal;
  vec2 TexCoords;
} fs_in;

// OpenGL 에서 전송해 줄 uniform 변수들 선언
uniform sampler2D floorTexture; // 바닥 평면 텍스쳐 (0번 texture unit 에 바인딩된 텍스쳐 객체 샘플링)
uniform vec3 lightPos; // 광원 위치 > 조명벡터 계산에서 사용
uniform vec3 viewPos; // 카메라 위치 > 뷰 벡터 계산에서 사용
uniform bool blinn; // blinn-phong 반사 모델 적용 모드 상태값

void main() {
  vec3 color = texture2D(floorTexture, fs_in.TexCoords).rgb;

  // 초록색을 최종 색상으로 출력
  FragColor = vec4(color, 1.0);
}