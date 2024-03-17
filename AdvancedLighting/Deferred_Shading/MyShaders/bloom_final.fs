#version 330 core

out vec4 FragColor;

// 버텍스 쉐이더에서 전송받은 텍스쳐 좌표 입력변수 선언
in vec2 TexCoords;

/* uniform 변수 선언 */

// scene 텍스쳐 (HDR 범위로 원본 씬을 렌더링한 텍스쳐 객체(즉, color attachment))
uniform sampler2D scene;

// bloomBlur 텍스쳐 (광원 큐브 영역에 대해 two-pass Gaussian blur 를 적용한 텍스쳐 객체(즉, color attachment))
uniform sampler2D bloomBlur;

// bloom 효과 활성화 여부
uniform bool bloom;

// tone mapping 에 사용할 노출값 (빛이 많을 때 / 적을 때 인간의 홍채, 카메라 조리개와 같은 역할!)
uniform float exposure;

void main() {
  // gamma correction 에 사용할 gamma 값
  const float gamma = 2.2;

  // HDR 범위로 원본 씬을 렌더링한 텍스쳐 객체에서 색상값 샘플링 -> 여기에 HDR 톤 매핑 효과를 적용할 것임!
  vec3 hdrColor = texture2D(scene, TexCoords).rgb;

  // 광원 큐브 영역에 대해 two-pass Gaussian blur 를 적용한 텍스쳐 객체에서 색상값 샘플링
  vec3 bloomColor = texture2D(bloomBlur, TexCoords).rgb;

  // bloom 상태값 여부에 따라 가산 혼합 적용
  if(bloom) {
    // 'HDR 범위로 렌더링된 원본 씬'에 '광원 큐브 영역의 two-pass Gaussian blur' 색상을 가산 혼합(additive blending)
    hdrColor += bloomColor;
  }

  /*
    [0, 1] 범위를 벗어난 HDR 색상값을
    [0, 1] 범위 내로 존재하는 LDR 색상값으로 변환하기

    -> 즉, Tone mapping 알고리즘 적용!
  */

  // Reinhard Tone mapping 알고리즘을 사용하여 HDR -> LDR 변환
  // vec3 result = hdrColor / (hdrColor + vec3(1.0));

  // exposure tone mapping 적용
  vec3 result = vec3(1.0) - exp(-hdrColor * exposure);

  // linear space 색 공간 유지를 위해 gamma correction 적용하여 최종 색상 출력 (하단 필기 참고)
  result = pow(result, vec3(1.0 / gamma));
  FragColor = vec4(result, 1.0);
}

/*      
  gamma correction

  
  CRT 모니터의 gamma 값 2.2 의 역수인 1.0 / 2.2 만큼으로 거듭제곱 함으로써,
  최종 출력 색상을 미리 밝게 보정해 줌.
  
  이렇게 하면, 
  모니터 출력 시, 2.2 거듭제곱으로 다시 gamma correction 이 적용됨으로써,

  미리 프래그먼트 쉐이더에서 gamma correction 되어 밝아진 색상이 
  다시 어두워짐으로써
  
  원래의 의도한 linear space 색 공간을
  모니터에 그대로 출력할 수 있게 됨
  
  -> 이것이 바로 'gamma correction'
*/

/*
  exposure tone mapping 알고리즘


  exposure 즉, 노출값을 기반으로
  톤 맵핑을 동적으로 적용할 수 있도록 함.

  인간의 홍채가 
  밝은 환경에서는 동공을 줄이고,
  어두운 환경에서는 동공을 늘려서 
  눈으로 들어오는 빛의 양을 조절하는 것과 같은 원리로,

  조명이 밝은 씬에서는 exposure 을 낮추고,
  조명이 어두운 씬에서는 exposure 을 높이는 식으로

  어느 영역에 detail 을 살릴 지 동적으로 결정할 수 있도록 함.

  즉, exposure 이 높으면,
  어두운 영역의 detail 이 더 살아나고,
  exposure 이 낮으면,
  밝은 영역의 detail 이 더 살아남.


  참고로, 이 알고리즘에 사용된 
  exp() 내장함수는 자연상수 e = 2.7182818284... 의
  거듭제곱을 계산해주는 함수임.
*/