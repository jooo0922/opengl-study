#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoorss;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

/* fragment shader 단계로 출력할 interface block 선언 (관련 설명 AdvancedGLSL 참고)  */
// 참고로, block Name (VS_OUT)은 다음 단계의 쉐이더와 일치시켜야 하고, instance Name (vs_out) 쉐이더 단계마다 임의로 다르게 지어줘도 됨.
out VS_OUT {
  vec3 FragPos; // 월드공간 프래그먼트 위치값
  vec2 TexCoords;
  vec3 TangentLightPos; // 탄젠트 공간 조명 위치값
  vec3 TangentViewPos; // 탄젠트 공간 카메라 위치값
  vec3 TangentFragPos; // 탄젠트 공간 프래그먼트 위치값
} vs_out;

/* 변환 행렬을 전송받는 uniform 변수 선언 */ 

// 뷰 행렬
uniform mat4 view;

// 투영 행렬
uniform mat4 projection;

// 모델 행렬
uniform mat4 model;

/* 기타 uniform 변수 */

// 월드공간 조명 위치값
uniform vec3 lightPos;

// 월드공간 카메라 위치값
uniform vec3 viewPos;

void main() {
  /* interface block 에 정의된 멤버변수들에 프래그먼트 쉐이더 단계로 보간하여 출력할 값들을 할당 */

  // 프래그먼트 위치값을 오브젝트 공간 -> 월드 공간으로 변환하여 전송
  vs_out.FragPos = vec3(model * vec4(aPos, 1.0));

  // uv 좌표값 프래그먼트 쉐이더로 보간하여 전송
  vs_out.TexCoords = aTexCoorss;

  /* 노멀벡터의 좌표계 변환을 위한 TBN 행렬 계산 */

  // 노멀행렬 계산 (하단 필기 참고)
  mat3 normalMatrix = transpose(inverse(mat3(model)));

  // 탄젠트 공간의 tangent, normal 벡터를 월드공간 벡터로 변환
  vec3 T = normalize(normalMatrix * aTangent);
  vec3 N = normalize(normalMatrix * aNormal);

  // 그람-슈미트 직교화 (Gram-Schmidt process) 를 이용하여 tangent 벡터 재직교화(re-orthogonalized) (하단 필기 참고)
  T = normalize(T - dot(T, N) * N);

  // tangent, normal 벡터를 외적하여 두 벡터에 모두 직교하는 bitangent 벡터 계산
  vec3 B = cross(N, T);

  /* 조명 계산에 필요한 월드 공간 좌표값들을 탄젠트 공간으로 역변환 (하단 필기 참고) */

  // TBN 행렬의 역행렬 계산 (역행렬을 transpose() 로 구하는 이유 하단 필기 참고)
  mat3 TBN = transpose(mat3(T, B, N));

  // 조명 위치값을 월드 공간 -> 탄젠트 공간으로 역변환
  vs_out.TangentLightPos = TBN * lightPos;

  // 카메라 위치값을 월드 공간 -> 탄젠트 공간으로 역변환
  vs_out.TangentViewPos = TBN * viewPos;

  // 프래그먼트 위치값을 월드 공간 -> 탄젠트 공간으로 역변환
  vs_out.TangentFragPos = TBN * vs_out.FragPos;

  // 오브젝트 공간 좌표에 모델 행렬 > 뷰 행렬 > 투영 행렬 순으로 곱해서 좌표계를 변환시킴.
  gl_Position = projection * view * model * vec4(aPos, 1.0);
}

/*
  탄젠트 공간의 벡터를 월드공간으로 변환 시, 모델행렬 vs 노멀행렬 간 차이점


  탄젠트 공간을 기준으로 정의된 tangent, bitangent, normal 벡터를
  월드공간으로 변환할 때, 모델행렬을 사용하는 것도 '가능은' 함.

  그러나, 모델행렬에 비균일 스케일링(non-uniform sacle) 변환이 적용되어 있는 상태라면,
  모델행렬로 변환 시, 월드공간으로의 정확한 변환을 보장할 수 없음.

  그래서, '가급적이면' 정확한 변환을 위해
  '노멀행렬'을 별도로 만들어서 적용해주는 게 좋음!

  관련하여 아래 링크 참고!

  - https://github.com/jooo0922/opengl-study/blob/main/Lighting/Basic_Lighting/MyShaders/basic_lighting.vs
  - https://learnopengl.com/Lighting/Basic-Lighting
*/

