#version 330 core

// 프래그먼트 쉐이더 출력 변수 선언
out vec4 FragColor;

// vertex shader 단계에서 전달받는 입력 변수 선언
in vec3 WorldPos;

// 큐브맵으로 변환된 HDR 이미지 텍스쳐 선언
uniform samplerCube environmentMap;

// mip level 에 따라 5단계로 나누어져 전송될 roughness 값
uniform float roughness;

// PI 상수값 정의
const float PI = 3.14159265359;

// specular lobe 영역 내에서 하프 벡터 H 를 뽑을 pdf(확률 밀도 함수)를 계산 시 사용할 NDF 함수
float DistributionGGX(vec3 N, vec3 H, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float NdotH = max(dot(N, H), 0.0);
  float NdotH2 = NdotH * NdotH;

  float nom = a2;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = PI * denom * denom;

  return nom / denom;
}

/*
  Hammersley 알고리즘에서 두 번째 이후 차원부터의 좌표값 계산에 사용되는
  Van Der Corput 시퀀스 알고리즘을 구현한 함수 (노션 IBL 필기 참고)

  -> 효율적인 연산을 위해 비트 연산자를 사용했다고 함.
  http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html 참고!
*/
float RadicalInverse_VdC(uint bits) {
  bits = (bits << 16u) | (bits >> 16u);
  bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
  bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
  bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
  bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
  return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

/*
  Quasi-Monte Carlo 적분에서 사용하는
  low-discrepancy sequence(저불일치 시퀀스)를 생성하는 알고리즘으로써 Hammersley 알고리즘 사용

  (노션 IBL 필기 참고)
*/
vec2 Hammersley(uint i, uint N) {
  return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

/*
  반구 영역 내에 존재하는 모든 sample vector 중에서
  specular lobe 영역 내에 들어오는 반사 벡터들만 선별적으로 골라 sample vector 를 생성하고,
  그것들만 가지고서 기댓값을 계산하는 Importance Sampling 함수 구현

  (노션 IBL 필기 참고)
*/
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
  // 시각적으로 더 나은 결과물을 반영하기 위해 Epic Games 엔진에서 사용 중인 Squared roughness 값을 사용함
  float a = roughness * roughness;

  // remapping 된 roughness 값 a 와 균일한 랜덤 분포로부터 생성된 random sample 벡터인 Xi 를 가지고서 구면좌표계 phi, theta 계산
  /*
    참고로, roughness 값에 따른 speucular lobe 영역 내에 존재하는 sample vector 의 
    구면좌표계를 계산하기 위해, BRDF 함수에서 NDF 항의 계산 공식을 일부 차용했다고 함.
    
    -> 변환 공식의 원리 자체가 NDF 함수에 기반하므로, NDF 함수를 깊이 팔 게 아닌 이상
    굳이 자세히 이해하려고 들 필요는 없을 듯...

    대신, sinTheta 구하는 공식은 간단함.

    cos^2 + sin^2 = 1 이라는 삼각함수 기본 성질로부터 유도하여 (게임수학 p.118 참고)

    sin^2 = 1 - cos^2,
    sqrt(sin^2) = sqrt(1 - cos^2),
    sin = sqrt(1 - cos^2) 로 도출된 공식을 코드화한 것임!
  */
  float phi = 2.0 * PI * Xi.x;
  float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
  float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

  /*
    Xi 및 roughness 로부터 계산된 구면좌표계 -> surface point P 지점을 원점으로 하는 tangent space 기준의 방향벡터로 변환 
    irradiance_convolution.fs 에서 동일한 방식의 변환을 사용하고 있음. 해당 코드 참고할 것!
  */
  vec3 H;
  H.x = cos(phi) * sinTheta;
  H.y = sin(phi) * sinTheta;
  H.z = cosTheta;

  /*
    tangent space 기준으로 정의된
    specular lobe 영역 내의 하프벡터 H 를 world space 로 변환하기 위해,
    
    현재 tangent space 의 기저 축(Tangent, Bitangent, Normal)을
    world space 로 변환하여 계산해 둠.

    이때, 이미 N 자체가 world space 기준으로 계산되어 있으므로,
    world space 업 벡터인 up 과 외적하여 나머지 기저 축 또한 
    world space 기준으로 계산할 수 있음!
  */
  vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
  vec3 tangent = normalize(cross(up, N));
  vec3 bitangent = cross(N, tangent);

  // tangent space 기준 하프벡터 H 를 world space 로 변환 (자세한 설명 하단 참고)
  vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;

  return normalize(sampleVec);
}

void main() {
  /*
    정규화한 각 프래그먼트의 world space 방향벡터를
    surface point P 의 방향벡터 N (= 반구 영역의 방향벡터) 로 사용함.

    -> 나중에 pbrShader 에서 pre-filtered env map 로부터  
    split sum approximation 의 첫 번째 적분식의 결과값을 샘플링할 때,
    각 프래그먼트(surface point)에 대해 이 방향벡터를 사용할 것임!
  */
  vec3 N = normalize(WorldPos);

  /*
    카메라 view vector 를 현재 surface point P 의 방향벡터 N 과 같다고 가정함.

    -> 즉, 카메라가 surface point P 바로 위에서 수직 방향으로 바라보고 있다고 가정하고,
    specular lobe 영역 내에 존재하는 Half vector 를 기준으로 이 view vector 의 
    반사벡터를 역추적한 입사광 벡터 L 을 계산함.
    
    TODO : 이 부분은 확실하지 않으므로, 나중에 다시 문서를 읽어보거나 구글링해서 정확한 내용을 찾아볼 것.
  */
  vec3 V = N;

  /* surface point P 지점에서 specular lobe 영역으로 반사되는 빛들의 총합을 Monte Carlo 적분으로 계산 */

  // Monte Carlo 적분의 샘플링 개수를 uint 상수값으로 정의
  const uint SAMPLE_COUNT = 1024u;

  // split sum approximation 의 첫 번째 적분식을 계산할 때, 결과값을 누산할 변수 초기화 (노션 IBL 관련 필기 참고)
  vec3 prefilteredColor = vec3(0.0);

  /*
    specular lobe 영역 내의 반사된 빛의 총합을 Monte Carlo 적분으로 계산할 때,
    기댓값 E 를 계산하는 시그마 계산식에서 샘플링 개수 N 에 대하여 평균을 구하는 
    1/N 에 해당하는 역할을 수행할 가중치 값 초기화 (노션 IBL 관련 필기 참고)
  */
  float totalWeight = 0.0;

  // Monte Carlo 적분의 샘플링 개수만큼 for-loop 를 순회하며 기댓값 E 에 대한 시그마 식을 이산적(discretely)으로 계산
  for(uint i = 0u; i < SAMPLE_COUNT; i++) {

    // low-discrepancy sequence 를 생성하는 알고리즘 중 하나인 Hammersley sequence 함수를 사용하여 uniformly random 한 sample 추출 (노션 IBL 관련 필기 참고)
    // 이처럼 저불일치 시퀀스 분포로부터 샘플링하여 푸는 MC 적분을 Quasi-Monte Carlo 라고 부름!
    vec2 Xi = Hammersley(i, SAMPLE_COUNT);

    // low-discrepancy sequence 로부터 얻은 랜덤한 vec2 값으로 표면의 roughness 값에 따라 정의되는 specular lobe 범위 내에 존재하는 하프벡터 H 계산 
    vec3 H = ImportanceSampleGGX(Xi, N, roughness);

    // 현재 surface point P 에 대한 하프벡터 H (specular lobe 범위 내에 존재) 를 기준으로, 카메라 view vector V 에 대한 반사벡터 (즉, 반사벡터의 반사벡터) 역추적
    // 이 공식은 자세히 들여다보면, 결국 반사벡터를 계산하는 GLSL 내장함수 reflect() 함수의 내부 구현부와 동일함을 알 수 있음!
    vec3 L = normalize(2.0 * dot(V, H) * H - V);

    // 입사광 벡터와 surface point P 의 노멀벡터 각도에 따른 [0.0, 1.0] 사이로 clamping 시킨 반사벡터의 세기(= 가중치) 계산
    float NdotL = max(dot(N, L), 0.0);

    /*
      가중치가 0 으로 곱해지면 어차피 적분의 누산값에 반영할 수 없으므로,
      가중치가 0 보다 큰 경우에만 한하여 누산값 prefilteredColor 에 반영함.
    */
    if(NdotL > 0.0) {
      /* Bright dots artifact 를 해결하기 위해 원본 HDR 큐브맵을 fetch 해올 mip level 계산 */

      /*
        specular lobe 영역 내의 전체 sample vector 중에서 
        하프 벡터 H 부근에 존재하는 sample vector 를 뽑을 확률을 계산하는 확률 밀도 함수(pdf) 계산
        (노션 IBL 필기 참고)
      */
      float D = DistributionGGX(N, H, roughness);
      float NdotH = max(dot(N, H), 0.0);
      float HdotV = max(dot(H, V), 0.0);
      float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;

      // 구체의 전체 표면적 상에서 원본 HDR 큐브맵의 각 단위 texel 들의 입체각(= 구체 상의 표면적) 계산 (노션 IBL 필기 참고)
      float resolution = 512.0; // 원본 HDR 큐브맵의 각 면의 해상도
      float saTexel = 4.0 * PI / (6.0 * resolution * resolution);

      // 구체의 전체 표면적 상에서 현재 하프벡터 H 부근 sample vector 의 입체각(= 구체 상의 표면적) 계산 (노션 IBL 필기 참고)
      float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

      /*
        (saSample / saTexel) 비율값을 mip level 삼아서
        원본 HDR 큐브맵으로부터 어느 정도의 해상도를 갖는 mipmap 을 샘플링할 것인지 결정
        (노션 IBL 필기 참고)
      */
      float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

      /*
        원본 HDR 큐브맵으로부터 샘플링해온 값,
        즉, 역추적한 입사광 벡터(= 반사벡터의 반사벡터)를 사용하여 fetch 한 texel 을 누산함.

        이는 split sum approximation 의 첫 번째 적분식에서 Li(p, wi) 에 해당하며,
        거기에 가중치 NdotL 을 곱하는 것은 미소량 dwi 로 각 샘플의 가중치를 조절해주는 것에 해당.
        (노션 IBL 관련 필기 참고)

        -> 이것은 specular lobe 영역 내에 존재하는 각각의 반사되는 빛의 총량을 합산하는 행위에 해당!
      */
      prefilteredColor += textureLod(environmentMap, L, mipLevel).rgb * NdotL;

      /*
        Monte Carlo 적분의 기댓값 E 를 계산할 때,
        시그마 합의 평균을 구하기 위해 곱해주는 1 / N 값 계산.

        사실, 1 / SAMPLE_COUNT 로 곱해주면 되는데,
        왜 각 샘플의 가중치인 NdotL 을 누산해서 1 / N 값을 계산하는 걸까?

        그 이유는, specular lobe 영역 내의 각 반사되는 빛의 가중치인 NdotL 이
        각 샘플의 확률 밀도 함수 pdf(x) 의 역할을 하기 때문임.

        노션 필기에서 보면 우리는 결국
        E[f(x) / pdf(x)] 에 대한 기댓값을 계산하는 것이기 때문에,
        원래대로 라면, ( texture(environmentMap, L).rgb * NdotL ) / NdotL 값을 누산해줘야 함.
        
        하지만 위 식을 보면 알겠지만, NdotL 끼리 서로 상쇄되어 버리기 때문에,
        pdf(x) 역할을 해야 할 NdotL 을 기댓값 E 를 계산할 때 곱해주는 1 / N 을 계산하는 과정에 반영해버려서
        나중에 최종 시그마 합 prefilteredColor 에 한꺼번에 적용하려는 것임.

        -> 즉, Monte Carlo 적분을 코드로 구현하는 과정에서
        구현 상의 편의를 위해 일부 간소화된 부분이라고 보면 됨.
      */
      totalWeight += NdotL;
    }
  }

  // MC 적분의 기댓값 E 계산 과정에서 시그마 합의 평균을 구하기 위해 1 / N 을 곱해줌.
  prefilteredColor = prefilteredColor / totalWeight;

  // 계산된 surface point P 지점의 diffuse term 의 irradiance 를 큐브맵 버퍼에 저장
  FragColor = vec4(prefilteredColor, 1.0);
}

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
