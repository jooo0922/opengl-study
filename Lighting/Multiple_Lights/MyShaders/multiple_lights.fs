#version 330 core

out vec4 FragColor;

// 서로 연관된 uniform 변수들을 묶어줄 구조체 선언
// Material 구조체 선언 (물체가 반사할 색상)
struct Material {
  sampler2D diffsue; // 물체가 diffuse 및 ambient 조명에 대해 반사하는 색상을 diffuseMap 텍스쳐에서 샘플링해와서 계산할 것임!
  sampler2D specular; // 물체가 specular 조명에 대해 반사하는 색상을 specularMap 텍스쳐에서 샘플링해와서 계산할 것임!
  float shininess;
};

/* Directional Light 구조체 선언 */
struct DirectionalLight {
  // vec3 position; // Directional Light 는 광원의 위치를 기준으로 조명벡터를 계산하지 않음!

   // Directional Light 는 광원의 위치와 무관하게 모든 프래그먼트에 평행한 방향으로 입사하는 조명을 계산하므로, 
   // 모든 조명벡터에 대해 '동일한 방향벡터'를 사용함.
   // 이때, 광원에서 프래그먼트 방향으로 입사되는 방향벡터를 입력받기 때문에, 실제 조명계산 시, negate 해줘야 함!
  vec3 direction;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

/* Point Light 구조체 선언 */
struct PointLight {
  vec3 position; // Point Light 광원 위치

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  // 감쇄 계산에 사용할 상수값 선언
  float constant; // Kc
  float linear; // Kl (linear term 상수)
  float quadratic; // Kq (quadratic term 상수)
};

/* Spot Light 구조체 선언 */
struct SpotLight {
  vec3 position; // Spot Light 광원 위치 (= 카메라 위치)
  vec3 direction; // Spot Light 방향 벡터 (= 카메라 앞쪽 방향벡터. 이하 SpotDir)
  float cutOff; // Spot Light 의 Inner Cone 영역의 최대 각도 cos 값
  float outerCutOff; // Spot Light 의 Outer Cone 영역의 최대 각도 cos 값

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  // 감쇄 계산에 사용할 상수값 선언
  float constant; // Kc
  float linear; // Kl (linear term 상수)
  float quadratic; // Kq (quadratic term 상수)
};

// Point Light 개수를 매크로 전처리기로 선언 > 컴파일러가 해당 매크로를 마주치면 4로 치환해 줌.
#define NR_POINTS_LIGHTS 4

// 버텍스 쉐이더에서 전송된 보간 변수
in vec3 Normal; // 월드공간 노멀벡터
in vec3 FragPos; // 월드공간 프래그먼트 위치값
in vec2 TexCoords; // 보간된 프래그먼트 uv 좌표값

// OpenGL 에서 전송해 줄 uniform 변수들 선언
uniform vec3 viewPos; // 카메라 위치 > 뷰 벡터 계산에서 사용
uniform DirectionalLight dirLight; // DirectionalLight 구조체 변수 선언 (각 멤버변수마다 uniform 값 전송 가능)
uniform PointLight pointLights[NR_POINTS_LIGHTS]; // PointLight 구조체 정적 배열 변수 선언 (각 멤버변수마다 uniform 값 전송 가능)
uniform SpotLight spotLight; // SpotLight 구조체 변수 선언 (각 멤버변수마다 uniform 값 전송 가능)
uniform Material material; // Material 구조체 변수 선언 (각 멤버변수마다 uniform 값 전송 가능)

// 각 light caster 타입별 조명계산 함수 전방선언
vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {
  // 각 light caster 타입별 조명계산 함수에 공통적으로 전달할 벡터들을 미리 계산
  vec3 norm = normalize(Normal); // 프래그먼트에 수직인 노멀벡터
  vec3 viewDir = normalize(viewPos - FragPos); // 뷰 벡터 (카메라 위치 ~ 프래그먼트 위치)

  /* Directional Light 에 의한 조명값 계산 결과를 최종 아웃풋 color 에 누산 */
  vec3 result = CalcDirLight(dirLight, norm, viewDir);

  /* 4개의 PointLight 구조체 배열을 순회하면서 조명값 계산 결과를 최종 아웃풋 color 에 누산 */
  for(int i = 0; i < NR_POINTS_LIGHTS; i++) {
    result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
  }

  /* Spot Light 에 의한 조명값 계산 결과를 최종 아웃풋 color 에 누산 */
  result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

  // 각 light caster 타입별 조명계산 결과가 누산된 색상값 result 를 최종 아웃풋 색상으로 출력
  FragColor = vec4(result, 1.0);
}

// Directional Light 에 의한 조명값 계산 함수 encapsulation
vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir) {
  vec3 lightDir = normalize(-light.direction); // 조명벡터 (Directional Light 구조체에 정의된 동일한 방향으로 평행하게 입사하는 방향벡터)

  /* diffuse 성분 계산 */
  // 조명계산에 사용되는 모든 방향벡터들은 항상 정규화를 해줄 것! -> 그래야 내적계산 시 정확한 cos 값만 얻을 수 있음!
  float diff = max(dot(normal, lightDir), 0.0); // 노멀벡터와 조명벡터 내적 > diffuse 성분의 세기(조도) 계산 (참고로, 음수인 diffuse 값은 조명값 계산을 부정확하게 만들기 때문에, 0.0 으로 clamping 시킴)

  /* specular 성분 계산 */
  vec3 reflectDir = reflect(-lightDir, normal); // 반사 벡터 (조명벡터는 카메라 위치부터 출발하도록 방향을 negate)
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); // 뷰 벡터와 반사 벡터의 내적값을 32제곱 > 32는 shininess 값으로써, 값이 클수록 highlight 영역이 정반사되고, 값이 작을수록 난반사됨. > specular 조도 계산

  // 계산해 둔 각 성분값에 입력받은 조명값과 light map 에서 샘플링한 값을 적용
  vec3 ambient = light.ambient * texture2D(material.diffsue, TexCoords).rgb; // (ambient 강도 * 물체가 ambient 에 대해 반사하는 색상) 으로 최종 ambient 성분값 계산 (원래는 각 성분마다 색상을 별도 지정할 수 있어야 함.)
  vec3 diffuse = light.diffuse * (diff * texture2D(material.diffsue, TexCoords).rgb); // diffuse 조명 색상 * (diffuse 조도 * 물체가 diffuse 에 대해 반사하는 색상) 으로 최종 diffuse 성분값 계산
  vec3 specular = light.specular * (spec * texture2D(material.specular, TexCoords).rgb); // specular 조명색상 * (specular 조도 * 물체가 specular 에 대해 반사하는 색상) 으로 specular 성분값 계산

  // 3가지 성분을 모두 component-wise 합으로 계산하여 최종 색상 결정
  return (ambient + diffuse + specular);
}

