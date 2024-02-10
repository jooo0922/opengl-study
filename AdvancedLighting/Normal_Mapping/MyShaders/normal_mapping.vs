#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoorss;

/* fragment shader 단계로 출력할 interface block 선언 (관련 설명 AdvancedGLSL 참고)  */
// 참고로, block Name (VS_OUT)은 다음 단계의 쉐이더와 일치시켜야 하고, instance Name (vs_out) 쉐이더 단계마다 임의로 다르게 지어줘도 됨.
out VS_OUT {
  vec3 FragPos;
  vec3 Normal;
  vec2 TexCoords;
} vs_out;

/* 변환 행렬을 전송받는 uniform 변수 선언 */ 

// 뷰 행렬
uniform mat4 view;

// 투영 행렬
uniform mat4 projection;

void main() {
  // interface block 에 정의된 멤버변수들에 프래그먼트 쉐이더 단계로 보간하여 출력할 값들을 할당
  vs_out.FragPos = aPos;
  vs_out.Normal = aNormal;
  vs_out.TexCoords = aTexCoorss;

  // 오브젝트 공간 좌표에 뷰 행렬 > 투영 행렬 순으로 곱해서 좌표계를 변환시킴. (물체를 transform 하지 않으므로, 모델 행렬 미적용!)
  gl_Position = projection * view * vec4(aPos, 1.0);
}
