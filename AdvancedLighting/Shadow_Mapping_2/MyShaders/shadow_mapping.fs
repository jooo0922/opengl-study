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
  // float shadow = currentDepth > closestDepth ? 1.0 : 0.0;

  // shadow acne 현상 해결을 위해, 조명벡터와 프래그먼트의 방향벡터(노멀벡터) 각도를 내적하여 shadow bias 계산 (하단 필기 참고)
  vec3 normal = normalize(fs_in.Normal);
  vec3 lightDir = normalize(lightPos - fs_in.FragPos);

  // bias 계산 시, [0.005, 0.05] 범위 내의 값으로 계산되도록 clamping 내적
  float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

  // shadow testing 시, 현재 프래그먼트의 깊이값에 bias 값만큼 빼서 깊이값이 광원에 더 가까워지도록 보정
  float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

  // 현재 프래그먼트의 깊이값(projCoords.z)이 NDC 좌표계 상의 far plane 의 깊이값(1.0)을 넘어서면,
  // 무조건 그림자 영역이 아닌 것으로 판정하도록 함으로써, Over sampling 이슈 해결 (관련 필기 하단 참고)
  if(projCoords.z > 1.0) {
    shadow = 0.0;
  }

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

/*
  shadow bias


  shadow bias 란, 
  shadow acne 라고 불리는 artifact 를 해결하기 위해
  계산하는 보정값이라고 보면 됨.


  shadow map 은 일정한 해상도가 존재하는 텍스쳐에 불과하기 때문에,
  모든 프래그먼트의 좌표값(projCoords.xy)에 1:1 로 맵핑되는 texel 들이 존재하는 게 아님.

  즉, 여러 개의 프래그먼트 좌표값이
  shadow map 으로부터 동일한 texel 을 샘플링하게 될 확률이 높다는 뜻이지!


  만약에 이렇다면 어떤 상황이 발생하게 될까?
  서로 다른 프래그먼트들이 shadow map 으로부터 동일한 깊이값(closestDepth)을 샘플링하여,
  자신의 깊이값(currentDepth)과 비교하게 될 것임.

  이것은 일반적인 상황에서는 큰 문제가 되지는 않겠지만,
  만약 조명벡터와 각 프래그먼트들의 표면 사이가 '기울어져' 있다면,
  LearnOpenGL 본문에 삽입된 일러스트처럼,

  샘플링하려는 shadow map 의 texel 이 각 프래그먼트의 표면과
  지그재그로 기울어진 상태로 샘플링되는 꼴이 되어버림!
  (혹은, 각 프래그먼트의 표면을 기울여진다고 상상해보면, 
  그것도 마찬가지로 shadow map 의 texel 과 지그재그로 교차되는 꼴이 될 것임!)


  이렇게 되면, 해당 일러스트에서 보는 것과 같이,
  각 프래그먼트의 깊이값이 다르더라도, 
  해상도의 한계로 인해 shadow map 으로부터 동일한 깊이값을 샘플링해와서
  비교할 수 밖에 없다보니, 
  
  서로 가까이에 있는 프래그먼트들이라 하더라도,
  어떤 부분은 그림자 영역 내에 있는 것으로 판정되고(일러스트의 검은색 부분),
  어떤 부분은 그림자 영역 밖에 있는 것으로 판정되는(일러스트의 노란색 부분) 것임


  이로 인해 약간의 Moiré 같은 패턴이 생기게 되는데,
  이를 'shadow acne' 라고 함.


  이를 해결하기 위한 방법은 아주 간단한데,
  각 프래그먼트들의 깊이값(currentDepth)들을 전반적으로
  광원의 위치에 더 가깝게 당겨줌으로써, 

  동일한 shadow map 의 깊이값(closestDepth)을 지그재그 형태로 샘플링하더라도,

  두 깊이값 사이의 차이가 확연하게 나지 않을 정도면,
  항상 현재 프래그먼트의 깊이값이(currentDepth) 더 광원에 가까운 것으로 판정하는,
  즉, 그림자 영역 밖에 있는 것으로 판정되도록 깊이값을 보정하는 것이지.


  이럴 떄 사용하는 보정값이 
  'shadow bias' 라고 보면 됨.


  그런데, shadow bias 값을 계산할 때에는
  몇 가지 주의할 사항들이 존재하는데,

  첫째로, 각 프래그먼트 표면과 조명벡터 사이의 각도가 
  전부 다르기 때문에, 각 프래그먼트와 shadow map 의 texel 이
  어느 정도로 지그재그 되는지 그 각도마저도 모두 다름.

  따라서, 그 각도에 대한 내적값을 이용해서,
  각도에 따라 shadow bias 값을 프래그먼트마다 다르게 적용할 수 있도로 함.


  둘째로, shadow bias 값이 너무 커지면,
  확연히 그림자 영역 내에 존재해야 할 프래그먼트들 조차
  광원에 더 가까워지다보니,

  그림자 영역 밖에 있는 것으로 판정되는 이슈가 발생함.

  이로 인해, 마치 그림자가 '떼어진 것처럼 보이는' 현상이 발생하는데
  이를 Peter panning 이라고 함.

  이 현상이 발생하지 않으려면,
  shadow bias 를 계산할 때, 
  일정 범위 내에서 clamping 되도록 범위를 지정해줘야 함. 
*/

/*
  Over sampling


  Over sampling 에 대한 기본적인 설명은
  shadow_mapping_2.cpp 에 정리해놨으니 참고하면 됨.


  그러나, Wrapping mode 를 변경하는 것만으로
  모든 Over sampling 이슈를 해결하기는 어려운 경우가 존재하는데,
  
  그건 바로 현재 프래그먼트의 깊이값(projCoords.z)이
  NDC 좌표계 기준 far plane 의 깊이값(1.0)을 넘어서는 경우임.


  projCoords.xy 값이 [0, 1] 범위를 넘어섰다는 사실은
  NDC 좌표계 상의 Frustum 에서 x 축 및 y 축과 교차하는 면들,
  즉, left, top, bottom, right 을 넘어서는 지점을 샘플링하려고 한다는
  사실은 알 수 있으나,

  near, far 처럼 z 축과 교차하는 면들을
  넘어섰는지 여부는 판단할 수 없음.


  따라서, far plane 에 대한 Over sampling 여부를 판단하려면, 
  현재 프래그먼트의 깊이값(projCoords.z) 과 
  NDC 좌표 기준 far plane 의 깊이값(1.0) 을 비교하면 되겠지.

  이 비교 결과를 통해, 
  far plane 에 대해 Over sampling 되었음이 판정될 경우,
  그림자 영역 안에 없는 현재 프래그먼트들이 
  억울하게 그림자 영역 안에 있는 것으로 판정되지 않도록,
  무조건 그림자 영역 밖에 있다고 판정해버릴 수 있겠지!
*/