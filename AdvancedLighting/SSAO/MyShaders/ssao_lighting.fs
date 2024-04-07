#version 330 core

// 최종 색상을 할당할 출력 변수 선언
out vec4 FragColor;

// vertex shader 단계에서 전달되면서 보간된 텍스쳐 좌표 입력 변수 선언
in vec2 TexCoords;

// Geometry pass 에서 저장한 geometry data 가 담긴 각 G-buffer 의 sampler 변수 선언
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;

// SSAO Blur 처리까지 완료된 텍스쳐 버퍼의 sampler 변수 선언
uniform sampler2D ssao;

/* 조명의 정보를 저장할 구조체 선언 */
struct Light {
  vec3 Position; // 조명 위치
  vec3 Color; // 조명 색상

  float Linear; // 거리에 따른 조명 감쇄 계산식에서 사용할 Linear 항의 계수
  float Quadratic; // 거리에 따른 조명 감쇄 계산식에서 사용할 Quadratic 항의 계수
};

/* 
  OpenGL 에서 전송해 줄 uniform 변수들 선언 
*/

// 조명 정보 구조체 변수 선언
uniform Light light;

void main() {
  /* G-buffer 로부터 geometry data 가져오기 */

  // G-buffer 로부터 현재 pixel 의 월드 공간 position 값 샘플링
  vec3 FragPos = texture(gPosition, TexCoords).rgb;

  // G-buffer 로부터 현재 pixel 의 월드 공간 normal 값 샘플링
  vec3 Normal = texture(gNormal, TexCoords).rgb;

  // G-buffer 로부터 현재 pixel 에 적용할 Diffuse 색상값 샘플링
  vec3 Diffuse = texture(gAlbedo, TexCoords).rgb;

  // SSAO Blur 처리된 텍스쳐 버퍼로부터 occlusion factor 샘플링
  float AmbientOcclusion = texture(ssao, TexCoords).r;

  /* G-buffer 에서 가져온 데이터로 조명 계산 */

  // ambient(환경광) 성분을 Diffuse 색상으로 계산하여 초기화함.
  // 이때, SSAO 텍스쳐 버퍼에서 샘플링한 occlusion factor 로 ambient 성분의 세기 조정
  vec3 ambient = vec3(0.3 * Diffuse * AmbientOcclusion);

  // 조명 연산 결과값을 ambient 성분으로 초기화
  vec3 lighting = ambient;

  // 뷰 벡터 (프래그먼트 위치 ~ 카메라 위치) 계산
  /*
    이 예제에서 사용되는 G-buffer 의 모든 값들은
    카메라가 원점인 view space 를 기준으로 저장되어 있으므로,

    이 좌표계에서의 프래그먼트 위치값은 곧
    카메라 ~ 프래그먼트 사이의 view vector 와 같다고도 볼 수 있겠지!
  */
  vec3 viewDir = normalize(-FragPos);

  /* diffuse 성분값 계산 */

  // 조명벡터 (프래그먼트 위치 ~ 광원 위치)
  vec3 lightDir = normalize(light.Position - FragPos);

  // 노멀벡터와 조명벡터 내적 > diffuse 성분의 세기(조도) 계산 (참고로, 음수인 diffuse 값은 조명값 계산을 부정확하게 만들기 때문에, 0.0 으로 clamping 시킴)
  // diffuse 성분값 * 모델의 원 색상(Diffuse) * 조명 색상
  vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * light.Color;

  /* specular 성분값 계산 */

  // blinn-phong half vector 계산
  vec3 halfwayDir = normalize(lightDir + viewDir);

  // spec 성분값 계산 (shininess 8 거듭제곱)
  float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);

  // 조명 색상 * specular 성분값
  vec3 specular = light.Color * spec;

  /* 거리에 따른 감쇄 계산 */

  // 광원으로부터의 거리값 계산
  float distance = length(light.Position - FragPos);

  // 거리에 따른 감쇄량 계산
  // 감쇄 계산 공식 관련 https://github.com/jooo0922/opengl-study/blob/main/Lighting/Light_Casters_2/MyShaders/light_casters.fs 참고
  float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distance * distance);

  // diffuse 성분과 specular 성분 각각에 감쇄 적용
  diffuse *= attenuation;
  specular *= attenuation;

  // diffuse 성분값과 specular 성분값을 더해 최종 결과값에 누산
  lighting += diffuse + specular;

  // FragColor 출력 변수에 누산된 최종 색상값 할당
  FragColor = vec4(lighting, 1.0);
}