#version 330 core

out vec4 FragColor;

// vertex shader 단계에서 전달받는 입력 변수 선언
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

/* OpenGL 에서 전송해 줄 uniform 변수들 선언 */

// PBR Material 파라미터 값을 전송받는 uniform 변수 선언
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

// diffuse term 에 대한 irradiance 계산 결과가 저장된 큐브맵 텍스쳐(= irradiance map) 선언
uniform samplerCube irradianceMap;

// specular term 에 대한 split-sum approximation 의 첫 번째 적분식 계산 결과가 저장된 큐브맵 텍스쳐(= pre-filtered env map) 선언
uniform samplerCube prefilterMap;

// specular term 에 대한 split-sum approximation 의 두 번째 적분식 계산 결과가 저장된 2D LUT 텍스쳐(= BRDF Integration map) 선언
uniform sampler2D brdfLUT;

// 광원 정보를 전송받는 uniform 변수 선언
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

// 카메라 위치값을 전송받는 uniform 변수 선언
uniform vec3 camPos;

// Pi 상수 선언
const float PI = 3.14159265359;

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
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = PI * denom * denom;

  // NDF 모델의 결과값 반환
  return nom / denom;
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
  // direct lighting(직접광) 계산 시, 아래와 같이 roughness(α) 을 remapping 한 k 항을 사용함. (하단 필기 참고)
  float r = (roughness + 1.0);
  float k = (r * r) / 8.0;

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
  Fresnel Equation

  들어오는 빛(Wi)이 surface point(p) 에 도달했을 때,
  반사되는 빛(reflection)과 굴절되는 빛(refraction)으로 나뉘게 되는데,
  이 중에서 반사되는 빛의 비율(the ratio of light that gets reflected)을 근사함.

  -> 참고로, Fresnel 값은 에너지 보존 법칙에서 빛이 반사되는 비율을 뜻하는 kS 항을 대체할 수 있음.

  실제 Fresnel 을 계산하는 공식은 아주 복잡하지만,
  이 예제에서는 각 재질의 기본 반사율(base reflectivity) F0 을 가지고서
  Fresnel 값을 근사하는 Schlick's approximation 모델을 사용함.

  자세한 내용은 노션 계획표의 Fresnel Equation 관련 필기 참고
*/
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
  return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

