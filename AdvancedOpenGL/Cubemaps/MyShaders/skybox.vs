#version 330 core

// c++ 에서 전송받은 '오브젝트 공간 좌표'
layout(location = 0) in vec3 aPos;

// 프래그먼트 쉐이더로 보간하여 전송할 텍스쳐 좌표(= 방향벡터) 변수 선언
out vec3 TexCoords;

// glm::mat4 타입 좌표계 변환 행렬을 전송받는 uniform 변수 선언
// skybox 는 TRS(이동, 회전, 스케일)을 적용하지 않으므로, 모델행렬로 변환하지 않아도 됨 == '오브젝트 공간 좌표'와 '월드공간 좌표' 가 일치함!
uniform mat4 view; // 뷰 행렬
uniform mat4 projection; // 투영 행렬

void main() {
  // 오브젝트 공간 좌표는 큐브맵 샘플링을 위한 방향벡터로 보간해서 사용할 수 있음! 
  // -> why? 물체의 중심이 원점인 좌표계라면, 좌표값 그 자체가 '원점으로부터의 방향벡터'라고도 볼 수 있으니까! 
  TexCoords = aPos;

   // 오브젝트 공간 좌표에 모델 행렬 > 뷰 행렬 > 투영 행렬 순으로 곱해서 클립좌표로 변환시킴.
  vec4 pos = projection * view * vec4(aPos, 1.0);

  // 클립좌표를 xyww 로 swizzle 해서 출력변수에 할당하여 다음 파이프라인으로 전송 (관련 내용 하단 참고)
  gl_Position = pos.xyww;
}

/*
  왜 클립좌표를 .xyww 로 swizzle 하는가?


  이 부분은 사실 skybox 를 공부하면서 많이 접했던 내용임. 
  (ex> <쉐이더 코딩 입문>(p.260-262), <WebGL 3D 그래픽 프로그래밍>(p.212), <게임수학>(p.410))

  지금까지 알고 있던 것으로는, 
  클립좌표 -> NDC 좌표 변환(= '원근분할' 과정이라고도 함) 시, z / w = 1 로 나오게 하기 위한 것으로만 알고 있었지.

  근데 왜 하필 1로 만들어야 하는걸까?

  그 이유는, skybox 가 그려질 때,
  depth buffer 에 채울 깊이값을 1(= 카메라로부터 가장 먼 깊이값)로 맞추기 위한 것!

  이때, NDC 좌표의 z좌표값이 최종적으로 depth buffer 에 저장되는
  깊이값이라는 것은 알고 있어야 함!

  즉, skybox 영역의 모든 depth buffer 를 가장 깊은 깊이값으로 채우려는 것!

  이는 skybox 렌더링 최적화 기법과 관련이 있다.

  원래 skybox 는 scene 에서 그려지는 모든 물체들보다 가장 뒤에 그려져야 하는 물체임.
  왜냐하면, 말 그대로 skybox 는 '배경'이어야 하니까!

  그래서 skybox 를 가장 먼저 그리고 난 뒤,
  scene 의 나머지 물체들을 나중에 그려서 skybox 의 프래그먼트를 덮어쓰는 방식을
  생각해볼 수도 있겠지?

  그런데, 이렇게 그리면,
  어차피 다른 물체들에 가려져서 일부분만 보이게 될 skybox 프래그먼트를
  매 프레임마다 일일이 다 그려주고, 또 다른 물체들의 프래그먼트에 의해 덮어쓰여지고
  이 과정이 너무 비효율적인 것이지.

  만약, skybox 에서 나머지 물체에 의해 가려지지 않을 부분만 골라서
  프래그먼트를 계산하고, 가려지는 부분들은 프래그먼트를 미리 discard 할 수 있다면
  효율적으로 skybox 를 그릴 수 있겠지?

  그래서
  skybox 프래그먼트의 깊이값을 모두 1로 맞춰준 뒤,
  skybox 를 다른 나머지 물체들보다 가장 마지막에 그려주면,
  먼저 그려진 물체들과 겹치는 영역에서는 해당 물체들의 깊이값이 담긴 depth buffer 와
  depth test 비교를 통해 프래그먼트가 discard 될 것임! 

  왜냐하면, skybox 의 깊이값은 카메라에서 가장 먼, 즉, 가장 깊은 값인 1이니까!

  이로 인해, 먼저 그려진 물체들과 겹쳐진 영역은
  불필요한 프래그먼트 연산을 방지할 수 있게 되어 성능 최적화가 가능함!
*/