// Point Light 에 의한 조명값 계산 함수 encapsulation
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
  vec3 lightDir = normalize(light.position - fragPos); // 조명벡터

  /* diffuse 성분 계산 */
  // 조명계산에 사용되는 모든 방향벡터들은 항상 정규화를 해줄 것! -> 그래야 내적계산 시 정확한 cos 값만 얻을 수 있음!
  float diff = max(dot(normal, lightDir), 0.0); // 노멀벡터와 조명벡터 내적 > diffuse 성분의 세기(조도) 계산 (참고로, 음수인 diffuse 값은 조명값 계산을 부정확하게 만들기 때문에, 0.0 으로 clamping 시킴)

  /* specular 성분 계산 */
  vec3 reflectDir = reflect(-lightDir, normal); // 반사 벡터 (조명벡터는 카메라 위치부터 출발하도록 방향을 negate)
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); // 뷰 벡터와 반사 벡터의 내적값을 32제곱 > 32는 shininess 값으로써, 값이 클수록 highlight 영역이 정반사되고, 값이 작을수록 난반사됨. > specular 조도 계산

  /* attenuation (감쇄) 계산 */
  float distance = length(light.position - fragPos); // Point Light 광원에서 각 프래그먼트 사이의 거리
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); // 거리에 따라 지수적으로 감소하는 감쇄값 계산

  // 계산해 둔 각 성분값에 입력받은 조명값과 light map 에서 샘플링한 값을 적용
  vec3 ambient = light.ambient * texture2D(material.diffsue, TexCoords).rgb; // (ambient 강도 * 물체가 ambient 에 대해 반사하는 색상) 으로 최종 ambient 성분값 계산 (원래는 각 성분마다 색상을 별도 지정할 수 있어야 함.)
  vec3 diffuse = light.diffuse * (diff * texture2D(material.diffsue, TexCoords).rgb); // diffuse 조명 색상 * (diffuse 조도 * 물체가 diffuse 에 대해 반사하는 색상) 으로 최종 diffuse 성분값 계산
  vec3 specular = light.specular * (spec * texture2D(material.specular, TexCoords).rgb); // specular 조명색상 * (specular 조도 * 물체가 specular 에 대해 반사하는 색상) 으로 specular 성분값 계산

  // 계산해 둔 조명 성분에 감쇄 적용
  ambient *= attenuation;
  diffuse *= attenuation;
  specular *= attenuation;

  // 3가지 성분을 모두 component-wise 합으로 계산하여 최종 색상 결정
  return (ambient + diffuse + specular);
}