/*
  roughness 파라미터가 주입된 Fresnel Equation

  이론적으로 보자면, 프레넬 반사율은 표면의 거칠기(roughness)에 간접적으로 영향을 받게 됨.
  왜냐하면, 표면이 거칠수록 빛이 반사되는 분포가 더 넓어지기 때문에, 
  즉, 빛이 더 다양한 방향으로 반사되기 때문에, 
  Wo 방향(카메라 방향)으로 반사되어 들어오는 비율이 줄어들겠지!

  direct lighting(직접광) 에서 specular term 계산 시에는
  NDF, G 항에 의해 표면의 roughness 가 어느 정도 반영되었지만,

  IBL(indirect lighting(간접광)) 의 diffuse term 만 계산 시에는
  roughness 파라미터와 무관하게 프레넬 반사율을 계산하므로,
  표면의 거칠기와 무관하게 상대적으로 높은 반사율이 적용되어 버림.

  특히, non-metallic surface 에서 roughness 파라미터가 높을 때,
  표면의 가장자리(surface edges) 부분이 티가 날 정도로 희게 빛이 남. (-> LearnOpenGL 본문 이미지 참고)
  
  -> 그러나, 물리적으로 더 정확하게 렌더링하려면,
  roughness 가 클수록, 즉, 표면이 더 거칠수록 프레넬 반사율은 줄어들어야 함!

  이를 해결하기 위해, 기존 fresnelSchlick() 함수에
  roughness 파라미터를 매개변수를 통해 주입하고,
  표면의 거칠기에 의해 프레넬 반사율이 보정될 수 있도록 기존 함수를 약간 수정한 것!
*/
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
  return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
  /* 일반적인 조명 알고리즘에 필수적인 방향 벡터들 계산 */

  // 버텍스 쉐이더에서 보간된 world space 노멀벡터를 정규화하여 계산해 둠.
  vec3 N = normalize(Normal);

  // world space 뷰 벡터 계산
  vec3 V = normalize(camPos - WorldPos);

  /*
    카메라 view vector 방향으로 들어오는 빛 Wo 에 대한 
    반사벡터를 역추적하여 입사각 벡터 Wi 계산

    -> 현재 순회중인 surface point P 지점으로 들어와서
    specular lobe 영역으로 반사되는 빛들에 대한 적분값을 pre-filtered env map 으로부터 
    fetch 하기 위한 방향벡터 R 를 구하려는 것!
  */
  vec3 R = reflect(-V, N);

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

    // NDF 비율값 계산
    float NDF = DistributionGGX(N, H, roughness);

    // Geometry Function 비율값 계산
    float G = GeometrySmith(N, V, L, roughness);

    // Fresnel(빛의 파장별(r, g, b 채널) 반사되는 비율값) 계산
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    // Specular term 의 분자 항 계산
    vec3 numerator = NDF * G * F;

    // Specular term 의 분모 항 계산 -> 내적값이 0 이 되면 분모가 0이 되므로, 이를 방지하기 위해 0.0001 을 더함
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;

    // Specular term 계산
    vec3 specular = numerator / denominator;

    /* Diffuse term 계산 */

    // 빛이 반사되는 비율(kS)는 Fresnel 값과 일치함.
    vec3 kS = F;

    // 에너지 보존 법칙에 따라, 빛이 굴절되는 비율은 전체 비율 1 에서 반사되는 비율을 빼서 계산.
    vec3 kD = vec3(1.0) - kS;

    // metallic 값에 따라 빛의 굴절률을 조정함 (관련 필기 하단 참고)
    kD *= 1.0 - metallic;

    /* 
      surface point(p)와 direct lighting 이 방출하는 빛(Wi)의 각도에 따른 
      radiance 조절하기 위한 cosTheta 계산 

      -> 렌더링 방정식에서 n⋅ωi 에 해당
    */
    float NdotL = max(dot(N, L), 0.0);

    // 현재 순회중인 direct lighting 에 대한 surface radiance 를 렌더링 방정식으로 계산하고, 결과값을 누산
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
  }

  // 환경광(ambient lighting) 계산 -> IBL 챕터에서 이 환경광이 environment lighting 으로 대체될 것임. -> 하단 필기 참고
  // vec3 ambient = vec3(0.03) * albedo * ao;

  /* 상수값을 사용하던 환경광(ambient lighting) 을 IBL 로 대체하여 계산 (하단 필기 참고) */

  /*
    indirect lighting(간접광 또는 IBL)은 노멀벡터 N 을 중심으로 한 반구 영역 전체에 걸쳐서 
    무수히 많은 incoming lights 가 들어오므로,

    direct lighting(직접광)과 달리 fresnelSchlick() 함수로 반사율을 계산할 때,
    조명 벡터(Wi)와 뷰 벡터(Wo) 사이의 하프 벡터(H) 를 사용할 수 없음.

    Wi 가 무수히 많기 때문에, 단일한 하프 벡터 하나를 딱 정해서 내적 계산에 사용하기가 어려움.

    이를 위해, 현재 surface point P 에 대한 노멀 벡터(N)과 뷰 벡터(Wo 또는 V) 간의
    내적 계산으로 대체하여 사용하기로 함.

    + 표면의 거칠기에 따른 프레넬 반사율 보정을 위해, 
    roughness 값을 주입한 버전의 Fresnel Equation 함수를 사용함
  */
  vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

  // 에너지 보존 법칙에 따라, 빛이 굴절되는 비율은 전체 비율 1 에서 반사되는 비율을 빼서 계산.
  vec3 kS = F;
  vec3 kD = 1.0 - kS;

  // metallic 값에 따라 빛의 굴절률을 조정함 (관련 필기 하단 참고)
  kD *= 1.0 - metallic;

  // 현재 surface point P 지점의 방향벡터 N 을 사용하여 P 지점에 도달하는 모든 indirect lighting 의 총량인 irradiance 를 읽어옴
  vec3 irradiance = texture(irradianceMap, N).rgb;

  /*
    반사율 방정식의 diffuse term 을 계산한 irradiance 에다가 
    LearnOpenGL 본문의 이중시그마 식에서 c 에 해당하는 난반사 색상 albedo 를 곱함.

    이 c 값은 이론적으로 보자면,
    Environment map 에 존재하는 주변 물체들에서 한번 굴절되었다가 빠져나온 '난반사(diffuse term)'의
    색상값만을 별도의 계산 없이 단순한 상수값으로 정의한 것이라고 보면 될 것임!
  */
  vec3 diffuse = irradiance * albedo;

  /* 반사율 방정식의 specular term 계산 */

  // roughness level 에 따라 5단계로 저장된 pre-filtered env map 의 최대 mip level 을 상수로 초기화
  const float MAX_REFLECTION_LOD = 4.0;

  /*
    uniform 변수로 입력받는 [0.0, 1.0] 사이의 roughness 값에 따라 LOD 를 계산하여 
    그에 맞는 mip level 의 pre-fitered env map 으로부터 specular lobe 영역 내로 반사되는 빛들의 총합을 적분한
    split-sum approximation 의 첫 번째 적분식의 결과값을 fetch 해옴. 
  */ 
  vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;

  // BRDF Integration map 에 저장된 Scale 과 Bias 값 샘플링 (-> NdotV 내적값과 roughness 값을 uv좌표값 삼아 샘플링함.)
  vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;

  // 기본 반사율 F0 이 아닌, 실제 반사율 F 에 대한 선형결합인 F * Scale + Bias 형태로 specular term 적분식을 최종 계산 (노션 IBL 필기 참고)
  vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

  /*
    LearnOpenGL 본문의 이중시그마 식에서 kD 에 해당하는 굴절율을 diffuse term 에 곱해주고,
    pre-filtered env map 과 BRDF Integration map 으로부터 샘플링해와서 계산한 specular term 적분식의 결과값을 더하고,
    ambient occlusion factor 를 곱해서 환경광이 차폐되는 영역까지 고려하여
    IBL 을 사용한 최종 ambient lighting 계산 완료! 
  */
  vec3 ambient = (kD * diffuse + specular) * ao;

  // 현재 surface point 지점에서 최종적으로 반사되는 조명값 계산
  vec3 color = ambient + Lo;

  // Reinhard Tone mapping 알고리즘을 사용하여 HDR -> LDR 변환
  /*
    [0, 1] 범위를 벗어난 HDR 색상값을
    [0, 1] 범위 내로 존재하는 LDR 색상값으로 변환하기

    -> 즉, Tone mapping 알고리즘 적용!
    https://github.com/jooo0922/opengl-study/blob/main/AdvancedLighting/HDR/MyShaders/hdr.fs 참고
  */
  color = color / (color + vec3(1.0));

  // linear space 색 공간 유지를 위해 gamma correction 적용하여 최종 색상 출력
  color = pow(color, vec3(1.0 / 2.2));
  FragColor = vec4(color, 1.0);
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

