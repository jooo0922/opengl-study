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

void main() {
  vec3 color = texture2D(floorTexture, fs_in.TexCoords).rgb;

  /* ambient 성분 계산 */
  float ambientStrength = 0.05; // ambient 강도
  vec3 ambient = ambientStrength * color; // ambient 강도에 텍스쳐 색상 곱해서 최종 ambient 성분값 계산

  /* diffuse 성분 계산 */
  // 조명계산에 사용되는 모든 방향벡터들은 항상 정규화를 해줄 것! -> 그래야 내적계산 시 정확한 cos 값만 얻을 수 있음!
  vec3 lightDir = normalize(lightPos - fs_in.FragPos); // 조명벡터 (프래그먼트 위치 ~ 광원 위치)
  vec3 normal = normalize(fs_in.Normal); // 프래그먼트에 수직인 노멀벡터
  float diff = max(dot(lightDir, normal), 0.0); // 노멀벡터와 조명벡터 내적 > diffuse 성분의 세기(조도) 계산 (참고로, 음수인 diffuse 값은 조명값 계산을 부정확하게 만들기 때문에, 0.0 으로 clamping 시킴)
  vec3 diffuse = diff * color; // diffuse 조도에 텍스쳐 색상 곱해서 최종 diffuse 성분값 계산

  /* specular 성분 계산 */
  vec3 viewDir = normalize(viewPos - fs_in.FragPos); // 뷰 벡터 (카메라 위치 ~ 프래그먼트 위치)
  float spec = 0.0; // specular 조도 선언 및 초기화

  if(blinn) {
    /* blinn-phong 반사 모델 적용 시 spec 계산 */

    // 뷰 벡터와 조명벡터 사이의 halfway 벡터 계산 (두 벡터의 합 -> 두 벡터 사이를 가로지르는 하프 벡터 (<셰이더 코딩 입문> p.222 참고))
    vec3 halfwayDir = normalize(lightDir + viewDir);

    // specular 성분의 조도 계산
    // 프래그먼트 지점의 normal 벡터와 halfway 벡터를 clamping 내적함
    // 내적값을 32제곱 > 32는 shininess 값으로써, 값이 클수록 highlight 영역이 정반사되고, 값이 작을수록 난반사됨.
    // Blinn-Phong 모델에서는 동일한 조건 하에 기본 Phong 모델에 비해 내적값이 더 작게 계산되기 때문에,
    // 일반적인 관례상 기본 Phong 모델보다 2~4배 큰 shininess(광택값)을 사용한다.
    spec = pow(max(dot(normal, halfwayDir), 0.0), 32);
  } else {
    /* 기본 phong 반사 모델 적용 시 spec 계산 */

    // 반사 벡터 (조명벡터는 카메라 위치부터 출발하도록 방향을 negate) 계산
    vec3 reflectDir = reflect(-lightDir, normal); 

    // specular 성분의 조도 계산
    // 뷰 벡터와 반사 벡터를 clamping 내적함
    spec = pow(max(dot(viewDir, reflectDir), 0.0), 8);
  }
  vec3 specular = spec * vec3(0.3); // specular 조도에 조명색상(약간 밝은 흰색)을 곱해 specular 성분값 계산

  // 3가지 성분을 모두 더하여 최종 색상 결정
  FragColor = vec4(ambient + diffuse + specular, 1.0);
}

/*
  Blinn-Phong 반사 모델을 사용하는 이유


  기본 Phong 반사 모델에서
  낮은 shininess(광택값)이 적용되어 specular 반경이 커질 경우,
  특정 지점부터 specular 가 끊기는(immediatley cut off) 것처럼 렌더링되는데

  기본적으로는 이러한 artifact 를 해결하기 위해
  Blinn-Phong 반사 모델을 사용하는 것임.

  자세한 내용은 LearnOpenGL 본문 및 <셰이더 코딩 입문> P.222 참고
*/