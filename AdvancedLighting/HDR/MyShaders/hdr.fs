#version 330 core

out vec4 FragColor;

// 버텍스 쉐이더에서 전송받은 텍스쳐 좌표 입력변수 선언
in vec2 TexCoords;

/* uniform 변수 선언 */

// hdrBuffer 텍스쳐 (Floating point framebuffer 에 저장된 색상 버퍼)
uniform sampler2D hdrBuffer;

// HDR 효과 활성화 여부
uniform bool hdr;

// tone mapping 에 사용할 노출값 (빛이 많을 때 / 적을 때 인간의 홍채, 카메라 조리개와 같은 역할!)
uniform float exposure;

void main() {
  // gamma correction 에 사용할 gamma 값
  const float gamma = 2.2;

  // Floating point framebuffer 에 저장된 텍스쳐 색상 버퍼에서 색상값 샘플링 -> 여기에 HDR 효과를 적용!
  vec3 hdrColor = texture2D(hdrBuffer, TexCoords).rgb;

  if(hdr) {
    /*
      [0, 1] 범위를 벗어난 HDR 색상값을
      [0, 1] 범위 내로 존재하는 LDR 색상값으로 변환하기

      -> 즉, Tone mapping 알고리즘 적용!
    */

    // Reinhard Tone mapping 알고리즘을 사용하여 HDR -> LDR 변환
    vec3 result = hdrColor / (hdrColor + vec3(1.0));

    // linear space 색 공간 유지를 위해 gamma correction 적용하여 최종 색상 출력 (하단 필기 참고)
    result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(result, 1.0);
  } else {
    // linear space 색 공간 유지를 위해 gamma correction 적용하여 최종 색상 출력
    vec3 result = pow(hdrColor, vec3(1.0 / gamma));
    FragColor = vec4(result, 1.0);
  }

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
