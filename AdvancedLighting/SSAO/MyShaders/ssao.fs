#version 330 core

// 최종 색상을 할당할 출력 변수 선언
/*
    SSAO 가 적용된 occlusion factor 를 렌더링할 텍스쳐 버퍼는
    내부 포맷이 GL_RED 로 설정되어 있으므로,

    float 타입의 값만 출력 변수에 할당해주는 게 맞음.
*/
out float FragColor;

// vertex shader 단계에서 전달되면서 보간된 텍스쳐 좌표 입력 변수 선언
in vec2 TexCoords;

/* 
  OpenGL 에서 전송해 줄 uniform 변수들 선언 
*/

// Geometry pass 에서 저장한 geometry data 가 담긴 각 G-buffer 의 sampler 변수 선언
uniform sampler2D gPosition;
uniform sampler2D gNormal;

// 16개의 Random rotation vector 가 저장된 4*4 텍스쳐 버퍼의 sampler 변수 선언
uniform sampler2D texNoise; 

// 반구 영역 내의 64개의 sample kernel 이동 벡터가 전송될 배열 변수 선언
uniform vec3 samples[64];

// view space 기준 sample points 위치값을 NDC 좌표계로 변환하는 과정에서 사용할 투영 행렬
uniform mat4 projection;

/* SSAO Parameters */

// 반구 영역 내의 sample kernel 개수
int kernelSize = 64;

// 반구 영역의 반지름 (sample kernel 이동 벡터의 길이를 반구 영역의 반지름에 맞게 전체적으로 조정할 때 사용)
float radius = 0.5;

// scene 의 복잡도에 따라 발생할 수도 있는 acne effect 방지를 위한 bias 값
// (acne 현상 관련 https://github.com/jooo0922/opengl-study/blob/main/AdvancedLighting/Shadow_Mapping_2/MyShaders/shadow_mapping.fs 참고)
float bias = 0.025;

/*
  4*4 크기의 Random rotation vector 텍스쳐 버퍼를 
  800*600 크기의 해상도 전체 영역에 걸쳐 tiling 하기 위해,

  텍스쳐 버퍼를 GL_REPEAT 모드로 반복 샘플링하여 사용할 수 있도록 
  보간된 uv 좌표에 적용할 scale 값 계산
*/
const vec2 noiseScale = vec2(800.0 / 4.0, 600.0 / 4.0);

