#version 330 core

layout(location = 0) in vec3 aPos;

// 변환 행렬을 전송받는 uniform 변수 선언

// 뷰 행렬과 투영 행렬은 uniform block 으로 선언 
layout(std140) uniform Matrices {
  mat4 view; // 뷰 행렬
  mat4 projection; // 투영 행렬
};

uniform mat4 model; // 모델 행렬

void main() {
  gl_Position = projection * view * model * vec4(aPos, 1.0); // 오브젝트 공간 좌표에 모델 행렬 > 뷰 행렬 > 투영 행렬 순으로 곱해서 좌표계를 변환시킴.
}

/*
  Uniform block 이란 무엇인가?


  Uniform Buffer Object(이하 'UBO') 를 가져다가 사용할 쉐이더 프로그램이라면
  반드시 선언해줘야 하는 독특한 구조의 uniform 변수를 의미함.

  즉, 어떤 쉐이더 코드가 
  UBO 에 저장된 데이터들을 가져다가 쓰고 싶다면,
  해당 쉐이더에 UBO 에서 사용할 uniform 변수의 값들을
  위와 같이 Uniform block 에 선언해줘야 함.


  Uniform Buffer Object 와 Uniform block 의 관계에 대한 설명은
  .cpp 본문 필기를 참조하면 될 것 같고,

  여기서는 'std140' 이라고 하는
  표준 메모리 레이아웃을 정의하는 키워드만 알아보면 될 것임.


  저 메모리 레이아웃은 'shared layout' 이라고 하는
  또 다른 형태의 레이아웃을 대체하기 위한 시도로 도입됨.

  'shared layout' 은 메모리 공간을 절약하는
  훌륭한 최적화 레이아웃이긴 하지만, 하드웨어마다 데이터 타입별로 사용하는
  메모리 offset 값이 다 달라서 OpenGL 에게 특정 함수로 쿼리를 날려서
  하드웨어에 정의된 offset 값을 매번 얻어와야 한다는 불편함이 있음.

  이러한 번거로운 작업을 생략하기 위해
  하드웨어에 독립적인 '표준 메모리 레이아웃', 
  즉 std140 레이아웃을 사용함으로써, 별도의 쿼리 작업 없이 
  
  std140 에서 정의하는 set of rules(규칙)에 따라
  UBO 버퍼 객체에 데이터를 할당해놓으면,

  std140 레이아웃으로 정의된 Uniform block 은
  std140 의 규칙에 따라 데이터들이 저장되었다고 가정하고,
  그 규칙들을 기준으로 UBO 의 데이터를 읽어들임.

  
  std140 표준에서 정의하고 있는
  데이터 타입별 메모리 저장 규칙은
  LearnOpenGL 본문 테이블 참고!
*/