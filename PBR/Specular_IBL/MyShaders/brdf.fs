#version 330 core

// 프래그먼트 쉐이더 출력 변수 선언
out vec2 FragColor;

// vertex shader 단계에서 전달받는 입력 변수 선언
in vec2 TexCoords;

// PI 상수값 정의
const float PI = 3.14159265359;

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

/*
  Geometry Function

  전체 미세면(microfacets)들 중에서, 
  다른 미세면을 가림으로써, 특정 방향으로 진행하는 빛이 차페(occluded)되는 면적의 비율,
  한마디로 미세면의 거칠기에 의해 생성되는 그림자의 면적 비율을 통계적으로 근사함.

  roughness 가 클수록 미세면이 울퉁불퉁 해지므로,
  특정 미세면이 다른 미세면을 더 많이 가리게 됨.

  따라서, Geometry Function 의 비율값이 작아지므로, 더 많은 빛이 차폐되어 보이도록, 
  즉, 그림자가 더 많아 보이도록 렌더링됨.
*/

/*
  이 예제에서는 특정 방향(첫 번째 매개변수 NdotV 의 V)으로 진행되는 빛이
  미세면에 의해 차폐되는 비율을 근사하는 Schlick-GGX 모델을 Geometry Function 으로 사용함.
*/
float GeometrySchlickGGX(float NdotV, float roughness) {
  // IBL(간접광) 계산 시, 아래와 같이 roughness(α) 을 remapping 한 k 항을 사용함. (노션 IBL 필기 참고)
  float a = roughness;
  float k = (a * a) / 2.0;

  // Geometry Function 모델의 분자 항 계산
  float nom = NdotV;

  // Geometry Function 모델의 분모 항 계산
  float denom = NdotV * (1.0 - k) + k;

  // Geometry Function 모델의 결과값 반환
  return nom / denom;
}

/*
  Smith's method

  Geometry Function 을 효과적으로 근사하기 위해서는
  빛이 들어오는 방향(= 조명벡터. Wi)에서 미세면에 의해 차폐되는 면적의 비율과
  빛이 반사되어 나가는 방향(= 뷰 벡터. Wo)에서 미세면에 의해 차폐되는 면적의 비율을
  모두 고려해야 함.

  이를 효과적으로 수행하는 방법은,
  Schlick-GGX 함수로 두 빛의 진행 방향에 대한 
  차페되어 그림자 지는 면적의 비율을 각각 계산하고,
  두 비율값을 곱하는 방법을 사용하는데, 이를 'Smith's method' 라고 함.
*/
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
  float NdotV = max(dot(N, V), 0.0);
  float NdotL = max(dot(N, L), 0.0);

  // 빛이 반사되어 나가는 방향에서 차폐되는 면적의 비율 계산
  float ggx1 = GeometrySchlickGGX(NdotV, roughness);

  // 빛이 들어오는 방향에서 차폐되는 면적의 비율 계산
  float ggx2 = GeometrySchlickGGX(NdotL, roughness);

  return ggx1 * ggx2;
}

