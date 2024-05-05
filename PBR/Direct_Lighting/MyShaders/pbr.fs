#version 330 core

out vec4 FragColor;

// vertex shader 단계에서 전달받는 입력 변수 선언
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

/* OpenGL 에서 전송해 줄 uniform 변수들 선언 */

// PBR Material 파라미터 값이 텍셀 단위로 저장된 텍스쳐 객체들의 sampler 변수 선언
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

// 광원 정보를 전송받는 uniform 변수 선언
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

// 카메라 위치값을 전송받는 uniform 변수 선언
uniform vec3 camPos;

// Pi 상수 선언
const float PI = 3.14159265359;

// 노멀맵으로부터 노멀벡터를 추출하여 반환하는 함수 (하단 필기 참고)
vec3 getNormalFromMap() {
  // 노멀맵에서 tangent space 노멀벡터를 샘플링한 뒤, 해당 값의 범위를 [0.0, 1.0] -> [-1.0, 1.0] 으로 맵핑함
  vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;

  // 현재 프래그먼트와 이웃한 프래그먼트와의 WorldPos.x 의 변화량에 대한 WorldPos(world space 위치값) 의 변화량을 편미분하여 계산 -> Normal Mapping 챕터에서 Edge 벡터 E₁ 에 해당
  vec3 Q1 = dFdx(WorldPos);

  // 현재 프래그먼트와 이웃한 프래그먼트와의 WorldPos.y 의 변화량에 대한 WorldPos(world space 위치값) 의 변화량을 편미분하여 계산 -> Normal Mapping 챕터에서 Edge 벡터 E₂ 에 해당
  vec3 Q2 = dFdy(WorldPos);

  // 현재 프래그먼트와 이웃한 프래그먼트와의 TexCoords.x 의 변화량에 대한 TexCoords(uv 좌표값) 의 변화량을 편미분하여 계산 -> Normal Mapping 챕터에서 uv 좌표 변화량 (ΔU₁, ΔV₁) 에 해당
  vec2 st1 = dFdx(TexCoords);

  // 현재 프래그먼트와 이웃한 프래그먼트와의 TexCoords.y 의 변화량에 대한 TexCoords(uv 좌표값) 의 변화량을 편미분하여 계산 -> Normal Mapping 챕터에서 uv 좌표 변화량 (ΔU₂, ΔV₂) 에 해당
  vec2 st2 = dFdy(TexCoords);

  // world space 로 변환하여 보간된 노멀벡터 정규화
  vec3 N = normalize(Normal);

  // Normal Mapping 챕터에서 탄젠트 공간의 탄젠트 벡터 계산 공식과 거의 유사한 방식으로 계산
  // (참고로, 여기서는 Q1, Q2 를 'world space 위치값'으로 계산했으니, '월드 공간의 탄젠트 벡터' 라고 봐야겠지?)
  vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);

  // 노멀벡터와 탄젠트 벡터를 외적하여 비탄젠트 벡터 계산
  vec3 B = -normalize(cross(N, T));

  /*
    world space 기준으로 정의된 3개의 직교 축인 
    tangent, bitangent, normal 벡터들을 열 벡터로 꽂아넣어
    3개의 world space 직교 벡터를 기저 벡터로 삼는 3*3 TBN 행렬 생성
  */
  mat3 TBN = mat3(T, B, N);

  // tangent space 노멀벡터에 TBN 행렬을 곱해 world space 노멀벡터로 변환하여 반환
  return normalize(TBN * tangentNormal);
}

/* Cook-Torrance BRDF 의 Specular term 계산에 필요한 함수들 구현 */

/*
  Normal Distribution Function

  전체 미세면(microfacets)들 중에서, 
  미세면의 노멀벡터가 half vector(조명 벡터(Wi)와 뷰 벡터(Wo) 사이의 벡터) 방향으로
  정렬되어 있는 미세면의 면적 비율을 통계적으로 근사함.

  roughness 가 클수록 미세면이 중구난방으로 정렬되고,
  NDF 의 비율값이 작아져서 표면이 더욱 거칠어보이게 렌더링됨.

  이 예제에서는 NDF 모델들 중에서 'Trowbridge-Reitz GGX' 라는 모델을 사용함.
*/
float DistributionGGX(vec3 N, vec3 H, float roughness) {
  // roughness 를 거듭제곱하여 α 에 remapping 함. -> 관련 내용 하단 필기 참고
  float a = roughness * roughness;

  // NDF 모델의 α² 항 계산
  float a2 = a * a;

  // NDF 모델의 n⋅h 계산
  float NdotH = max(dot(N, H), 0.0);

  // 내적값 제곱 계산
  float NdotH2 = NdotH * NdotH;

  // NDF 모델의 분자 항 계산
  float nom = a2;

  // NDF 모델의 분모 항 계산
  float denom = (NdotH * (a2 - 1.0) + 1.0);
  denom = PI * denom * denom;

  // NDF 모델의 결과값 반환
  return nom / denom;
}

