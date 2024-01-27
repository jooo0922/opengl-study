#version 330 core

out vec4 FragColor;

// vertex shader 단계에서 전달받는 입력 interface block 선언
in VS_OUT {
  vec3 FragPos;
  vec3 Normal;
  vec2 TexCoords;
  vec4 FragPosLightSpace;
} fs_in;

// OpenGL 에서 전송해 줄 uniform 변수들 선언
uniform sampler2D diffuseTexture; // 바닥 평면 텍스쳐 (0번 texture unit 에 바인딩된 텍스쳐 객체 샘플링)
uniform sampler2D shadowMap; // shadow map 텍스쳐 (1번 texture unit 에 바인딩된 텍스쳐 객체 샘플링)
uniform vec3 lightPos; // 광원 위치 > 조명벡터 계산에서 사용
uniform vec3 viewPos; // 카메라 위치 > 뷰 벡터 계산에서 사용

// 현재 프래그먼트가 그림자 안에 있는지 여부를 반환해주는 함수
float ShadowCalculation(vec4 fragPosLightSpace) {
  // 현재 프래그먼트 위치값을 light space 좌표계(== projection 행렬까지 적용된 clip space 라고 봐도 무방) > NDC 좌표계로 변환 (자세한 내용은 하단 참고)
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

  // [-1, 1] 사이의 NDC 좌표계 > [0, 1] 사이의 범위로 맵핑 (-> uv 좌표로 쓰기 위해...) (자세한 내용은 하단 참고)
  projCoords = projCoords * 0.5 + 0.5;

  // [0, 1] 사이로 맵핑된 좌표의 x, y 값을 uv 좌표로 사용해서 shadow map 샘플링
  float closestDepth = texture2D(shadowMap, projCoords.xy).r;

  // [0, 1] 사이로 맵핑된 좌표의 z 값을 현재 프래그먼트의 깊이값으로 할당
  float currentDepth = projCoords.z;

  // shadow map 에서 샘플링한 깊이값(closestDepth)과 현재 프래그먼트의 깊이값(currentDepth)을 비교하여,
  // 현재 프래그먼트가 그림자 영역 내에 존재하는지 (== occluded 되는지) 판단
  float shadow = currentDepth > closestDepth ? 1.0 : 0.0;

  return shadow;
}

void main() {
  // 바닥 평면 텍스쳐를 샘플링하여 diffuse color 값 저장
  vec3 color = texture2D(diffuseTexture, fs_in.TexCoords).rgb;

  // 프래그먼트에 수직인 노멀벡터 정규화
  vec3 normal = normalize(fs_in.Normal);

  // 조명색상을 어두운 회색으로 초기화
  vec3 lightColor = vec3(0.3);

  /* blinn-phong 반사 모델 기준으로 조명 계산 */

  /* ambient 성분 계산 */
  float ambientStrength = 0.3; // ambient 강도
  vec3 ambient = ambientStrength * lightColor; // ambient 강도에 조명 색상 곱해서 최종 ambient 성분값 계산

  /* diffuse 성분 계산 */
  // 조명계산에 사용되는 모든 방향벡터들은 항상 정규화를 해줄 것! -> 그래야 내적계산 시 정확한 cos 값만 얻을 수 있음!
  vec3 lightDir = normalize(lightPos - fs_in.FragPos); // 조명벡터 (프래그먼트 위치 ~ 광원 위치)
  float diff = max(dot(lightDir, normal), 0.0); // 노멀벡터와 조명벡터 내적 > diffuse 성분의 세기(조도) 계산 (참고로, 음수인 diffuse 값은 조명값 계산을 부정확하게 만들기 때문에, 0.0 으로 clamping 시킴)
  vec3 diffuse = diff * lightColor; // diffuse 조도에 조명 색상 곱해서 최종 diffuse 성분값 계산

  /* specular 성분 계산 */
  vec3 viewDir = normalize(viewPos - fs_in.FragPos); // 뷰 벡터 (카메라 위치 ~ 프래그먼트 위치)
  float spec = 0.0; // specular 조도 선언 및 초기화

  // 뷰 벡터와 조명벡터 사이의 halfway 벡터 계산 (두 벡터의 합 -> 두 벡터 사이를 가로지르는 하프 벡터 (<셰이더 코딩 입문> p.222 참고))
  vec3 halfwayDir = normalize(lightDir + viewDir);

  // specular 성분의 조도 계산
  // 프래그먼트 지점의 normal 벡터와 halfway 벡터를 clamping 내적함
  // 내적값을 32제곱 > 32는 shininess 값으로써, 값이 클수록 highlight 영역이 정반사되고, 값이 작을수록 난반사됨.
  // Blinn-Phong 모델에서는 동일한 조건 하에 기본 Phong 모델에 비해 내적값이 더 작게 계산되기 때문에,
  // 일반적인 관례상 기본 Phong 모델보다 2~4배 큰 shininess(광택값)을 사용한다.
  spec = pow(max(dot(normal, halfwayDir), 0.0), 32);

  vec3 specular = spec * lightColor; // specular 조도에 조명 색상을 곱해 specular 성분값 계산

  // 현재 프래그먼트의 light space 좌표계를 매개변수로 전달하여 그림자 영역 내에 존재하는지 여부를 판단
  float shadow = ShadowCalculation(fs_in.FragPosLightSpace);

  // 3가지 성분을 모두 더한 뒤, 바닥 평면 텍스쳐 색상값(diffuse color)를 곱하여 최종 색상 계산
  /*
    조명 성분 중, diffuse 및 specular 성분에만
    shadow 값(그림자 영역 내에 얼만큼 속하는지)을 적용함

    why? 그림자 영역도 마찬가지로
    환경광(ambient)의 영향을 받기 때문에,
    완전히 검정색인 그림자는 존재하지 않음.

    따라서, ambient 성분값은 그대로 보존하고,
    diffuse 과 specular 성분에만 shadow 값을 곱해준 것!
  */
  vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

  FragColor = vec4(lighting, 1.0);
}

