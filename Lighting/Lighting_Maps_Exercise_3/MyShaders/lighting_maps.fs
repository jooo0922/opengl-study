#version 330 core

out vec4 FragColor;

// 서로 연관된 uniform 변수들을 묶어줄 구조체 선언
// Material 구조체 선언 (물체가 반사할 색상)
struct Material {
  sampler2D diffsue; // 물체가 diffuse 및 ambient 조명에 대해 반사하는 색상을 diffuseMap 텍스쳐에서 샘플링해와서 계산할 것임!
  sampler2D specular; // 물체가 specular 조명에 대해 반사하는 색상을 specularMap 텍스쳐에서 샘플링해와서 계산할 것임!
  sampler2D emission; // 물체가 발산하는 빛의 색상을 emissionMap 텍스쳐에서 샘플링해와서 계산할 것임!
  float shininess;
};

// Light 구조체 선언 (조명 위치 및 색상)
struct Light {
  vec3 position;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

// 버텍스 쉐이더에서 전송된 보간 변수
in vec3 Normal; // 월드공간 노멀벡터
in vec3 FragPos; // 월드공간 프래그먼트 위치값
in vec2 TexCoords; // 보간된 프래그먼트 uv 좌표값

// OpenGL 에서 전송해 줄 uniform 변수들 선언
uniform vec3 viewPos; // 카메라 위치 > 뷰 벡터 계산에서 사용
uniform Material material; // Material 구조체 (각 멤버변수마다 uniform 값 전송 가능)
uniform Light light; // Light 구조체 (각 멤버변수마다 uniform 값 전송 가능)

void main() {
  /* ambient 성분 계산 */
  vec3 ambient = light.ambient * texture2D(material.diffsue, TexCoords).rgb; // (ambient 강도 * 물체가 ambient 에 대해 반사하는 색상) 으로 최종 ambient 성분값 계산 (원래는 각 성분마다 색상을 별도 지정할 수 있어야 함.)

  /* diffuse 성분 계산 */
  // 조명계산에 사용되는 모든 방향벡터들은 항상 정규화를 해줄 것! -> 그래야 내적계산 시 정확한 cos 값만 얻을 수 있음!
  vec3 norm = normalize(Normal); // 프래그먼트에 수직인 노멀벡터
  vec3 lightDir = normalize(light.position - FragPos); // 조명벡터 (프래그먼트 위치 ~ 광원 위치)
  float diff = max(dot(norm, lightDir), 0.0); // 노멀벡터와 조명벡터 내적 > diffuse 성분의 세기(조도) 계산 (참고로, 음수인 diffuse 값은 조명값 계산을 부정확하게 만들기 때문에, 0.0 으로 clamping 시킴)
  vec3 diffuse = light.diffuse * (diff * texture2D(material.diffsue, TexCoords).rgb); // diffuse 조명 색상 * (diffuse 조도 * 물체가 diffuse 에 대해 반사하는 색상) 으로 최종 diffuse 성분값 계산

  /* specular 성분 계산 */
  vec3 viewDir = normalize(viewPos - FragPos); // 뷰 벡터 (카메라 위치 ~ 프래그먼트 위치)
  vec3 reflectDir = reflect(-lightDir, norm); // 반사 벡터 (조명벡터는 카메라 위치부터 출발하도록 방향을 negate)
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); // 뷰 벡터와 반사 벡터의 내적값을 32제곱 > 32는 shininess 값으로써, 값이 클수록 highlight 영역이 정반사되고, 값이 작을수록 난반사됨. > specular 조도 계산
  vec3 specular = light.specular * (spec * texture2D(material.specular, TexCoords).rgb); // specular 조명색상 * (specular 조도 * 물체가 specular 에 대해 반사하는 색상) 으로 specular 성분값 계산

  /* emission 성분 계산 */
  vec3 emission = texture2D(material.emission, TexCoords).rgb;

  // 3가지 성분을 모두 더한 조명 색상에 물체 색상을 component-wise 곱으로 계산하여 최종 색상 결정
  vec3 result = ambient + diffuse + specular + emission;
  FragColor = vec4(result, 1.0);
}

/*
  본문에 따르면, 
  
  objectColor * lightColor 값이 
  물체가 반사하는 빛의 양이고, 

  lightColor - (objectColor * lightColor) 값이 
  물체가 흡수하는 빛의 양이 나온다고 함.
*/