void main() {
  /* G-buffer 로부터 geometry data 가져오기 */

  // G-buffer 로부터 현재 pixel 의 view space position 값 샘플링
  vec3 fragPos = texture(gPosition, TexCoords).rgb;

  // G-buffer 로부터 현재 pixel 의 view space normal 값 샘플링
  vec3 normal = texture(gNormal, TexCoords).rgb;

  // 보간된 uv 좌표를 scaling 하여 4*4 크기의 Random rotation vector 텍스쳐 버퍼를 반복 샘플링
  vec3 randomVec = texture(texNoise, TexCoords * noiseScale).xyz;

  /* 
    tangent space 로 계산된 sample kernel 이동 벡터를 
    view space 로 변환할 때 사용할 TBN 행렬 계산 
  */

  // 그람-슈미트 직교화 (Gram-Schmidt process) 를 이용하여 random rotation vector 기반의 tangent 기저 축 재직교화(re-orthogonalized)
  // (그람-슈미트 직교화 관련 https://github.com/jooo0922/opengl-study/blob/main/AdvancedLighting/Normal_Mapping/MyShaders/normal_mapping.vs 참고)
  vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));

  // tangent, normal 기저 축을 외적하여 두 벡터에 모두 직교하는 bitangent 기저 축 계산
  vec3 bitangent = cross(normal, tangent);

  // tangent space 의 세 기저 축을 열 벡터로 꽂아넣은 TBN 행렬 계산
  mat3 TBN = mat3(tangent, bitangent, normal);

  /*
    반구 영역의 sample kernel 을 순회하며
    occlusion factor 를 누산
  */

  // 반복문을 순회하며 누산할 occlusion factor 변수 초기화
  float occlusion = 0.0;

  // 반구 영역 내의 sample kernel 개수만큼 반복문 순회
  for(int i = 0; i < kernelSize; i++) {
    // tangent space 기준으로 정의된 sample kernel 이동 벡터를 view space 로 변환
    vec3 samplePos = TBN * samples[i];

    // 현재 프래그먼트 위치에서 각 sample kernel 이동 벡터를 더해 sample point 좌표값 계산
    samplePos = fragPos + samplePos * radius;

    /* 
      view space 기준 sample point 좌표값을 NDC 좌표계로 변환하여
      G-buffer 로부터 샘플링할 때 사용할 uv 좌표로 계산
    */

    // 투영행렬과 곱하기 위해 동차좌표계로 맞춤
    vec4 offset = vec4(samplePos, 1.0);

    // 투영행렬과 곱해 clip space 로 변환
    offset = projection * offset;

    // 변환된 clip space 좌표계를 w 컴포넌트로 원근 분할 -> NDC 좌표계로 변환
    offset.xyz /= offset.w;

    // [-1.0, 1.0] 사이의 NDC 좌표계를 [0.0, 1.0] 사이의 값으로 맵핑 -> G-buffer 샘플링에 사용할 uv 좌표 범위로 변환
    offset.xyz = offset.xyz * 0.5 + 0.5;

    // sample point 위치에 대응되는 NDC 좌표 지점에서 G-buffer 에 저장된 view space position 의 깊이값 샘플링
    float sampleDepth = texture(gPosition, offset.xy).z;

    // G-buffer 로부터 샘플링한 깊이값 Range Check (하단 필기 참고)
    float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));

    // 샘플링된 깊이값과 현재 프래그먼트 깊이값을 비교하여 occlusion 여부 테스트 (하단 필기 참고)
    // 또한, occlusion 테스트 결과값의 영향도를 rangeCheck 값으로 조정
    occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
  }

  // 누산된 occlusion factor 의 최댓값이 64 일테니까, 누산된 결과값을 64.0 으로 나눠서 [0.0, 1.0] 사이의 값으로 normalize
  // + 정규화된 occlusion factor 값을 1.0 에서 빼서 뒤집어 줌 
  // -> occlusion 이 많이 발생한 프래그먼트 일수록 occlusion 값이 0 에 가까워지겠군!
  occlusion = 1.0 - (occlusion / kernelSize);

  // FragColor 출력 변수에 누산된 최종 색상값 할당
  /*
    환경광 차폐가 많이 발생한 프래그먼트일수록 
    occlusion 값이 0에 가까울 것이므로,

    프래그먼트의 색상값도 0에 가까워지는,
    즉, 더 어둡게 표현되어 텍스쳐 버퍼에 저장될 것임.
  */
  FragColor = occlusion;
}

/*
  Range Check 란 무엇인가?


  이 예제에서는 각 프래그먼트의 노멀 벡터를 중심으로 한
  반구 영역의 sample point 들을 NDC 좌표계로 변환해서 그 좌표값으로
  G-buffer 에 저장된 깊이값을 샘플링하고 있음.

  그런데, 만약 현재 프래그먼트가 그것이 속한 geometry 의
  edge 에 가까운 부분의 프래그먼트라면?

  그 edge 지점에 위치한 프래그먼트 주변에서 샘플링한 깊이값은
  '아주 아주 멀리 있는 깊이값'도 샘플링하게 될 수 있다는 것이지!


  이렇게 '아주 아주 멀리 있는 깊이값'으로 occlusion 테스트를 했을 때,
  그 값이 정확하지는 않을 것임.


  따라서, 반구 영역의 반지름인 radius 값에 대해
  '현재 프래그먼트의 깊이값 - 샘플링하나 깊이값 차이' 가
  얼마만큼의 비율인지에 따라 해당 occlusion 테스트 결과값의 기여도(contribution)을
  조정할 수 있을 것임.


  즉, 두 깊이값의 차이가 너무 크게 나서
  반구 영역의 반지름(radius)보다 커질 정도라면, 
  그 값으로 계산한 occlusion test 결과값도 정확하지 못할 확률이 높으므로,
  그 영향도(또는 가중치, contribution)를 깊이값 차이가 크면 클수록 0 에 가깝게 조정하고,
  
  두 깊이값의 차이가 반구 영역의 반지름(radius)보다 작다면,
  무조건 영향도를 1로 해서 occlusion factor 누산에 반영하는 것이지!


  이처럼 두 깊이값 차이에 따른 occlusion factor 영향도 계산에 적합한 함수가
  smoothstep() 이라고 보면 됨!
*/

/*
  왜 sampleDepth ≥ samplePos.z 일수록 occlusion Factor 를 증가시킬까? 
  
  sampleDepth 의 z값과 samplePos 모두 view space 좌표계이므로, 
  view space 좌표계는 카메라 원점에서 멀어질수록 음수가 됨. 
  
  따라서, 카메라에서 더 멀리있는 깊이값, 
  즉, occluded 되는 깊이값이라면, 
  z 값이 더 작아야 함! 
  
  (게임수학 p.348 참고)
*/