#version 330 core

// 입력 primitive 지정자 (하단 필기 참고)
layout(points) in;

// 출력 primitive 지정자 (하단 필기 참고)
layout(triangle_strip, max_vertices = 5) out;

// vertex shader 단계에서 전달받는 입력 interface block 선언
// -> interface block 은 쉐이더 단계 간 이름을 동일하게 맞춰줄 것!
// interface block 관련 https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL 참고
in VS_OUT {
  vec3 color;
} gs_in[]; // interface block 을 배열로 선언한 이유 하단 필기 참고

// 프래그먼트 쉐이더로 보간하여 전송할 색상값 출력 변수 선언
out vec3 fColor;

// triangle_strip primitive 생성 함수
void build_house(vec4 position) {
  // 하나의 triangle_strip primitive 에 대해
  // 모든 vertex 들이 동일한 색상값을 프래그먼트 쉐이더로 전송할 수 있도록,
  // interface block 에 정의된 동일한 색상을 출력 색상으로 지정
  fColor = gs_in[0].color;

  // bottom-left 정점 위치 계산 및 추가 
  gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0);
  EmitVertex(); // EmitVertex() 관련 필기 하단 참고

  // bottom-right 정점 위치 계산 및 추가 
  gl_Position = position + vec4(0.2, -0.2, 0.0, 0.0);
  EmitVertex();

  // top-left 정점 위치 계산 및 추가 
  gl_Position = position + vec4(-0.2, 0.2, 0.0, 0.0);
  EmitVertex();

  // top-right 정점 위치 계산 및 추가 
  gl_Position = position + vec4(0.2, 0.2, 0.0, 0.0);
  EmitVertex();

  // top 정점 위치 계산 및 추가 
  gl_Position = position + vec4(0.0, 0.4, 0.0, 0.0);

  // top 정점의 색상은 예외적으로 흰색으로 출력 -> house 지붕에 눈덮인 효과를 주려는 의도!
  fColor = vec3(1.0, 1.0, 1.0);
  EmitVertex();

  // EmitVertex() 로 추가된 정점들을 binding(결합)하여 1개의 triangle_strip primitive 생성
  // -> 이것이 그래픽스 파이프라인에서 'primitive assembly' 단계에 해당!
  EndPrimitive();
}

void main() {
  // glsl 내장변수 gl_in(하단 필기 참고) 로부터 primitive 의 각 정점 위치 데이터 gl_Position 을 가져오고,
  // 이를 primitive 생성 함수인 build_house 에 인자로 전달
  build_house(gl_in[0].gl_Position);
}

/*
  입력 primitive 지정자 (input primitive qualifier)


  지오메트리 쉐이더는
  버텍스 쉐이더와 프래그먼트 쉐이더 사이의 중간 단계 파이프라인으로써,
  primitive 단위의 데이터를 입력받는다.

  버텍스 쉐이더가 'vertex' 단위로 데이터를 입력받고,
  프래그먼트 쉐이더가 'fragment(pixel)' 단위로 데이터를 입력받는 것처럼,
  지오메트리 쉐이더는 'primitive(points, triangles, lines 등...)' 단위로 데이터를 입력받는다.

  따라서, 지오메트리 쉐이더가 이전 단계인 버텍스 쉐이더로부터
  입력받는 정점 데이터들은 'primitive' 단위로 묶여서 전달받게 되어있음!

  그러나, primitive 의 종류가 아주 많기 때문에,
  어떤 타입의 primitive 로 입력받느냐에 따라
  동일한 정점 데이터의 집합이라고 하더라도, 정점들이 묶이게 되는 primitive 의 단위가 달라짐!

  예를 들어, points 라면,
  정점 1개씩 primitive 단위로 묶여서 전달될 것이고,
  triangles 라면,
  정점 3개씩 primitive 단위로 묶여서 전달될 것임!

  이러한 입력 primitive 단위를 결정하는 것이 입력 primitive 지정자임.

  단, 아무런 primitive 를 맘대로 지정해도 되는 것은 절대 아니고,
  입력 primitive 타입은 항상 glDrawArrays() 같은 
  그리기 명령 함수에서 지정한 primitive 타입에 대응되는 지정자로 정의해줘야 함!
*/

