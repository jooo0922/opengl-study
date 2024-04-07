#version 330 core

// 최종 색상을 할당할 출력 변수 선언
/*
    SSAO Blur 를 적용하여 렌더링할 텍스쳐 버퍼도 
    SSAO 텍스쳐 버퍼와 마찬가지로 내부 포맷이 GL_RED 로 설정되어 있으므로,

    float 타입의 값만 출력 변수에 할당해주는 게 맞음.
*/
out float FragColor;

// vertex shader 단계에서 전달되면서 보간된 텍스쳐 좌표 입력 변수 선언
in vec2 TexCoords;

/* 
  OpenGL 에서 전송해 줄 uniform 변수들 선언 
*/

// SSAO occlusion factor 를 렌더링한 텍스쳐 버퍼의 sampler 변수 선언
uniform sampler2D ssaoInput;

void main() {
  // SSAO occlusion factor 가 렌더링된 텍스쳐 버퍼의 단일 texel 크기 계산
  vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));

  // blur 결과를 누산할 변수 초기화
  float result = 0.0;

  // 이중 for 문을 순회하며 현재 프래그먼트를 중심으로 주변 texel 샘플링 및 누산
  for(int x = -2; x < 2; ++x) {

    for(int y = -2; y < 2; ++y) {
      // 주변 texel 을 샘플링할 uv 좌표값 계산을 위해,
      // 현재 프래그먼트의 uv 좌표값으로부터 더해줄 offset 값 계산
      vec2 offset = vec2(float(x), float(y)) * texelSize;

      // 주변 texel 을 샘플링한 뒤 누산
      // SSAO 텍스쳐 버퍼는 GL_RED 포맷으로 저장되므로, 샘플링한 texel 의 r 값을 참조하여 누산할 것!
      result += texture(ssaoInput, TexCoords + offset).r;
    }

  }

  // FragColor 출력 변수에 누산된 최종 색상값 할당
  // 이때 최대 누산값인 16(= 4*4) 로 나눠서 [0.0, 1.0] 사이의 값으로 정규화함.
  FragColor = result / (4.0 * 4.0);
}

/*
  SSAO input 텍스쳐 버퍼 blur 처리 기법


  SSAO 텍스쳐 버퍼를 blur 처리하는 이유는,
  4*4 크기의 Random rotation vector 텍스쳐 버퍼를
  GL_REPEAT 모드로 반복적으로 사용하기 때문에 발생하는
  noise pattern 을 해소하기 위해 처리하는 것임.

  그렇다면, SSAO 텍스쳐 버퍼 내에서 
  정확히 이 4*4 크기 만큼의 영역 단위로 blur 처리를 해주면
  4*4 크기 단위의 격자 패턴을 알아볼 수 없을 정도로 흐려지겠지?


  따라서, 
  위 코드의 이중 for 문을 순회하면서
  
  현재 픽셀을 중심으로 4*4 크기의 영역 내의
  총 16개의 주변 texel 들을 샘플링하여 모두 누산한 뒤,
  
  16개의 texel 을 누산하여 얻을 수 있는 최댓값 16으로
  현재 프래그먼트의 누산값을 정규화해주면 되겠지!


  마지막으로 이 정규화된 누산값을
  최종 출력 변수에 할당해주면 됨.
*/