/*
  ShadowCalculation() 함수 내에서의 좌표계 변환
  

  1. light space 좌표계 > NDC 좌표계 변환

  shadow map 의 깊이 버퍼를 기록할 때 사용하는
  shadow_mapping_depth.vs 버텍스 쉐이더 내에서는

  gl_Position 변수에 light space 좌표값(== clip space)을 할당하는 순간,
  OpenGL 내부에서 곧바로 원근분할을 수행한 뒤에 깊이 버퍼에 z값을 저장함.


  그러나, 실제 씬을 렌더링하는
  shadow_mapping.vs, shadow_mapping.fs 함수 내에서는
  light space 좌표계에 대한 원근분할을 수행하지 않으므로,
  쉐이더 코드 내에서 직접 원근분할 작업을 구현해줘야 함.

  이렇게 원근분할을 수행함으로써,
  light space 좌표계를 NDC 좌표계로 변환해준 것.


  2. [-1, 1] 사이의 NDC 좌표계 > [0, 1] 사이의 범위로 맵핑

  위와 같은 좌표계 범위 맵핑을 해주는 이유는 크게 두 가지인데,


  첫 번째는, 일단 projCoords 의 x, y 값을
  shadow map 을 샘플링하는 uv 좌표계로 사용하고 싶은 것임.

  그렇게 하려면, uv 좌표는 [0, 1] 사이의 범위까지만
  유효한 텍스쳐 범위로 가정하고, 그 범위를 넘어서면
  texture wrapping 모드가 적용되는 것으로 간주하므로,

  uv 좌표계로 사용하려면 [0, 1] 사이의 범위로 맵핑하는 게 좋겠지.


  두 번째는, projCoords 의 z 값을
  shadow map 에서 샘플링한 깊이값과 비교하고 싶은 것임.
  
  그런데, shadowm map 은 grey scale(흑백) 텍스쳐이고,
  이는 shadow map 안에 기록되어 있는 텍셀값이 [0, 1] 사이의 범위로
  한정되어 있다는 의미가 됨.

  따라서, [0, 1] 사이의 범위로 기록되어 있는
  shadow map 내의 깊이값과 비교하려면

  당연히 projCoords 의 z 값 또한
  [0, 1] 사이의 범위로 맞춰주는 게 타당하겠지!
*/