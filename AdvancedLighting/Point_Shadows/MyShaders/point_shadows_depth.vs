#version 330 core

layout(location = 0) in vec3 aPos;

/* 변환 행렬을 전송받는 uniform 변수 선언 */ 

// light space 내부의 각 오브젝트들에 적용할 모델 행렬
uniform mat4 model;

void main() {
  // 오브젝트 공간 좌표에 모델 행렬을 곱해서 월드 공간까지만 변환시킨 후, geometry shader 파이프라인으로 넘겨줌.
  gl_Position = model * vec4(aPos, 1.0);
}

/*
  왜 월드 공간까지만 변환시킨 후 geometry shader 로 넘기는걸까?


  omnidirectional shadow mapping 기법 구현 시,
  큐브맵의 각 6면에 대해 각각의 light space 변환행렬을 적용해줘야 함.

  그런데, vertex shader 단계에서는
  각 6면에 대한 각각의 변환을 수행하기 어렵겠지!

  따라서, vertex shader 단계에서는 월드 공간 좌표계까지만 변환시키고,
  light space 좌표계로의 변환은 geometry shader 단계에 맡기려는 것!
*/
