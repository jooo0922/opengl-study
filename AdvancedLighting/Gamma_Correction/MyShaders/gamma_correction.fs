#version 330 core

out vec4 FragColor;

/* vertex shader 단계에서 전달받는 입력 interface block 선언 */
in VS_OUT {
  vec3 FragPos;
  vec3 Normal;
  vec2 TexCoords;
} fs_in;

/* 
  OpenGL 에서 전송해 줄 uniform 변수들 선언 
*/

// 바닥 평면 텍스쳐 (0번 texture unit 에 바인딩된 텍스쳐 객체 샘플링)
uniform sampler2D floorTexture;

// 4개의 광원 위치를 전송받는 정적 배열 선언
uniform vec3 lightPositions[4];

// 4개의 조명 색상을 전송받는 정적 배열 선언
uniform vec3 lightColors[4];

// 카메라 위치 > 뷰 벡터 계산에서 사용
uniform vec3 viewPos;

/*
  BlinnPhong 계산 함수 별도 추출
*/
vec3 BlinnPhong(vec3 normal, vec3 fragPos, vec3 lightPos, vec3 lightColor) {
  /* 
    diffuse 성분 계산 
  */

  // 조명계산에 사용되는 모든 방향벡터들은 항상 정규화를 해줄 것! -> 그래야 내적계산 시 정확한 cos 값만 얻을 수 있음!

  // 조명벡터 (프래그먼트 위치 ~ 광원 위치)
  vec3 lightDir = normalize(lightPos - fs_in.FragPos);

  // 노멀벡터와 조명벡터 내적 > diffuse 성분의 세기(조도) 계산 (참고로, 음수인 diffuse 값은 조명값 계산을 부정확하게 만들기 때문에, 0.0 으로 clamping 시킴)
  float diff = max(dot(lightDir, normal), 0.0);

  // diffuse 조도에 조명 색상 곱해서 최종 diffuse 성분값 계산
  vec3 diffuse = diff * lightColor; 

  /* 
    specular 성분 계산 (blinn-phong 반사 모델 적용) 
  */

  // 뷰 벡터 (카메라 위치 ~ 프래그먼트 위치)
  vec3 viewDir = normalize(viewPos - fs_in.FragPos);

  // specular 조도 선언 및 초기화
  float spec = 0.0; 

  // 뷰 벡터와 조명벡터 사이의 halfway 벡터 계산 (두 벡터의 합 -> 두 벡터 사이를 가로지르는 하프 벡터 (<셰이더 코딩 입문> p.222 참고))
  vec3 halfwayDir = normalize(lightDir + viewDir);

  // specular 성분의 조도 계산
  // 프래그먼트 지점의 normal 벡터와 halfway 벡터를 clamping 내적함
  // 내적값을 64제곱 > 64는 shininess 값으로써, 값이 클수록 highlight 영역이 정반사되고, 값이 작을수록 난반사됨.
  // Blinn-Phong 모델에서는 동일한 조건 하에 기본 Phong 모델에 비해 내적값이 더 작게 계산되기 때문에,
  // 일반적인 관례상 기본 Phong 모델보다 2~4배 큰 shininess(광택값)을 사용한다.
  spec = pow(max(dot(normal, halfwayDir), 0.0), 64);

  // specular 조도에 조명색상을 곱해 specular 성분값 계산
  vec3 specular = spec * lightColor;

  /* 
    Point(?) Light 감쇄 계산 
  */

  // 광원에서 각 프래그먼트 사이의 거리
  float distance = length(lightPos - fragPos);

  // 물리적으로 정확한 계산을 위해, 거리 제곱의 반비례로 감쇄 계산
  float attenuation = 1.0 / distance * distance;

  /*
    각 조명 성분에 감쇄 적용
  */
  diffuse *= attenuation;
  specular *= attenuation;

  // 각 조명 성분을 더해서 최종 BlinnPhong 조명값 계산하여 반환
  return diffuse + specular;
}

void main() {
  // diffuse 텍스쳐 샘플링
  vec3 color = texture2D(floorTexture, fs_in.TexCoords).rgb;

  // 누산할 조명값 초기화
  vec3 lighting = vec3(0.0);

  // 광원 개수만큼 반복문을 순회하여 BlinnPhong 조명값 누산
  for(int i = 0; i < 4; i++) {
    lighting += BlinnPhong(normalize(fs_in.Normal), fs_in.FragPos, lightPositions[i], lightColors[i]);
  }

  // 텍스쳐 색상에 누산된 조명값을 곱해 최종 색상 계산
  color *= lighting;

  // 3가지 성분을 모두 더하여 최종 색상 결정
  FragColor = vec4(color, 1.0);
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