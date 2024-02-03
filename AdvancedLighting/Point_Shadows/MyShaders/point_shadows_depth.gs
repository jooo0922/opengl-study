#version 330 core

// 입력 primitive 지정자
layout(triangles) in;

// 출력 primitive 지정자 (하단 필기 참고)
layout(triangle_strip, max_vertices = 18) out;

// 큐브맵의 각 6면을 바라보는 light space 좌표계로 변환해주는 행렬들을 전송받는 uniform 배열 변수 선언
uniform mat4 shadowMatrices[6];

/*
  vertex shader 로부터 받은 각 정점의 월드공간 좌표계를
  
  별도의 변환처리 없이
  fragment shader 단계에 그대로 
  보간하면서 넘겨주기 위해 선언한 출력 변수
*/
out vec4 FragPos;

void main() {
  /*
    입력받은 삼각형 primitive 를
    큐브맵의 각 6면에 대해 순회하면서 
    각각의 light space 좌표계로 변환한 후,

    큐브맵 텍스쳐의 깊이 버퍼에
    각각 변환된 6개의 triangle_strip primitive 를 렌더링함.  
  */
  for(int face = 0; face < 6; face++) {
    // 현재 프레임버퍼에 바인딩된 큐브맵 버퍼의 몇 번째 face 에 출력된 primitive 를 렌더링할 지 지정 (gl_Layer 관련 하단 필기 참고)
    gl_Layer = face;

    // 입력받은 삼각형의 각 정점들을 순회
    for(int i = 0; i < 3; i++) {
      // vertex shader 로부터 받은 월드공간 정점을 fragment shader 단계로 그대로 보간하여 전송
      FragPos = gl_in[i].gl_Position;

      // 삼각형의 각 정점들을 '월드공간 좌표계 -> 큐브맵의 각 6면에 대한 light space 좌표계'로 변환
      gl_Position = shadowMatrices[face] * FragPos;

      // 생성할 출력 primitive 에 변환된 vertex 데이터를 추가
      EmitVertex();
    }

    // 큐브맵 각 6면의 light space 좌표계로 변환된 세 정점들을 삼각형으로 조립(primitive assembly) 후, 
    // 조립된 출력 primitive 를 다음 파이프라인(fragment shader)으로 전송
    EndPrimitive();
  }
}

/*
  출력 primitive 의 최대 정점 개수를 18 개인 이유


  vertex shader 단계에서 
  3개의 정점들로 구성된 삼각형 단위의 primitive 를 넘겨받았다면,
  
  geometry shader 단계에서는 이 삼각형을 
  큐브맵의 각 6면을 바라보는 light space 좌표계로 변환한 후,
  
  큐브맵 텍스쳐의 각 6면의 깊이 버퍼에 각각 렌더링 해줘야 함.

  즉, 동일한 삼각형을 각각 6번 좌표계 변환한 뒤,
  큐브맵의 각 6면의 깊이 버퍼마다 해당 삼각형을 6번 렌더링해줘야 한다는 뜻! 

  이 말은 즉,
  "이전 파이프라인(vertex shader)에서 동일한 삼각형을 전달받은 뒤,
  6개의 새로운 출력 primitive 로 만들어서 다음 파이프라인(fragment shader)으로 전송해줘야 함"을 의미함!

  따라서, 총 6개의 삼각형을 
  출력 primitive 로 조립하여 전송(== EndPrimitive())해야 하므로,
  출력 primitive 의 총 정점 개수는 '삼각형 정점 개수 3개 * 삼각형 개수 6개 = 18개' 가 나오는 것임! 
*/

/*
  지오메트리 쉐이더 내장변수 gl_Layer


  만약, 현재 프레임버퍼에 큐브맵 텍스쳐가 attach 되어있다면,
  
  geometry shader 단계에서 조립한 출력 primitive 를
  큐브맵의 어느 면에 해당하는 버퍼에 렌더링할 지 결정할 수 있음.

  geometry shader 의 내장변수인 gl_Layer 에
  해당 큐브맵 면의 index 를 입력하면,

  큐브맵의 해당 면에 대한 깊이, 색상 등의 데이터를 기록하는 버퍼 위치에 
  조립된 출력 primitive 를 렌더링할 수 있도록 해줌.

  이를 통해, omnidirectional shadow map 으로 사용할
  큐브맵 텍스쳐의 특정 면에 해당하는 버퍼 위치에
  해당 면 방향으로 바라봤을 때의 가장 가까운 깊이값들을 채워넣을 수 있을 것임!
*/

/*
  지오메트리 쉐이더 내장변수 gl_in


  버텍스 쉐이더 단계에서 계산된 데이터들을
  지오메트리 쉐이더 단계로 전달할 때, 
  
  '필수적으로 반드시 전달해줘야 하는 데이터들'만
  interface block 형태로 따로 묶어서 전달해주는 데이터들을
  내장변수 gl_in 을 통해 관리하고 있음.

  이 내장변수는 내부적으로 아래 형태로 구현되어 있음.

  in gl_Vertex
  {
      vec4  gl_Position;
      float gl_PointSize;
      float gl_ClipDistance[];
  } gl_in[];

*/
