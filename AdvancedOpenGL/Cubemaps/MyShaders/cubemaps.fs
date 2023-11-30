#version 330 core

out vec4 FragColor;

// 버텍스 쉐이더로부터 보간되어 입력된 월드공간 위치 및 노멀벡터
in vec3 Normal;
in vec3 Position;

/* uniform 변수 선언 */
uniform vec3 cameraPos; // 뷰 벡터 계산을 위해 선언한 카메라 위치 변수
uniform samplerCube skybox; // 큐브맵 반사 구현을 위해 선언한 큐브맵 sampler 변수

void main() {
  /* 큐브맵 '반사' 재질 구현 */

  // 뷰 벡터 계산
  vec3 I = normalize(Position - cameraPos); 

  // 정규화된 뷰 벡터와 노멀벡터를 인자로 전달하여 반사벡터 계산 (reflect() 내장함수 사용)
  vec3 R = reflect(I, normalize(Normal)); 

  // 계산된 반사벡터를 방향벡터 삼아 큐브맵 샘플링 후, 최종 색상 계산 -> 큐브맵 반사(거울 재질) 구현!
  FragColor = vec4(texture(skybox, R).rgb, 1.0);
}