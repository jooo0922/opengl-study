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

  // 3가지 성분을 모두 더한 뒤, 바닥 평면 텍스쳐 색상값(diffuse color)를 곱하여 최종 색상 계산
  vec3 lighting = (ambient + diffuse + specular) * color;

  FragColor = vec4(lighting, 1.0);
}
