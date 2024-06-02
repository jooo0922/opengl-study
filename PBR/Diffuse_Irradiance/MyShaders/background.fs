#version 330 core

// 프래그먼트 쉐이더 출력 변수 선언
out vec4 FragColor;

// vertex shader 단계에서 전달받는 입력 변수 선언
in vec3 WorldPos;

// HDR 이미지 데이터가 렌더링된 큐브맵 텍스쳐 선언 -> skybox 에 적용 예정
uniform samplerCube environmentMap;

void main() {
  // world space 좌표는 큐브맵 샘플링을 위한 방향벡터로 보간해서 사용할 수 있음!
  // -> skybox 버텍스 쉐이더에서 model 행렬이 적용되지 않았으므로, world space == local space 일치하는 상황!
  vec3 envColor = texture(environmentMap, WorldPos).rgb;

  // Reinhard Tone mapping 알고리즘을 사용하여 HDR -> LDR 변환
  /*
    큐브맵에서 샘플링한 [0, 1] 범위를 벗어난 HDR 이미지 데이터를
    [0, 1] 범위 내로 존재하는 LDR 색상값으로 변환하기

    -> 즉, Tone mapping 알고리즘 적용!
    https://github.com/jooo0922/opengl-study/blob/main/AdvancedLighting/HDR/MyShaders/hdr.fs 참고
  */
  envColor = envColor / (envColor + vec3(1.0));

  // linear space 색 공간 유지를 위해 gamma correction 적용하여 최종 색상 출력
  envColor = pow(envColor, vec3(1.0 / 2.2));
  FragColor = vec4(envColor, 1.0);
}
