#version 330 core

// 프래그먼트 쉐이더 출력 변수 선언
out vec4 FragColor;

// vertex shader 단계에서 전달받는 입력 변수 선언
in vec3 WorldPos;

// 큐브맵으로 변환된 HDR 이미지 텍스쳐 선언
uniform samplerCube environmentMap;

// PI 상수값 정의
const float PI = 3.14159265359;

void main() {
  /*
    정규화한 각 프래그먼트의 world space 방향벡터를
    surface point P 의 방향벡터 N (= 반구 영역의 방향벡터) 로 사용함.

    -> 나중에 pbrShader 에서 irradiance map 로부터 irradiance 값을 샘플링할 때,
    각 프래그먼트(surface point)에 대해 이 방향벡터를 사용할 것임!
  */
  vec3 N = normalize(WorldPos);

  /* surface point P 지점으로 들어오는 irradiance 총량을 diffuse term 적분식으로 계산 */

  // diffuse term 적분식을 이산적인(discrete) 리만 합(Riemann sum)으로 계산할 때, irradiance 결과값을 누산할 변수 초기화 (노션 IBL 관련 필기 참고)
  vec3 irradiance = vec3(0.0);

  /*
    surface point P 를 원점으로 하는 tangent space 기준으로 정의된
    tangentSample 방향벡터를 world space 로 변환하기 위해,
    
    현재 tangent space 의 기저 축(Tangent, Bitangent, Normal)을
    world space 로 변환하여 계산해 둠.

    right -> world space 기준 Tangent 기저 축
    up -> world space 기준 Bitangent 기저 축
    N -> world space 기준 Normal 기저 축

    이때, 이미 N 자체가 world space 기준으로 계산되어 있으므로,
    world space 업 벡터인 up 과 외적하여 나머지 기저 축 또한 
    world space 기준으로 계산할 수 있음!
  */
  vec3 up = vec3(0.0, 1.0, 0.0);
  vec3 right = normalize(cross(up, N));
  up = normalize(cross(N, right));

  // 반구 영역의 고도각(polar azimuth)과 방위각(zenith angle)을 이산적으로(discretely) 순회할 각도 간격 정의 (노션 IBL 관련 필기 참고)
  // -> 이 간격이 작을수록 더 정확한 적분(리만 합(Riemann sum))을 계산할 수 있음. 즉, 더 정확한 irradiance 계산 가능
  float sampleDelta = 0.025;

  // LearnOpenGL 본문의 반구 영역의 고도각과 방위각에 대한 이중 시그마 식의 전체 항 개수를 누산해나갈 변수 초기화 -> 즉, 시그마 식의 n1n2 에 해당
  float nrSamples = 0.0;

  // 반구 영역의 방위각(zenith angle) 을 sampleDelta 간격으로 2PI(360도)까지 순회
  for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {

    // 반구 영역의 고도각(polar azimuth) 을 sampleDelta 간격으로 PI / 2(90도)까지 순회
    for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {
      // 현재 순회 중인 구면좌표계 -> surface point P 지점을 원점으로 하는 tangent space 기준의 방향벡터로 변환 (자세한 설명 하단 참고)
      vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

      // tangent space 기준 샘플링 방향벡터를 world space 로 변환 (자세한 설명 하단 참고)
      vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

      // LearnOpenGL 본문에 정리된 이중시그마의 각 항을 계산하여 irradiance 변수에 누산 -> 즉, Li(p, phi, theta) * cos(theta) * sin(theta) 를 계산! 
      // 이때, 고도각(theta)이 높은 영역의 contribution 을 보정하기 위해 sin(theta) 만큼 가중치를 곱해줌(노션 IBL 관련 필기 참고)
      irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);

      // 이중시그마 외부로 추출된 항의 분모인 n1n2 를 누산함. -> 이중시그마 식의 전체 항 개수
      nrSamples++;
    }
  }

  // 이중시그마 외부로 추출된 항(kD * c * PI / n1n2)의 일부분을 이중시그마 수열의 합(irradiance) 에 곱해줌 (노션 IBL 관련 필기 참고)
  irradiance = PI * irradiance * (1.0 / float(nrSamples));

  // 계산된 surface point P 지점의 diffuse term 의 irradiance 를 큐브맵 버퍼에 저장
  FragColor = vec4(irradiance, 1.0);
}

/*
  현재 순회 중인 방위각(phi)과 고도각(theta) 기준의 구면좌표계를
  surface point P 지점을 원점으로 하는 tangent space 기준의 3D 데카르트 방향벡터로 변환

  -> 이럴거면 왜 기존 적분식을 방위각(phi)과 고도각(theta)에 대한 이중 적분식으로 바꾼거야?

  방위각(phi)과 고도각(theta)을 기준으로 이중 적분식을 계산하면, 
  무수히 많은 입체각 wi 를 좀 더 이산적으로(discretely) 계산할 수 있다는 장점이 있음.

  그러나, 리만 합을 계산하는 과정에서 우리가 샘플링해야 할 environmentMap 은
  여전히 큐브맵 형태이므로, 별 수 없이 3차원 방향벡터가 있어야 샘플링이 가능함.

  참고로, 현재 구면좌표계는 surface point P 지점을 중심으로 한
  구체에 대한 구면좌표계이므로, 이를 3차원 데카르트 좌표계로 변환하면,
  당연히 surface point P 를 원점으로 하는 tangent space 기준의 데카르트 좌표계가 계산될 것임.

  또한, 위 코드에서 사용된 변환식은
  equirectangular_to_cubemap.fs 에서 사용된
  pbrt 4판의 변환 공식과는 다름.

  위 변환식은 아래 링크에서 확인 가능!
  https://ko.wikipedia.org/wiki/구면좌표계 
*/

/*
  tangent space 기준 방향벡터에 right, up, N 벡터를 곱하는 이유


  상단 주석에도 정리가 되어있듯이,

  right, up, N 벡터는 각각
  surface point P 에 대한 world space 기준
  Tangent, Bitangent, Normal 기저 축이라고 했었지?

  근데 이 기저벡터들을
  tangent space 공간 기준으로 정의된 방향벡터의 각 컴포넌트에 곱해준다?

  결국, tangent space 공간 기준으로 정의된 좌표값에
  TBN 행렬을 곱해서 world space 공간 기준으로 변환하는 것과 동일한 행위임.

  왜냐하면, TBN 행렬도 결국
  world space 로 변환된 Tangent, Bitangent, Normal
  기저 축을 열 벡터로 꽂아넣은 행렬이잖아!
*/
