#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

/* fragment shader 단계로 전송할 출력 변수 선언  */
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

/* 변환 행렬을 전송받는 uniform 변수 선언 */ 

// 투영 행렬
uniform mat4 projection;

// 뷰 행렬
uniform mat4 view;

// 모델 행렬
uniform mat4 model;

/* 기타 uniform 변수 선언 */

// 노멀벡터 뒤집기 여부
uniform bool invertedNormals;

void main() {
  // view space 정점 좌표로 변환하여 보간 출력
  vec4 viewPos = view * model * vec4(aPos, 1.0);
  FragPos = viewPos.xyz;

  // 텍스쳐 좌표 보간 출력
  TexCoords = aTexCoords;

  // 노멀벡터를 view space 노멀벡터로 변환하기 위한 노멀행렬 계산
  mat3 normalMatrix = transpose(inverse(mat3(view * model)));

  // 노멀벡터 뒤집기 여부에 따라 노멀벡터를 뒤집어 줌. (room cube 는 BACK_FACE 를 렌더링하므로, 바깥 쪽으로 향하는 노멀벡터를 뒤집어줘야 함!)
  Normal = normalMatrix * (invertedNormals ? aNormal : -aNormal);

  // view space 좌표에 투영 행렬을 마저 곱해서 좌표계를 변환시킴.
  gl_Position = projection * viewPos;
}