/*
  출력 primitive 지정자 (output primitive qualifier)


  지오메트리 쉐이더는 최소한 1개 이상의 primitive 를 생성하여
  다음 파이프라인으로 전달해줘야 할 의무가 있음.

  이때, 지오메트리 쉐이더의 내장 함수 EndPrimitive() 를 사용하면,
  출력 primitive 타입으로 지정된 primitive 를 하나 생성하게 되고,
  (-> 이것이 그래픽스 파이프라인에서 'primitive assembly' 단계에 해당)

  이 출력 primitive 가 rasterization 을 거쳐 픽셀화 됨.
  (-> 그래픽스 파이프라인에서 'rasterization' 단계에 해당)

  픽셀화된 primitive 들은
  fragment 단위로 프래그머트 쉐이더 단계로 전달되어
  프래그먼트 쉐이더를 각 프래그먼트마다 실행하게 되는 것임!

  이때, 어떠한 타입의 primitive 로 생성하여
  다음 파이프라인으로 출력할 지 정하는 것이 '출력 primitive 지정자'!

  이것은 입력 primitive 지정자와 달리
  원하는 primitive 를 자유롭게 지정할 수 있으며,
  (그렇다 해도, points, line_strip, triangle_strip 세 개의 타입 중 하나만 지정 가능)

  명심해야 할 것은,
  출력 primitive 를 지정할 시,
  이 출력 primitive 를 생성할 때 EmitVertex() 내장함수를 통해 추가할 
  최대 vertex 개수를 'max_vertices = ' 키워드를 통해 명시해줘야 함!
  이 개수를 넘어서는 vertex 들은 OpenGL 이 그리기 명령에서 제외시킴.

  이 예제에서는 총 5개의 vertex 를 가지고
  1개의 triangle_strip primitive 를 생성하므로,

  layout(triangle_strip, max_vertices = 5) out;

  와 같이 출력 primitive 지정자를 선언함.
*/

/*
  geometry shader 에서 interface block 을 배열로 선언한 이유


  입력 primitive 지정자 관련 필기에서 설명했듯이,
  지오메트리 쉐이더는 입력 단위가 primitive 이기 때문에,
  
  해당 primitive 를 구성하는 
  여러 개의 vertex 집합을 한꺼번에 입력으로 받게 됨.

  따라서, 버텍스 쉐이더에서 출력해주는
  interface block 또한 그에 대응하여
  
  해당 primitive 에 대해
  여러 개의 interface block 집합을 한꺼번에 입력받아야 타당하겠지.

  따라서, 실제로는 1개씩의 vertex 와 interface block 만
  입력받는다고 하더라도,

  지오메트리 쉐이더에서는 항상 그것을
  배열 형태로 받도록 설계되어 있음.

  points 를 제외하면, 대부분의 primitive 는
  여러 개의 vertex 집합을 입력받아야 하니까!
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

/*
  EmitVertex()


  지오메트리 쉐이더의 주요 내장함수 중 하나인
  EmitVertex() 는 '생성할 primitive 에 vertex 데이터를 추가' 하는 역할!

  그런데, 좀 더 구체적으로 어떻게 데이터를 추가하는지 설명하자면,
  '현재 시점에 저장된 데이터를 기준으로' vertex 를 추가하고,
  다음 파이프라인으로 해당 데이터들을 넘긴다고 보면 됨!

  예를 들어,
  EmitVertex() 를 호출하는 시점에
  '현재 시점에 gl_Position 에 저장된 좌표값을 기준으로'
  정점 좌표를 primitive 에 추가하게 됨.

  또는, 프래그먼트 쉐이더 단계로 보간하여 전송할
  출력변수에 각 정점의 데이터를 넘기고자 한다면,
  (위 예제에서는 fColor 변수가 이에 해당!)

  EmitVertex() 를 호출하는 시점에
  '현재 시점에 fColor 출력 변수에 저장된 데이터를 기준으로'
  프래그먼트 쉐이더 단계로 해당 색상 데이터를 보간하여 전송하겠지!

  이처럼, 어떤 정점에 대응되는 특정 데이터를 
  다음 파이프라인 단계인 프래그먼트 쉐이더로 넘기고자 한다면,

  EmitVertex() 를 호출하는 시점에 이미 각 출력변수에
  넘기고자 하는 데이터가 저장되어 있어야 함!
*/