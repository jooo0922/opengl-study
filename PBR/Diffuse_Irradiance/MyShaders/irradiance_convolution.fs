#version 330 core

// 프래그먼트 쉐이더 출력 변수 선언
out vec4 FragColor;

// vertex shader 단계에서 전달받는 입력 변수 선언
in vec3 WorldPos;

// 큐브맵으로 변환된 HDR 이미지 텍스쳐 선언
uniform sampler2D equirectangularMap;

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

  // diffuse term 적분식을 이산적인(discrete) 리만 합(Riemann sum)으로 계산할 때, irradiance 결과값을 누산할 변수 초기화 (노션 IBL 관련 필기 정리 참고)
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
}
