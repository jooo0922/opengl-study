#version 330 core

// layout location specifier 로 MRT 프레임버퍼에 바인딩된 각 color attachment(G-buffer) 에 대응되는 출력 변수 선언
// ex> location = 0 으로 지정 시, 해당 출력 변수에 입력되는 색상은 GL_COLOR_ATTACHMENT0 버퍼에 저장됨.
layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gAlbedo;

/* vertex shader 단계에서 전달받는 입력 변수 선언 */
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

void main() {
  // gPosition 텍스쳐 버퍼에는 프래그먼트의 view space 위치값 저장 
  gPosition = FragPos;

  // gNormal 텍스쳐 버퍼에는 프래그먼트의 view space 노멀벡터 저장
  gNormal = normalize(Normal);

  // gAlbedo 텍스쳐 버퍼의 .rgb 성분에는 임의의 white color (vec3(0.95)) 저장
  /*
    어차피 이번 예제에서 사용하는 G-Buffer 는 
    SSAO occlusion factor 계산에 사용될 뿐이므로,
    굳이 Albedo 정보를 G-Buffer 에 담아둘 필요가 없음.
  */
  gAlbedo.rgb = vec3(0.95);
}