/*
  그람-슈미트 직교화 (Gram-Schmidt process)


  이 예제에서는 불필요한 작업이지만,
  만약 Assimp 등을 통해서 로드한 mesh 에서
  각 삼각형이 여러 정점들을 공유하는 형태로 정점 데이터가 저장되어 있다면,

  일반적으로 각 정점의 tangent 벡터는
  해당 정점을 공유하는 삼각형들의 tangent 벡터의 평균으로 계산되는 경우가 많음.

  그래야 쉐이딩이 부드럽게 처리된다고 하는데,
  이것이 갖고 있는 문제는,

  그렇게 각 정점의 tangent 벡터가 점점 부정확해지는 문제가 발생함.

  이로 인해, 직교행렬이어야 할 TBN 행렬이
  직교행렬이 아닌 것이 되고, TBN 행렬을 사용한 월드공간 변환이
  부정확해지는 문제까지 발생할 수 있음.

  이를 위해,
  월드공간으로 변환시킨 tangent 벡터를
  '그람-슈미트 직교화' 라는 선형대수의 공식을 사용하여
  월드공간으로 변환시킨 노멀벡터에 대해 
  re-orthogonalize 시키는 작업을 처리함.

  이에 대한 자세한 내용은 아래 링크 참고! 

  https://subprofessor.tistory.com/70
*/

/*
  월드 공간 -> 탄젠트 공간으로 '역변환' 하는 이유


  일반적으로 normal mapping 을 적용할 때의 절차는 다음과 같음.
  
  먼저, 각 삼각형을 local 로 하는 tangent space 의
  3개의 직교 축인 Tangent, Bitangent, Normal 벡터들을
  월드공간으로 변환한 후, 열 벡터로 꽂아넣음으로써,
  3개의 월드공간 직교 벡터를 기저벡터로 삼는, 3*3 의 TBN 행렬을 만들어 놓음.

  여기서부터 2가지 방식으로 갈리게 되는데,

  1. 첫 번째 방법은 가장 일반적인 방법으로, 
  위에서 만든 TBN 행렬을 프래그먼트 쉐이더로 전송하고,
  프래그먼트 쉐이더에서 normal map 에서 노멀벡터를 샘플링하고,
  
  (참고로, normal map 의 노멀벡터들은 
  어떠한 임의의 삼각형을 기준으로 하는 탄젠트 공간에 
  정의되어 있는 것으로 가정하여 저장됨.)
  
  TBN 행렬로 곱하여 이를 월드공간 노멀벡터로 변환한 뒤,
  이후의 조명 계산 공식에 사용됨.


  2. 그러나, 위 방식은 프래그먼트 쉐이더에서
  TBN 행렬을 사용한 행렬 곱이 지나치게 빈번해진다는 단점이 있음.

  그래서, 
  "아싸리 프래그먼트 쉐이더에서 처리하던 행렬 곱을
  모두 버텍스 쉐이더로 옮겨버리면,
  행렬 곱의 횟수가 줄어들어서 계산 효율을 높일 수 있지 않을까?"

  라는 아이디어에서 출발한 것이 두 번째 방법임.

  두 번째 방법에서는 TBN 행렬의 역행렬을 계산해버린 다음,
  이 역행렬로 '조명 위치, 카메라 위치, 프래그먼트 위치' 등
  월드 공간으로 정의된 위치값들을 탄젠트 공간으로 역변환 시켜버림.

  그리고 이 위치값들을 프래그먼트 쉐이더로 보간하여 전송하고,
  조명 계산에서 사용하도록 함.

  어차피 이 위치값들은 탄젠트 공간으로 변환된 위치값들이므로,
  normal map 에서 샘플링한 탄젠트 공간 노멀벡터를 조명계산에 바로 사용할 수 있음!

  
  참고로, TBN 행렬의 역행렬을 계산할 때에는,
  inverse() 대신 transpose() 내장함수를 사용하는데,
  
  그 이유는, 첫째, inverse() 함수는 아주 비싼 연산이고,
  둘째, TBN 행렬같은 직교 행렬은 역행렬과 전치행렬이 동일하다는 특성이 있음!
*/