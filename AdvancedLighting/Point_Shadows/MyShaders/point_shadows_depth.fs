#version 330 core

// geometry shader 단계에서 전송받는 각 프래그먼트의 보간된 월드공간 좌표계 입력변수 선언
in vec4 FragPos;

/* omnidirectional shadow map 생성에 필요한 값들을 전송받는 uniform 변수들 선언 */

// 광원 위치
uniform vec3 lightPos;

// light space 로 변환할 때 사용할 원근 투영행렬의 far_plane 깊이값(== 광원으로부터의 거리값 or 해당 light space 절두체의 최대거리)
uniform float far_plane;

void main() {
  // 월드공간 기준 '각 프래그먼트 ~ 광원 사이의 거리' 계산
  float lightDistance = length(FragPos.xyz - lightPos);

  // '각 프래그먼트 ~ 광원 사이의 거리'를 [0, 1] 사이의 값으로 정규화
  lightDistance = lightDistance / far_plane;

  // [0, 1] 사이로 정규화된 거리값을 깊이 버퍼에 저장
  gl_FragDepth = lightDistance;
}

/*
  omnidirectional shadow map 에서는 왜 '월드공간 거리값'을 깊이값으로 사용하는가?


  지난 번 예제에서는 원근분할된 NDC 좌표의 z 값을
  깊이 버퍼에 자동으로 쓰이도록 하여 깊이값으로 활용했었지?

  그런데, 그 예제에서는
  directional shadow map 을 사용했기 때문에,
  '직교 투영행렬'로 light space 좌표계를 계산했었고,
  
  다행히 NDC 좌표의 z값(== 깊이값)이 linear 하게 계산됨.

  
  그렇지만,
  omnidirectional shadow map 을 사용할 때에는
  '원근 투영행렬'로 light space 좌표계를 계산하므로,
  NDC 좌표의 z값이 non-linear(정확히는 logarithmic)하게 계산될 수밖에 없음.

  따라서, 계산의 편의를 위해
  '원근 투영행렬'로 light space 좌표계를 계산할 때에도
  linear 한 깊이값을 사용하고 싶다면,

  '월드공간 기준으로 각 프래그먼트에서 광원까지의 거리'를 계산한 뒤,
  [0, 1] 사이로 정규화한 값을 사용하면 linear 한 깊이값으로 활용 가능!

  항상 '원근 투영행렬'을 적용하기 전 단계,
  그러니까 '클립좌표계' 직전까지의 좌표계는
  z값이 linear 하게 계산된다는 것을 명심할 것!
*/