/*
  Geometry Function 에서의 roughness 거듭제곱 방식


  NDF 함수에서와 마찬가지로, Geometry Function 에서도
  roughness 값을 거듭제곱해야 더 정확한 미세면을 렌더링할 수 있는데,

  해당 모델을 direct lighting 에서 사용하느냐,
  IBL(Image Based Lighting) 같은 간접광에서 사용하느냐에 따라
  roughness 를 거듭제곱하여 remapping 하는 방식이 달라짐.

  자세한 공식은 LearnOpenGL PBR > Theory 참고
*/

/*
  metallic 값에 따른 빛의 굴절률 조정
  
  금속은 굴절된 빛을 모두 흡수해서 난반사(diffuse reflection)가 존재하지 않음.
  반면, 비전도체(dielectric)는 난반사가 존재하고, 비금속 이물질이 금속 표면 위에 쌓인 경우라면,
  '일부분만' 난반사가 될 것임.

  따라서, Metallic workflow 를 따라
  선형적으로 굴절률을 조정하기 위해,
  굴절률에 linear metallic 값을 1.0 에서 뒤집은 값만큼 곱해줌.
  
  이렇게 하면, 완전 금속이라면, metallic 이 1.0 일테니,
  0.0(= 1.0 - 1.0) 을 곱하게 되어 굴절률 kD 가 0.0 이 되어버리겠지!
*/

/*
  ambient lighting (환경광) 계산 시, albedo 색상값을 곱해주는 이유


  환경광도 다른 직접광(point light, spot light, rect light)들 처럼, 
  반사되는 부분과 굴절되는 부분이 존재하고, 
  굴절되었다가 입자에 흡수되는 부분이 있고, 
  다시 표면 밖으로 빠져나오는 빛(== diffuse light)이 있겠지?

  따라서, 환경광이 굴절되어서 표면 밖으로 빠져나오는 라이팅을 계산하려면, 
  reflectance equation 의 diffuse term 에서 Kd * albedo 로 곱해줬던 것과 마찬가지로, 
  환경광의 조명값 vec3(0.03) 에 albedo 를 곱해줘야겠지!
*/

/*
  ambient lighting (환경광) 을 IBL 로 교체하는 이유


  IBL 에서는 Environment map 내의 각 texel 들을
  주변 물체들로부터 반사된 간접광(indirect lighting)으로 가정하기 때문에,

  이러한 Environment map 을 convolution 하여 만들어 낸
  각 surface point P 에 대한 irradiance 의 총량을 계산하여 저장한
  큐브맵 버퍼인 irradiance map 로부터 샘플링한 값 자체가 곧 환경광(ambient lighting)이라고 볼 수 있겠지!
*/
