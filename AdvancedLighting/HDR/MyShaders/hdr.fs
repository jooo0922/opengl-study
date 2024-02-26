#version 330 core

out vec4 FragColor;

// 버텍스 쉐이더에서 전송받은 텍스쳐 좌표 입력변수 선언
in vec2 TexCoords;

/* uniform 변수 선언 */

// shadow map 텍스쳐
uniform sampler2D depthMap;

// 깊이값을 non-linear(logarithmic) > linear 로 변환할 때 사용할 near 값
uniform float near_plane;

// 깊이값을 non-linear(logarithmic) > linear 로 변환할 때 사용할 far 값
uniform float far_plane;

// 원근투영 행렬 내에서 비선형 방정식에 의해 계산된 프래그먼트의 깊이값 gl_FragCoord.z 를 선형적인 뷰 공간 z value 로 되돌리기 위해 구현된
// 비선형 방정식의 '역함수' (비선형 깊이값 -> 선형적인 z value 로 되돌림)
float LinearizeDepth(float depth) {
  // 비선형 깊이값을 클립좌표로 되돌림
  float z = depth * 2.0 - 1.0;

  // non-linear(정확히는 logarithmic) 클립좌표 z -> linear 뷰 좌표계 z 값으로 역변환  
  return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main() {
  // shadow map 에서 샘플링한 텍셀의 임의의 컴포넌트(흑백이라 r, g, b 값이 모두 동일하니 어떤 걸 사용하든 무방)를 깊이값으로 할당
  float depthValue = texture2D(depthMap, TexCoords).r;

  /*
    만약, 원근 투영(perspective projection)행렬로 렌더링된 shadow map 이라면, 
    non-linear(정확히는 logarithmic) 하게 깊이값이 계산되기 때문에,

    shadow map 을 '시각화(debugging)'하는 QuadMesh 에 적용하려면
    아래 코드와 같이, 깊이값을 가급적 linear 하게 변환한 다음에 
    최종 색상으로 출력해주는 게 나음.
    
    그래야 깊이값의 차이가 눈에 확연하게 드러남.

    물론, 오로지 '시각화'를 위해서 linear 한 깊이값으로
    변환해주는 것일 뿐이지,

    shadow testing 에 사용할 깊이값이라면
    원본 그대로의 정확한 깊이값을 사용하는 게 맞겠지!

    깊이값 linear 변환 관련해서는
    https://github.com/jooo0922/opengl-study/blob/main/AdvancedOpenGL/Depth_Testing/MyShaders/depth_testing.fs 참고!
  */
  // FragColor = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0);

  // 샘플링한 깊이값을 최종 색상으로 출력 -> shadow map 에 기록된 깊이 버퍼를 QuadMesh 에 시각화함
  FragColor = vec4(vec3(depthValue), 1.0);
}
