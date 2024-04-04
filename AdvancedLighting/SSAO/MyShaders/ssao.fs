#version 330 core

// 최종 색상을 할당할 출력 변수 선언
out vec4 FragColor;

// vertex shader 단계에서 전달되면서 보간된 텍스쳐 좌표 입력 변수 선언
in vec2 TexCoords;

// Geometry pass 에서 저장한 geometry data 가 담긴 각 G-buffer 의 sampler 변수 선언
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

/* 각 조명의 정보를 저장할 구조체 선언 */
struct Light {
  vec3 Position; // 조명 위치
  vec3 Color; // 조명 색상

  float Linear; // 거리에 따른 조명 감쇄 계산식에서 사용할 Linear 항의 계수
  float Quadratic; // 거리에 따른 조명 감쇄 계산식에서 사용할 Quadratic 항의 계수
  float Radius; // light volume 의 반경
};

/* 
  OpenGL 에서 전송해 줄 uniform 변수들 선언 
*/

// 조명 개수를 상수로 선언
const int NR_LIGHTS = 32;

// NR_LIGHTS 개의 조명 정보를 전송받은 정적 배열 선언
uniform Light lights[NR_LIGHTS]; 

// 카메라 위치값
uniform vec3 viewPos;

void main() {
  /* G-buffer 로부터 geometry data 가져오기 */

  // G-buffer 로부터 현재 pixel 의 월드 공간 position 값 샘플링
  vec3 FragPos = texture(gPosition, TexCoords).rgb;

  // G-buffer 로부터 현재 pixel 의 월드 공간 normal 값 샘플링
  vec3 Normal = texture(gNormal, TexCoords).rgb;

  // G-buffer 로부터 현재 pixel 에 적용할 Diffuse 색상값 샘플링
  vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;

  // G-buffer 로부터 현재 pixel 에 적용할 Specular intensity 값 샘플링
  float Specular = texture(gAlbedoSpec, TexCoords).a;

  /* G-buffer 에서 가져온 데이터로 조명 계산 */

  // 각 조명을 순회하며 계산된 조명값들을 누산할 변수 초기화
  // ambient(환경광) 성분을 Diffuse 색상으로 계산하여 초기화함.
  vec3 lighting = Diffuse * 0.1;

  // 뷰 벡터 (프래그먼트 위치 ~ 카메라 위치) 계산
  vec3 viewDir = normalize(viewPos - FragPos);

  // 각 조명을 순회하며 조명값 계산 및 누산
  for(int i = 0; i < NR_LIGHTS; i++) {
    // 광원으로부터의 거리값 계산
    float distance = length(lights[i].Position - FragPos);

    // light volume 내에 들어오는 프래그먼트에 대해서만 조명 연산 수행 -> light volume 을 활용한 최적화!
    if(distance < lights[i].Radius) {
    /* diffuse 성분값 계산 */

    // 조명벡터 (프래그먼트 위치 ~ 광원 위치)
      vec3 lightDir = normalize(lights[i].Position - FragPos);

    // 노멀벡터와 조명벡터 내적 > diffuse 성분의 세기(조도) 계산 (참고로, 음수인 diffuse 값은 조명값 계산을 부정확하게 만들기 때문에, 0.0 으로 clamping 시킴)
    // diffuse 성분값 * 모델의 원 색상(Diffuse) * 조명 색상
      vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lights[i].Color;

    /* specular 성분값 계산 */

    // blinn-phong half vector 계산
      vec3 halfwayDir = normalize(lightDir + viewDir);

    // spec 성분값 계산 (shininess 16 거듭제곱)
      float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);

    // 조명 색상 * specular 성분값 * specular intensity
      vec3 specular = lights[i].Color * spec * Specular;

    /* 거리에 따른 감쇄 계산 */

    // 거리에 따른 감쇄량 계산
    // 감쇄 계산 공식 관련 https://github.com/jooo0922/opengl-study/blob/main/Lighting/Light_Casters_2/MyShaders/light_casters.fs 참고
      float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);

    // diffuse 성분과 specular 성분 각각에 감쇄 적용
      diffuse *= attenuation;
      specular *= attenuation;

    // diffuse 성분값과 specular 성분값을 더해 최종 결과값에 누산
      lighting += diffuse + specular;
    }
  }

  // FragColor 출력 변수에 누산된 최종 색상값 할당
  FragColor = vec4(lighting, 1.0);
}