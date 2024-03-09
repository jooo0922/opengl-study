#version 330 core

// layout location specifier 로 MRT 프레임버퍼에 바인딩된 각 color attachment 에 대응되는 출력 변수 선언
// ex> location = 0 으로 지정 시, 해당 출력 변수에 입력되는 색상은 GL_COLOR_ATTACHMENT0 버퍼에 저장됨.
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

/* vertex shader 단계에서 전달받는 입력 interface block 선언 */
in VS_OUT {
  vec3 FragPos;
  vec3 Normal;
  vec2 TexCoords;
} fs_in;

/* 각 조명의 정보를 저장할 구조체 선언 */
struct Light {
  vec3 Position; // 조명 위치
  vec3 Color; // 조명 색상
};

/* 
  OpenGL 에서 전송해 줄 uniform 변수들 선언 
*/

// 4개의 조명 정보를 전송받은 정적 배열 선언
uniform Light lights[4]; 

// diffuse 텍스쳐 (0번 texture unit 에 바인딩된 텍스쳐 객체 샘플링)
uniform sampler2D diffuseTexture;

// 카메라 위치값
uniform vec3 viewPos;

void main() {
  // diffuse 텍스쳐 샘플링
  vec3 color = texture2D(diffuseTexture, fs_in.TexCoords).rgb;

  // 보간된 월드 공간 노멀벡터의 길이를 1로 맞춤
  vec3 normal = normalize(fs_in.Normal);

  // ambient 성분값(환경광)을 미리 계산하여 캐싱해 둠.
  vec3 ambient = 0.0 * color;

  // 각 조명을 순회하며 계산된 조명값들을 누산할 변수 초기화
  vec3 lighting = vec3(0.0);

  // 각 조명을 순회하며 조명값 계산 및 누산
  for(int i = 0; i < 4; i++) {
    /* diffuse 성분값 계산 */

    // 조명벡터 (프래그먼트 위치 ~ 광원 위치)
    vec3 lightDir = normalize(lights[i].Position - fs_in.FragPos);

    // 노멀벡터와 조명벡터 내적 > diffuse 성분의 세기(조도) 계산 (참고로, 음수인 diffuse 값은 조명값 계산을 부정확하게 만들기 때문에, 0.0 으로 clamping 시킴)
    float diff = max(dot(lightDir, normal), 0.0);

    // diffuse 조도 * 조명 색상 * diffuseMap 색상을 곱해서 최종 diffuse 성분값 계산
    vec3 diffuse = lights[i].Color * diff * color;
    vec3 result = diffuse;

    /* 조명으로부터의 거리에 따른 attenuation(감쇄) 계산 */

    // 조명 ~ 각 프래그먼트 위치 사이의 거리값 계산
    float distance = length(fs_in.FragPos - lights[i].Position);

    // 물리적으로 정확한 계산을 위해, 거리 제곱의 반비례로 감쇄 계산
    /*
      gamma correction 활성화할 것이므로,
      감쇄 계산 시, '거리 제곱에' 반비례하게 계산함,
      
      이렇게 하는 이유는,

      어차피 CRT 모니터에 의해서
      최종 색상 출력 시 2.2 제곱이 적용될 것이고,

      gamma correction 이 활성화되면,
      프래그먼트 쉐이더에서 최종 색상에는 1 / 2.2 제곱이 적용되기 때문에,
      사실상 두 거듭제곱 값이 상쇄되어 버림.

      따라서, 감쇄가 거리 제곱에 반비례하도록 계산하려면
      거리값을 거듭제곱 해줘야 함.
    */
    result *= 1.0 / (distance * distance);

    // 각 조명의 최종 결과값 누산
    lighting += result;
  }

  // 최종 누산된 조명값에 환경광을 더해서 최종 색상 계산
  vec3 result = ambient + lighting;

  // 최종 색상을 grayscale 로 변환하여 해당 색상의 밝기값 계산
  // https://github.com/jooo0922/opengl-study/blob/main/AdvancedOpenGL/Framebuffers/MyShaders/framebuffers_screen_grayscale.fs 참고!
  // 위 쉐이더 코드에서의 grayscale 계산을 내적 계산(dot)으로 단순화한 것 뿐임!
  float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));

  // BrightColor 출력 변수(= 즉, GL_COLOR_ATTACHMENT1 텍스쳐 객체)에는 특정 밝기값 이상의 색상만 저장함. 
  if(brightness > 1.0) {
    // 특정 밝기값 threshold 를 넘는 색상만 BrightColor 출력 변수에 저장
    BrightColor = vec4(FragColor.rgb, 1.0);
  } else {
    // 특정 밝기값 threshold 를 넘지 못하는 색상들은 모두 검정색으로 BrightColor 출력 변수에 저장
    BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
  }

  // FragColor 출력 변수(= 즉, GL_COLOR_ATTACHMENT0 텍스쳐 객체)에는 씬에 렌더링되는 모든 색상을 저장함.
  FragColor = vec4(ambient + lighting, 1.0);
}