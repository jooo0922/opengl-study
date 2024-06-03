#version 330 core

// 프래그먼트 쉐이더 출력 변수 선언
out vec4 FragColor;

// vertex shader 단계에서 전달받는 입력 변수 선언
in vec3 WorldPos;

// 큐브맵으로 변환된 HDR 이미지 텍스쳐 선언
uniform sampler2D equirectangularMap;

// PI 상수값 정의
const float PI = 3.14159265359;

void main() {
  /*
    정규화한 각 프래그먼트의 world space 방향벡터를
    surface point P 의 방향벡터 N (= 반구 영역의 방향벡터) 로 사용함.

    -> 나중에 pbrShader 에서 irradiance map 로부터 irradiance 값을 샘플링할 때,
    각 프래그먼트(surface point)에 대해 이 방향벡터를 사용할 것임!
  */
  vec3 N = normalize(WorldPos);

}
