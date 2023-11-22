#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

// off-screen framebuffer 에 attach 된 texture unit 위치값이 전달됨.
uniform sampler2D screenTexture; 

// 현재 uv 좌표를 중심으로 동서남북 방향으로 주변 텍셀들을 추가로 샘플링하기 위해,
// 현재 uv 좌표에 각 방향으로 더해줄 offset 값 -> 주변 uv 좌표 계산에 사용
const float offset = 1.0 / 300.0;

void main() {
  /* sharpen kernel post-processing 적용 */

  // 현재 uv 좌표 (offset 값이 vec2(0.0, 0.0) 인 것이 곧 현재 uv 좌표와 같음!)를 중심으로 
  // 주변 uv 좌표 계산을 위해 각 방향마다 더해줄 offset 값의 정적 배열
  vec2 offsets[9] = vec2[](
  // 각 방향벡터 * offset
  vec2(-offset, offset), // top-left
  vec2(0.0, offset), // top-center
  vec2(offset, offset), // top-right
  vec2(-offset, 0.0),   // center-left
  vec2(0.0, 0.0),   // center-center
  vec2(offset, 0.0),   // center-right
  vec2(-offset, -offset), // bottom-left
  vec2(0.0, -offset), // bottom-center
  vec2(offset, -offset)  // bottom-right    
  );

  // kernel (= convolution matrix) 선언 및 초기화 (관련 설명 하단 참고) -> sharpen kernel
  float kernel[9] = float[](
  // row-major
  -1, -1, -1, // row1
  -1, 9, -1, // row2
  -1, -1, -1 // row3
  );

  // 현재 텍셀과 그 주변 텍셀들을 샘플링하여 저장할 동적 배열
  vec3 sampleTex[9];
  for(int i = 0; i < 9; i++) {
    sampleTex[i] = vec3(texture2D(screenTexture, TexCoords + offsets[i]));
  }

  // 샘플링한 텍셀들에 가중치(kernel)를 적용하여 합산(convolution)
  vec3 col = vec3(0.0);
  for(int i = 0; i < 9; i++) {
    col += sampleTex[i] * kernel[i];
  }

  FragColor = vec4(col, 1.0);
}

/*
  kernel (convolution matrix)

  
  커널이란 개념은 주로 이미지 처리, 딥러닝, 신호 처리 등의 분야에서 주로 사용되는 개념.
  뭐 특별한 개념은 아니고, 우리말로 '알맹이, 핵심'이라는 뜻을 갖고 있음.

  그러나 이미지 처리에서 kernel 은 특정한 형태의 행렬을 나타내는 단어인데,
  딥러닝 분야에서는 '가중치 행렬, 필터'라고도 부르는 convolution matrix 를 의미함!

  convolution 이라는 단어보다는 '가중치 행렬'이라는 표현이 더 와닿는 설명일텐데,
  이 행렬이 뭐에 쓰이는 지를 알면 금방 이해할 수 있음.

  이미지 처리 과정에서
  현재 샘플링한 픽셀 뿐만 아니라, 주변 픽셀 데이터도 함께 가져와서
  주변 픽셀과 현재 픽셀을 적절히 가공해서 새로운 데이터를 만들어내는 작업을 하는 경우가 많은데,
  이때, 각 주변 픽셀과 현재 픽셀에 곱해주는 가중치들을 모아놓은 행렬이 '가중치 행렬'!

  그래서 아무래도 현재 픽셀에 곱해주는 가중치가
  주변 픽셀에 곱해주는 가중치들과 달리 눈에 띄게 다른 값을 곱해주다보니,
  행렬의 가운데 요소(알맹이)만 뭔가 값이 눈에 띄게 달라서 'kernel(핵심, 알맹이)' 라고 부르는 듯.

  이때, kernel 행렬의 각 가중치 요소들의 합은 1 이 되어야
  convolution 결과가 일정한 크기로 유지될 수 있겠지?

  그렇다면 왜 convolution 이라는 용어를 사용하나?
  
  프로그래밍에서 convolution 이란,
  주로 샘플링된 데이터에 가중치를 곱하고 합산하는 과정을 말함.

  이는 kernel 을 이용해서 특정 픽셀과 그 주변 픽셀에 가중치를 적용하는 과정과 정확히 일치하며,
  일반적으로 수학에서 사용하는 '적분'과 상당히 유사함.

  그래서 실제로 convolution 은 적분과 관련이 깊고, 
  적분 공식으로 표현하는 경우가 많음.

  kernel 관련 자료들을 북마크 해두었으니 참고할 것!
*/