void main() {
  /* PBR Material 파라미터 값을 각 텍스쳐들로부터 샘플링 */

  // albedo (물체의 diffuse 반사 색상값을 rgb 채널로 저장한 값) 샘플링 -> 샘플링된 texel 값 2.2 제곱 이유 하단 필기 
  vec3 albedo = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));

  // metallic 샘플링
  float metallic = texture(metallicMap, TexCoords).r;

  // roughness 샘플링
  float roughness = texture(roughnessMap, TexCoords).r;

  // ambient occlusion factor 샘플링
  /*
    https://github.com/jooo0922/opengl-study/blob/main/AdvancedLighting/SSAO/MyShaders/ssao_blur.fs
    
    위 SSAO 쉐이더 예제 코드에서 사용했던 SSAO occlusion factor 가 렌더링된 텍스쳐 버퍼와 동일한 역할.
    다만, 직접 attach 된 텍스쳐 버퍼에 렌더링된 것을 사용하느냐, 기존 이미지 텍스쳐를 사용하느냐의 차이!
  */
  float ao = texture(aoMap, TexCoords).r;

  /* 일반적인 조명 알고리즘에 필수적인 방향 벡터들 계산 */

  // 노멀맵에서 샘플링하여 world space 노멀벡터로 변환
  vec3 N = getNormalFromMap();

  // world space 뷰 벡터 계산
  vec3 V = normalize(camPos - WorldPos);

  /* Schlick's approximation 에 필요한 기본 반사율(base reflectivity) F0 계산 (자세한 내용은 노션 계획표의 Fresnel Equation 관련 필기 참고) */

  // 대부분의 비전도체(dielectric) 또는 비금속 이물질들의 기본 반사율의 평균을 낸 값인 0.04 사용
  vec3 F0 = vec3(0.04);

  // [0.0, 1.0] 사이의 metalness 값에 따라, 비금속 이물질의 반사율(F0)과 금속 표면의 반사율(surfaceColor)을 선형보간하여 섞음. -> Metallic workflow
  F0 = mix(F0, albedo, metallic);

  // 반사율 방정식(혹은 rendering equation)의 결과값을 누산할 변수 초기화
  vec3 Lo = vec3(0.0);

  /*
    광원 개수만큼 반복문을 순회하며 반사율 방정식을 계산하여 
    현재 프래그먼트 지점(p) 에서 Wo 방향(뷰 벡터)으로 반사되는 surface radiance 의 총량(Lo) 누산
    -> direct lighting(직접광) 에서는 적분으로 계산하지 않는 이유 관련 하단 필기 참고
  */
  for(int i = 0; i < 4; i++) {
    /* 각 direct lighting(직접광)이 방출하는 radiance(즉, 렌더링 방정식의 Li) 근사 */

    // 각 광원으로부터 들어오는 조명 벡터(Wi) 계산
    vec3 L = normalize(lightPositions[i] - WorldPos);

    // 조명 벡터(Wi)와 뷰 벡터(Wo) 사이의 하프 벡터 계산
    vec3 H = normalize(V + L);

    // 각 직접광과 surface point(p) 사이의 거리 계산
    float distance = length(lightPositions[i] - WorldPos);

    // 각 직접광과의 거리의 제곱에 반비례하는 감쇄 성분 계산
    float attenuation = 1.0 / (distance * distance);

    // 각 직접광에서 방사되는 radiance 계산
    vec3 radiance = lightColors[i] * attenuation;

    /* Cook-Torrance BRDF 계산 */

    /* Specular term 계산 */

    //
  }

}

