#version 330 core

// 프래그먼트 쉐이더 출력 변수 선언
out vec4 FragColor;

// vertex shader 단계에서 전달받는 입력 변수 선언
in vec3 WorldPos;

// 구체를 평면에 투영해서 만든 2D HDR 이미지 텍스쳐 선언
uniform sampler2D equirectangularMap;

// cartesian to spherical 좌표 변환 시 사용할 상수값 정의 (vec2 의 각 컴포넌트는 1 / (2 * π), 1 / π 에 해당)
const vec2 invAtan = vec2(0.1591, 0.3183);

/*
  단위큐브의 각 프래그먼트 world space 위치값을 3차원 데카르트 좌표계의 방향벡터로 삼고,
  해당 방향벡터를 구면좌표계로 변환함.

  변환된 구면좌표계의 theta, phi 각도는
  평면에 투영된 HDR 이미지 텍스쳐를 샘플링할 때 사용할
  uv 좌표값으로 사용됨.

  HDR 이미지는 구체가 평면에 투영된 것이므로,
  spherical mapping 기법과 유사하게 샘플링할 수 있음.
  
  따라서, 변환된 구면좌표계를
  2D 텍스쳐 샘플링에 사용할 수 있음!
*/
vec2 SampleSphericalMap(vec3 v) {
  /*
    cartesian to spherical 좌표 변환 공식은 pbrt 4판의 공식을 약간 변형해서 사용함.
    https://pbr-book.org/4ed/Textures_and_Materials/Texture_Coordinate_Generation#SphericalMapping 참고

    참고로, uv 좌표로 사용하기 위한 구면좌표계 변환 공식은 
    일반적인 cartesian to spherical 좌표 변환 공식과 약간 차이가 있음.

    그래서 구글링이나 위키백과에 검색해서 나온 공식과
    약간 다르게 보일 것임.
  */
  vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
  uv *= invAtan;

  // 변환된 구면좌표계를 uv 좌표값으로 사용할 수 있도록 [0.0, 1.0] 범위로 맵핑하여 반환
  uv += 0.5;
  return uv;
}

void main() {
  // 정규화한 각 프래그먼트의 방향벡터를 입력하여 HDR 이미지 샘플링에 사용할 uv 좌표값으로 계산
  vec2 uv = SampleSphericalMap(normalize(WorldPos));

  // 2D HDR 이미지 텍스쳐 샘플링
  vec3 color = texture(equirectangularMap, uv).rgb;

  // 샘플링한 HDR 이미지 texel 을 단위큐브의 각 프래그먼트 색상으로 출력
  FragColor = vec4(color, 1.0);
}