/*
  Monte Carlo 적분을 사용해서 split-sum approximation 의
  두 번째 적분식(= BRDF Integration map)을 convolution 하는 함수
  (노션 IBL 필기 참고)

  BRDF Integration map 의 uv 좌표값 역할을 하는 
	내적값 NdotV 와 roughness 를 매개변수로 받아 적분을 계산함.
*/
vec2 IntegrateBRDF(float NdotV, float roughness) {
  // 매개변수인 내적값 NdotV 로부터 카메라 뷰 벡터(= V) 를 역추적하여 계산 (하단 필기 및 노션 필기 참고)
  vec3 V;
  V.x = sqrt(1.0 - NdotV * NdotV);
  V.y = 0.0;
  V.z = NdotV;

  // split sum approximation 의 두 번째 적분식을 계산할 때, 결과값(scale 과 bias)을 누산할 변수 초기화
  float A = 0.0;
  float B = 0.0;

  // convolution 과정에서 사용할 노멀벡터 N 을 Positive-Z 축에 대응되는 (0, 0, 1)로 가정함. (관련 필기 하단 참고)
  vec3 N = vec3(0.0, 0.0, 1.0);

  /* 
    surface point P 지점에서 specular lobe 영역으로 반사되는 빛들에 대한 
    BRDF 항 결과값의 총합(= 두 번째 적분식)을 Monte Carlo 적분으로 계산 
  */

  // Monte Carlo 적분의 샘플링 개수를 uint 상수값으로 정의
  const uint SAMPLE_COUNT = 1024u;

  // Monte Carlo 적분의 샘플링 개수만큼 for-loop 를 순회하며 기댓값 E 에 대한 시그마 식을 이산적(discretely)으로 계산
  for(uint i = 0u; i < SAMPLE_COUNT; i++) {

    /*
      Importance Sampling 기법을 사용하여
      표면의 roughness 값에 따라 정의되는 specular lobe 범위 내에 존재하는 하프벡터 H 와
      반대방향의 입사각 벡터 L 을 계산하는 과정은

      Prefiltered env map 적분을 계산하는 예제에서 사용한 방식과 동일함! (노션 IBL 필기 참고)
		*/

    // low-discrepancy sequence 를 생성하는 알고리즘 중 하나인 Hammersley sequence 함수를 사용하여 uniformly random 한 sample 추출 (노션 IBL 관련 필기 참고)
    vec2 Xi = Hammersley(i, SAMPLE_COUNT);

    // low-discrepancy sequence 로부터 얻은 랜덤한 vec2 값으로 표면의 roughness 값에 따라 정의되는 specular lobe 범위 내에 존재하는 하프벡터 H 계산 
    vec3 H = ImportanceSampleGGX(Xi, N, roughness);

    // 현재 surface point P 에 대한 하프벡터 H (specular lobe 범위 내에 존재) 를 기준으로, 카메라 view vector V 에 대한 반사벡터 (즉, 반사벡터의 반사벡터) 역추적
    // 이 공식은 자세히 들여다보면, 결국 반사벡터를 계산하는 GLSL 내장함수 reflect() 함수의 내부 구현부와 동일함을 알 수 있음!
    vec3 L = normalize(2.0 * dot(V, H) * H - V);

    /*
      BRDF 계산에 사용할 내적값을 미리 계산해 둠. (하단 필기 및 노션 IBL 필기 참고)
    */
    float NdotL = max(L.z, 0.0);
    float NdotH = max(H.z, 0.0);
    float VdotH = max(dot(V, H), 0.0);

    /*
      가중치가 0 으로 곱해지면 어차피 적분의 누산값에 반영할 수 없으므로,
      가중치가 0 보다 큰 경우에만 한하여 누산값 A, B 에 반영함.
    */
    if(NdotL > 0.0) {
      // BRDF 함수의 G 항(= Geometry Function) 계산
      float G = GeometrySmith(N, V, L, roughness);

      // Unreal Engine PBR 관련 문서에 나오는 BRDF 함수 계산 최적화 코드를 가져다 씀. (하단 필기 및 노션 필기 참고)
      float G_Vis = (G * VdotH) / (NdotH * NdotV);

      // 두 번째 적분식을 기본반사율 F0 에 대한 선형결합된 적분식으로 유도한 것에서, (1 - dot(w_o, h))^5 항을 코드로 구현
      float Fc = pow(1.0 - VdotH, 5.0);

      // 선형결합된 두 적분식의 결과값을 누산할 두 변수 A(= scale), B(= bias) 에 각각 누산
      A += (1.0 - Fc) * G_Vis;
      B += Fc * G_Vis;
    }
  }

  // MC 적분의 기댓값 E 계산 과정에서 시그마 합의 평균을 구하기 위해 1 / N 을 곱해줌.
  A /= float(SAMPLE_COUNT);
  B /= float(SAMPLE_COUNT);

  // 선형결합으로 분리된 두 적분식의 결과값(scale, bias)를 vec2 타입으로 묶어서 반환
  return vec2(A, B);
}

void main() {
  /*
    MC 적분을 통해 계산된 BRDF Integration map 에 저장할 두 번째 적분식의 결과값을 반환받음.
    이때, 보간된 uv 좌표값을 BRDF Integration map 의 두 매개변수 NdotV 와 roughness 로 가정하고 입력 
  */
  vec2 integrateBRDF = IntegrateBRDF(TexCoords.x, TexCoords.y);

  // 현재 프레임버퍼에 attach 된 2D 텍스쳐 버퍼에 선형결합으로 분리된 두 적분식의 결과값(= scale, bias)을 저장
  FragColor = integrateBRDF;
}

