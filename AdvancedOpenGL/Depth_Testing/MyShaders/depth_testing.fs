#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

// .cpp 에서 투영행렬 계산 시 사용했던 near, far 값 초기화
float near = 0.1;
float far = 100.0;

// 투영행렬 내에서 비선형 방정식에 의해 계산된 프래그먼트의 깊이값 gl_FragCoord.z 를 선형적인 뷰 공간 z value 로 되돌리기 위해 구현된
// 비선형 방정식의 '역함수' (비선형 깊이값 -> 선형적인 z value 로 되돌림)
float LinearizeDepth(float depth) {
  float z = depth * 2.0 - 1.0;
  return (2.0 * near * far) / (far + near - z * (far - near));
}

void main() {
  // 전송받은 텍스쳐 객체를 샘플링하여 최종 색상 계산
  // FragColor = texture2D(texture1, TexCoords);

  // gl_FragCoord 에 들어있는 현재 프래그먼트의 깊이값을 최종 색상으로 출력하여 depth buffer 시각화
  // FragColor = vec4(vec3(gl_FragCoord.z), 1.0);

  /*
    비선형 방정식에 의해 계산된 프래그먼트 깊이값 gl_FragCoord.z 는 
    대부분의 값이 1.0 에 가깝게 몰려있어 깊이버퍼 시각화에는 부적절했음.

    따라서, 비선형적인 깊이값을 다시 선형적인 뷰 공간 z value 로 되돌리면, 
    z값이 카메라로부터 멀어질수록 깊이값 차이를 선형적으로 시각화할 수 있을 것임!

    이를 위해, 깊이값 계산에 사용했던 비선형 방정식의 역함수를 LinearizeDepth() 함수로 구현하고, 
    이를 통해 뷰 공간 z 값을 구한 것

    이때, 뷰 공간 z 값은 near 와 far 사이인 0.1 ~ 100 사이의 값으로 계산될 것이므로,
    이를 [0, 1] 범위의 선형적인 깊이값으로 맵핑시키기 위해 far 값으로 나눠준 것임.

    그러나, 이는 정확한 선형 깊이값을 계산하는 방법은 아니지만,
    계산의 편의상 near 값이 0에 가까우므로 그냥 far 값으로만 나눠준 것!
  */
  float depth = LinearizeDepth(gl_FragCoord.z) / far;
  FragColor = vec4(vec3(depth), 1.0);
}

// 참고로, 위 예시처럼 깊이 버퍼를 시각화하기 위해 
// 비선형적인 깊이값 대신 선형적인 뷰 공간 z 값을 사용하는 아이디어는
// 이득우의 게임수학 p.442 ~ 443 에서도 사용했던 방식임! 