// Spot Light 에 의한 조명값 계산 함수 encapsulation
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
  vec3 lightDir = normalize(light.position - fragPos); // 조명벡터

  /* diffuse 성분 계산 */
  // 조명계산에 사용되는 모든 방향벡터들은 항상 정규화를 해줄 것! -> 그래야 내적계산 시 정확한 cos 값만 얻을 수 있음!
  float diff = max(dot(normal, lightDir), 0.0); // 노멀벡터와 조명벡터 내적 > diffuse 성분의 세기(조도) 계산 (참고로, 음수인 diffuse 값은 조명값 계산을 부정확하게 만들기 때문에, 0.0 으로 clamping 시킴)

  /* specular 성분 계산 */
  vec3 reflectDir = reflect(-lightDir, normal); // 반사 벡터 (조명벡터는 카메라 위치부터 출발하도록 방향을 negate)
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); // 뷰 벡터와 반사 벡터의 내적값을 32제곱 > 32는 shininess 값으로써, 값이 클수록 highlight 영역이 정반사되고, 값이 작을수록 난반사됨. > specular 조도 계산

  /* attenuation (감쇄) 계산 */
  float distance = length(light.position - fragPos); // Point Light 광원에서 각 프래그먼트 사이의 거리
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); // 거리에 따라 지수적으로 감소하는 감쇄값 계산

  /* Spot Light Intensity 계산 (Spot Light 의 경계선을 soft edge 로 렌더링하려는 목적) */
  float theta = dot(lightDir, normalize(-light.direction)); // SpotDir 방향벡터와 각 프래그먼트까지의 조명벡터 사이의 각도 계산 -> 내적계산이므로 결과값은 cos
  float epsilon = light.cutOff - light.outerCutOff; // Outer Cone 최대 각도와 Inner Cone 최대 각도의 사잇각 계산 -> 두 각도는 cos 로 받으므로 계산결과는 cos
  float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0); // Outer Cone 최대 각도와 조명벡터 각도의 사잇각 / epsilon -> Inner Cone 과 Outer Cone 사이에서 부드럽게 보간됨.

  // 계산해 둔 각 성분값에 입력받은 조명값과 light map 에서 샘플링한 값을 적용
  vec3 ambient = light.ambient * texture2D(material.diffsue, TexCoords).rgb; // (ambient 강도 * 물체가 ambient 에 대해 반사하는 색상) 으로 최종 ambient 성분값 계산 (원래는 각 성분마다 색상을 별도 지정할 수 있어야 함.)
  vec3 diffuse = light.diffuse * (diff * texture2D(material.diffsue, TexCoords).rgb); // diffuse 조명 색상 * (diffuse 조도 * 물체가 diffuse 에 대해 반사하는 색상) 으로 최종 diffuse 성분값 계산
  vec3 specular = light.specular * (spec * texture2D(material.specular, TexCoords).rgb); // specular 조명색상 * (specular 조도 * 물체가 specular 에 대해 반사하는 색상) 으로 specular 성분값 계산

  // 계산해 둔 조명 성분에 감쇄 적용
  // + Spot Light Intensity 를 조명 성분에 적용
  /*
    Outer Cone 바깥은 0으로 clamping 되고,
    Outer Cone 과 Inner Cone 사이는 0 ~ 1 사이로 부드럽게 보간되고,
    Inner Cone 안쪽은 1로 clamping 됨.

    -> 결과적으로 Spot Light 경계선이 부드럽게 보간되는 효과
  */
  ambient *= attenuation;
  diffuse *= attenuation * intensity;
  specular *= attenuation * intensity;

  // 3가지 성분을 모두 component-wise 합으로 계산하여 최종 색상 결정
  return (ambient + diffuse + specular);
}

/*
  본문에 따르면, 
  
  objectColor * lightColor 값이 
  물체가 반사하는 빛의 양이고, 

  lightColor - (objectColor * lightColor) 값이 
  물체가 흡수하는 빛의 양이 나온다고 함.
*/

/*
  attenuation (감쇄)

  점 광원에서 멀어질수록 지수적으로 감소하는 조명을 계산하기 위한 값
  
  분자는 1이고,
  분모는 아래 세 개의 항으로 이루어져 있음.

  1. constant term (Kc) : 분모가 1보다 작아지는 것을 방지하기 위해 더해주는 상수항
  2. linear term (Kl * d) : 거리에 따라 선형적으로 증가하는 값 -> 이 값이 클수록 감쇄의 선형성이 증가
  3. quadratic term (Kq * (d * d)) : 거리에 따라 지수적으로 증가하는 값 -> 이 값이 클수록 감쇄가 이차함수 그래프에 가까워 짐.
*/