#version 330 core

// layout location specifier 로 MRT 프레임버퍼에 바인딩된 각 color attachment(G-buffer) 에 대응되는 출력 변수 선언
// ex> location = 0 으로 지정 시, 해당 출력 변수에 입력되는 색상은 GL_COLOR_ATTACHMENT0 버퍼에 저장됨.
layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;

/* vertex shader 단계에서 전달받는 입력 변수 선언 */
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// Assimp 를 추상화한 Model 클래스에서 전송한 텍스쳐 객체의 sampler 변수 선언
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main() {
  // gPosition 텍스쳐 버퍼에는 프래그먼트의 월드 공간 위치값 저장 
  gPosition = FragPos;

  // gNormal 텍스쳐 버퍼에는 프래그먼트의 월드 공간 노멀벡터 저장
  gNormal = normalize(Normal);

  // gAlbedoSpec 텍스쳐 버퍼의 .rgb 성분에는 diffuse texture 에서 샘플링한 색상값 저장
  gAlbedoSpec.rgb = texture2D(texture_diffuse1, TexCoords).rgb;

  // gAlbedoSpec 텍스쳐 버퍼의 .a 성분에는 specular texture 에서 샘플링한 specular intensity 값 저장
  gAlbedoSpec.a = texture2D(texture_specular1, TexCoords).r;
}