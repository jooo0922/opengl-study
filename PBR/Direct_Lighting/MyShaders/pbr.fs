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

// 광원 정보를 전송받는 uniform 변수 선언
uniform vec3 lightPositions[1];
uniform vec3 lightColors[1];

// 카메라 위치값을 전송받는 uniform 변수 선언
uniform vec3 camPos;

// Pi 상수 선언
const float PI = 3.14159265359;

void main() {
  /* PBR Material 파라미터 값을 각 텍스쳐들로부터 샘플링 */

  // albedo (물체의 diffuse 반사 색상값을 rgb 채널로 저장한 값) 샘플링 -> 샘플링된 texel 값 2.2 제곱 이유 하단 필기 
  vec3 albedo = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));

  // metallic 샘플링
  float metallic = texture(metallicMap, TexCoords).r;

  // roughness 샘플링
  float roughness = texture(roughnessMap, TexCoords).r;

  // ambient occlusion factor 샘플링
  /*
    https://github.com/jooo0922/opengl-study/blob/main/AdvancedLighting/SSAO/MyShaders/ssao_blur.fs
    
    위 SSAO 쉐이더 예제 코드에서 사용했던 SSAO occlusion factor 가 렌더링된 텍스쳐 버퍼와 동일한 역할.
    다만, 직접 attach 된 텍스쳐 버퍼에 렌더링된 것을 사용하느냐, 기존 이미지 텍스쳐를 사용하느냐의 차이!
  */
  float ao = texture(aoMap, TexCoords).r;
}

/*
  샘플링된 albedo 색상값을 2.2 제곱 처리하는 이유


  보통 디자이너가 텍스쳐 이미지를 작업할 때, 
	이미 gamma correction(1/2.2 제곱) 이 적용된 상태에서
	이미지를 작업하는 경우가 많은데, 이를 'sRGB 색 공간이 적용된 텍스쳐' 라고 함.

	주로 diffuse, albedo 텍스쳐처럼,
	물체의 난반사된 색상을 표현하는 텍스쳐들은
	sRGB 색 공간으로 지정해놓고 작업하는 경우가 많음.

	그러나, gamma correction 이 적용되면 전체적으로 색상값이 밝아지기 때문에,

	쉐이더 객체에서 이미 gamma correction 이 적용된 텍스쳐를
	샘플링해서 다시 gamma correction (1/2.2 제곱) 을 적용해버리면

	결과적으로 gamma correction 이 두 번 적용됨으로써,
	텍스쳐 영역의 최종 색상이 과하게 밝아지는 문제가 발생함.

	이를 해결하기 위해,
	OpenGL 내부에서 자체적으로 텍스쳐 이미지 데이터를 저장할 때,
  internal format 을 GL_SRGB 또는 GL_SRGB_ALPHA 로 설정하면,

	"이 텍스쳐 데이터는 이미 sRGB 감마 보정이 적용되어 있으니, 
	linear space 색 공간으로 변환해서 저장해주세요"

	라고 명령하는 것과 같음.

  https://github.com/jooo0922/opengl-study/blob/main/AdvancedLighting/Gamma_Correction/gamma_correction.cpp 참고

  그러나, PBR 예제 코드는 Gamma Correction 예제 코드와 달리,
  loadTexture() 함수에서 internal format 을 GL_SRGB 로 저장하는 로직이 빠져있음.

  그래서, 이미 gamma correction 이 적용된 albedo 텍스쳐로부터 샘플링된 텍셀값에
  직접 2.2 제곱하여 linear space 색 공간으로 직접 변환하는 거라고 보면 됨. 

*/