/*
  샘플링된 albedo 색상값을 2.2 제곱 처리하는 이유


  보통 디자이너가 텍스쳐 이미지를 작업할 때, 
	이미 gamma correction(1/2.2 제곱) 이 적용된 상태에서
	이미지를 작업하는 경우가 많은데, 이를 'sRGB 색 공간이 적용된 텍스쳐' 라고 함.

	주로 diffuse, albedo 텍스쳐처럼,
	물체의 난반사된 색상을 표현하는 텍스쳐들은
	sRGB 색 공간으로 지정해놓고 작업하는 경우가 많음.

	그러나, gamma correction 이 적용되면 전체적으로 색상값이 밝아지기 때문에,

	쉐이더 객체에서 이미 gamma correction 이 적용된 텍스쳐를
	샘플링해서 다시 gamma correction (1/2.2 제곱) 을 적용해버리면

	결과적으로 gamma correction 이 두 번 적용됨으로써,
	텍스쳐 영역의 최종 색상이 과하게 밝아지는 문제가 발생함.

	이를 해결하기 위해,
	OpenGL 내부에서 자체적으로 텍스쳐 이미지 데이터를 저장할 때,
  internal format 을 GL_SRGB 또는 GL_SRGB_ALPHA 로 설정하면,

	"이 텍스쳐 데이터는 이미 sRGB 감마 보정이 적용되어 있으니, 
	linear space 색 공간으로 변환해서 저장해주세요"

	라고 명령하는 것과 같음.

  https://github.com/jooo0922/opengl-study/blob/main/AdvancedLighting/Gamma_Correction/gamma_correction.cpp 참고

  그러나, PBR 예제 코드는 Gamma Correction 예제 코드와 달리,
  loadTexture() 함수에서 internal format 을 GL_SRGB 로 저장하는 로직이 빠져있음.

  그래서, 이미 gamma correction 이 적용된 albedo 텍스쳐로부터 샘플링된 텍셀값에
  직접 2.2 제곱하여 linear space 색 공간으로 직접 변환하는 거라고 보면 됨. 
*/

/*
  getNormalFromMap()

  
  노멀맵에서 샘플링한 tangent space 기준 노멀벡터를
  world space 노멀벡터로 변환해 줌.

  이 함수는 LearnOpenGL Normal Mapping 챕터에서 QuadMesh 의
  tangent, bitangent 벡터를 직접 수동으로 계산하는 공식과 거의 유사함.

  다만, 이 예제에서는 프래그먼트 쉐이더에서 실행하고 있는 만큼,
  프래그먼트와 이웃한 프래그먼트 간의 변화량을 
  dFdx(), dFdy() 내장함수로 편미분하여 계산한다는 차이가 있을 뿐..

  자세한 내용은
  - https://learnopengl.com/Advanced-Lighting/Normal-Mapping 
  - https://github.com/jooo0922/opengl-study/blob/main/AdvancedLighting/Normal_Mapping/normal_mapping.cpp
  참고
*/

/*
  direct lighting 에서는 렌더링 방정식의 결과값을 적분하지 않는 이유


  direct lighting(또는 analytic lighting) 으로 렌더링 방정식을 계산할 때에는,
  적분을 사용하지 않고, 유한한 개수의 광원을 순회하며 계산된 방정식의 결과값을
  누산하는 식으로 코드를 구현하도록 되어있음.

  왜냐하면, direct lighting 은 말 그대로 '직접광'이기 때문에,
  반구 영역 전체에 걸쳐서 radiance(Li) 가 입사되는 것이 아님!

  point light, spot light, directional light 이런 애들은
  빛이 들어오는 방향이 한정되어 있으니까!

  즉, 빛이 들어오는 방향 벡터 Wi 가 유한하기(또는 '이산적이기') 때문에,
  애초에 '반구 영역'같은 넓은 영역으로 radiance 를 
  정의할 필요가 없음.

  그래서 이런 직접광들은 수학적인 방정식으로 결과값이 정확하게 표현되기 때문에,
  즉, 해석적 해(analytical solution)가 존재하기 때문에,
  '해석적 광원(analytic ligthing)' 이라고도 부름.

  따라서, 이러한 해석적 광원들은
  광원의 개수만큼 반복문을 순회하여 렌더링 방정식을 풀고,
  그 결과값을 특정 변수에 누산하는 방법을 통해
  특정 surface point 의 radiance(Lo)를 구할 수 있음!
*/

/*
  roughness 를 거듭제곱하여 사용하는 이유


  LearnOpenGL PBR > Theory 편에서는 
  Trowbridge-Reitz GGX 모델에서 roughness(α) 값 자체를
  거듭제곱하여 사용하지는 않지만, 

  Disney 및 Epic Games 연구 논문에 따르면,
  squared roughness 를 사용할수록, 미세면의 NDF 를 더 정확하게
  표현할 수 있다고 함.
*/
