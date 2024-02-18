#version 330 core

out vec4 FragColor;

// vertex shader 단계에서 전달받는 입력 interface block 선언
in VS_OUT {
  vec3 FragPos; // 월드공간 프래그먼트 위치값
  vec2 TexCoords;
  vec3 TangentLightPos; // 탄젠트 공간 조명 위치값
  vec3 TangentViewPos; // 탄젠트 공간 카메라 위치값
  vec3 TangentFragPos; // 탄젠트 공간 프래그먼트 위치값
} fs_in;

// OpenGL 에서 전송해 줄 uniform 변수들 선언
uniform sampler2D diffuseMap; // diffuse 텍스쳐 (0번 texture unit 에 바인딩된 텍스쳐 객체 샘플링)
uniform sampler2D normalMap; // normal 텍스쳐 (1번 texture unit 에 바인딩된 텍스쳐 객체 샘플링)

void main() {
  // normal map 으로부터 탄젠트 공간 노멀벡터 샘플링
  vec3 normal = texture2D(normalMap, fs_in.TexCoords).rgb;

  // [0, 1] 범위에 저장되어 있던 탄젠트 공간 노멀벡터를 [-1, 1] 범위로 변환
  normal = normalize(normal * 2.0 - 1.0);

  // diffsue map 으로부터 diffuse 색상 샘플링
  vec3 color = texture2D(diffuseMap, fs_in.TexCoords).rgb;

  /* ambient 성분 계산 */

  // ambient 강도
  float ambientStrength = 0.1;

  // ambient 강도에 텍스쳐 색상 곱해서 최종 ambient 성분값 계산
  vec3 ambient = ambientStrength * color; 

  /* diffuse 성분 계산 */
  // 조명계산에 사용되는 모든 방향벡터들은 항상 정규화를 해줄 것! -> 그래야 내적계산 시 정확한 cos 값만 얻을 수 있음!

  // 조명벡터 (탄젠트 공간 프래그먼트 위치 ~ 탄젠트 공간 광원 위치)
  vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);

  // 노멀벡터와 조명벡터 내적 > diffuse 성분의 세기(조도) 계산 (참고로, 음수인 diffuse 값은 조명값 계산을 부정확하게 만들기 때문에, 0.0 으로 clamping 시킴)
  float diff = max(dot(lightDir, normal), 0.0);

  // diffuse 조도에 텍스쳐 색상 곱해서 최종 diffuse 성분값 계산
  vec3 diffuse = diff * color; 

  /* specular 성분 계산 */

  // 뷰 벡터 (탄젠트 공간 카메라 위치 ~ 탄젠트 공간 프래그먼트 위치)
  vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos); 

  // 뷰 벡터와 조명벡터 사이의 halfway 벡터 계산 (두 벡터의 합 -> 두 벡터 사이를 가로지르는 하프 벡터 (<셰이더 코딩 입문> p.222 참고))
  vec3 halfwayDir = normalize(lightDir + viewDir);

  // specular 성분의 조도 계산
  // 프래그먼트 지점의 normal 벡터와 halfway 벡터를 clamping 내적함
  // 내적값을 32제곱 > 32는 shininess 값으로써, 값이 클수록 highlight 영역이 정반사되고, 값이 작을수록 난반사됨.
  // Blinn-Phong 모델에서는 동일한 조건 하에 기본 Phong 모델에 비해 내적값이 더 작게 계산되기 때문에,
  // 일반적인 관례상 기본 Phong 모델보다 2~4배 큰 shininess(광택값)을 사용한다.
  float spec = pow(max(dot(normal, halfwayDir), 0.0), 32);

  // specular 조도에 조명색상(어두운 흰색)을 곱해 specular 성분값 계산
  vec3 specular = spec * vec3(0.2); 

  // 3가지 성분을 모두 더하여 최종 색상 결정
  FragColor = vec4(ambient + diffuse + specular, 1.0);
}

  /*
    버텍스 쉐이더로부터
    탄젠트 공간 기준의 조명 위치값, 카메라 위치값, 프래그먼트 위치값을 모두 얻어왔으므로,

    normal map 에서 샘플링한
    탄젠트 공간 노멀벡터를 별도의 변환 처리 없이

    조명계산에 곧바로 사용할 수 있음!
  */