/*
	매개변수 NdotV 로부터 카메라 뷰 벡터 V 를 역추적 하는 원리


  여기서 vec3 V 는
	내적값인 NdotV 로부터 카메라 뷰 벡터를 역추적하여 계산한 결과임.
	
	그런데 어떻게 내적값으로부터 뷰 벡터를 계산할 수 있는걸까?
	
	일단 IntegrateBRDF() 함수에서는
	노멀벡터 N 을 Positive-Z 축에 대응되는 (0, 0, 1)로 가정함.
	
	어차피 Integration map 자체가 2D 평면이고,
	여기에 저장할 값을 계산하기 위해 사용하는 노멀벡터를 
	반드시 실제 공간 상의 표면에 대한 노멀벡터로 구할 필요가 없음.
	
	왜냐? 
	
	Integration map 에 새겨지는 각 BRDF 적분 계산의 결과값은
	노멀벡터에 따라 달라지는 것이 아니라, 
	'내적값 NdotV' 와 'roughness' 에 의해 달라지는 것이기 때문에
	
	노멀벡터는 그저 일정한 방향을 갖는 상수값(= (0, 0, 1))으로만
	고정시켜 놓더라도, 적분 결과값을 계산하거나 계산된 결과값을 sampling 하여
	가져다 사용하는 데에는 아무런 문제가 없음!
	
	
	자 그렇다면,
	노멀벡터가 Positive-Z 축 방향으로 고정되어 있다면,
	이 방향의 축과 카메라 뷰 벡터 간의 각도를 계산하고자 한다면,
	
	Positive-Z 축과 Positive-XZ half-plane 평면에 놓이는 임의의 벡터(= 이것을 뷰 벡터로 보는 것임!) 사이의
	각도를 상상한다면 더 직관적으로 계산할 수 있겠지?
	
	즉, 카메라 뷰 벡터를
	원점에서 시작하여 Positive-XZ half-plane 평면 상에 존재하는 임의의 벡터로 정의하고,
	이것과 상수 방향으로 고정된 노멀벡터 Positive-Z 축 간의 각도를 계산하면 되겠지.
	
	따라서, 카메라 뷰 벡터는 Positive-XZ 평면 상의 벡터이고,
	이 벡터가 Positive-Z 축과 이루는 각도 θ 로 본다면, 
	XZ 평면 상의 원의 좌표값을 계산하는 공식에 따라 
	카메라 뷰 벡터 V = (sinθ, 0, cosθ) 로 정의할 수 있음.
	
	그래서 일단 V.y = 0 으로 계산한 것이고,
	
	이때, NdotV 자체는 두 벡터 N, V 의 길이가 1이고,
	두 벡터의 사잇각도 θ 이므로, 두 벡터 N, V 의 내적값 NdotV 자체가 
	두 벡터 N, V 의 사잇각 cosθ 와 같다고 볼 수 있음.
	따라서, V.z = cosθ = NdotV 로 계산할 수 있음.
	
	마지막으로, 삼각함수의 기본 성질에 따라,
	sin^2θ + cos^2θ = 1, (게임수학 p.118 참고)
	sin^2θ = 1 - cos^2θ,
	sinθ = sqrt(1 - cos^2θ) 
	와 같이 유도할 수 있으므로,
	
	V.x = sqrt(1.0 - NdotV*NdotV) 로 계산한 것임.
	
	아래 링크 설명 참고!
	https://computergraphics.stackexchange.com/questions/10791/pbr-understanding-the-right-part-of-split-sum-integration-of-specular-ibl
*/

/*
	기저 축과의 내적 계산 간소화

  
  아래 NdotL 과 NdotH 내적을 계산할 때,
	dot() 내장함수를 사용하지 않고, 특이한 방식으로 내적값을 계산하고 있지?
	
	이것은 노멀벡터 N 이 (0, 0, 1) 로 정의되었기 때문에,
	내적 계산 공식을 그대로 따라 계산해보면,
	NdotL = (0 * L.x) + (0 * L.y) + (1 * L.z) = L.z
	와 같이 노멀벡터 N 와 내적 계산되는 상대방 벡터의 z 컴포넌트만 남는 것을 확인할 수 있음.
	
	따라서, 노멀벡터 N 과의 내적 계산은
	상대방 벡터의 z 컴포넌트를 clamping 하는 것으로
	아주 간편하게 계산할 수 있는 이점이 존재함!
	
	아래 링크 설명 참고!
	https://computergraphics.stackexchange.com/questions/10791/pbr-understanding-the-right-part-of-split-sum-integration-of-specular-ibl
*/

/*
  Unreal Engine 4 의 BRDF 계산식
  
  
  split-sum approximation 의 두 번째 적분식을 
  기본 반사율 F0 에 대한 선형 결합 형태로 유도된 적분식에서
  fr() 함수, 즉, BRDF 함수를 코드로 구현한 것인데,
  
  G_Vis 라는 난생 처음보는 값이 튀어나왔지?
  또, NdotL 내적값을 곱해주는 부분은 어디로 사라져버렸지?
  
  이 부분이 공식과 많이 달라서 당황스러울 수 있는데,
  이게 뭐 별거는 아니고, 언리얼 엔진 4 에서 BRDF Integration 계산할 때
  
  BRDF 함수 계산(여기서는 NDF 항과 G 항만 남이있음)을 
  최적화하여 변형한 코드를 그대로 가져다 쓴 거라고 보면 됨.
  
  아래 언리얼 PBR 관련 문서에서 예제 코드를 확인할 수 있음.
  https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
  
  그리고, 위 문서에서도 아주 작은 주석으로
  'Their shading model uses different D and G functions.'
  라고 표시되어 있음.
*/
