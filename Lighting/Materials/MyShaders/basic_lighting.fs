#version 330 core

out vec4 FragColor;

// 버텍스 쉐이더에서 전송된 보간 변수
in vec3 Normal; // 월드공간 노멀벡터
in vec3 FragPos; // 월드공간 프래그먼트 위치값

// OpenGL 에서 전송해 줄 uniform 변수들 선언
uniform vec3 lightPos; // 광원 위치 > 조명벡터 계산에서 사용
uniform vec3 viewPos; // 카메라 위치 > 뷰 벡터 계산에서 사용
uniform vec3 objectColor; // 물체가 반사할 색상값(= 물체의 색상)
uniform vec3 lightColor; // 광원의 색상값 (= 조명 색상)

void main() {
  /* ambient 성분 계산 */
  float ambientStrength = 0.1; // ambient 강도
  vec3 ambient = ambientStrength * lightColor; // ambient 강도에 조명색상 곱해서 최종 ambient 성분값 계산 (원래는 각 성분마다 색상을 별도 지정할 수 있어야 함.)

  /* diffuse 성분 계산 */
  // 조명계산에 사용되는 모든 방향벡터들은 항상 정규화를 해줄 것! -> 그래야 내적계산 시 정확한 cos 값만 얻을 수 있음!
  vec3 norm = normalize(Normal); // 프래그먼트에 수직인 노멀벡터
  vec3 lightDir = normalize(lightPos - FragPos); // 조명벡터 (프래그먼트 위치 ~ 광원 위치)
  float diff = max(dot(norm, lightDir), 0.0); // 노멀벡터와 조명벡터 내적 > diffuse 성분의 세기(조도) 계산 (참고로, 음수인 diffuse 값은 조명값 계산을 부정확하게 만들기 때문에, 0.0 으로 clamping 시킴)
  vec3 diffuse = diff * lightColor; // diffuse 조도에 조명색상 곱해서 최종 diffuse 성분값 계산

  /* specular 성분 계산 */
  float specularStrength = 0.5; // specular 강도
  vec3 viewDir = normalize(viewPos - FragPos); // 뷰 벡터 (카메라 위치 ~ 프래그먼트 위치)
  vec3 reflectDir = reflect(-lightDir, norm); // 반사 벡터 (조명벡터는 카메라 위치부터 출발하도록 방향을 negate)
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32); // 뷰 벡터와 반사 벡터의 내적값을 32제곱 > 32는 shininess 값으로써, 값이 클수록 highlight 영역이 정반사되고, 값이 작을수록 난반사됨. > specular 조도 계산
  vec3 specular = specularStrength * spec * lightColor; // specular 조도에 specular 강도와 조명색상을 곱해 specular 성분값 계산

  // 3가지 성분을 모두 더한 조명 색상에 물체 색상을 component-wise 곱으로 계산하여 최종 색상 결정
  vec3 result = (ambient + diffuse + specular) * objectColor;
  FragColor = vec4(result, 1.0);
}

/*
  본문에 따르면, 
  
  objectColor * lightColor 값이 
  물체가 반사하는 빛의 양이고, 

  lightColor - (objectColor * lightColor) 값이 
  물체가 흡수하는 빛의 양이 나온다고 함.
*/