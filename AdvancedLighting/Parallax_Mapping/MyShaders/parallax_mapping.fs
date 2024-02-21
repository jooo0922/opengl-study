#version 330 core

out vec4 FragColor;

// vertex shader 단계에서 전달받는 입력 interface block 선언
in VS_OUT {
  vec3 FragPos; // 월드공간 프래그먼트 위치값
  vec2 TexCoords;
  vec3 TangentLightPos; // 탄젠트 공간 조명 위치값
  vec3 TangentViewPos; // 탄젠트 공간 카메라 위치값
  vec3 TangentFragPos; // 탄젠트 공간 프래그먼트 위치값
} fs_in;

// OpenGL 에서 전송해 줄 uniform 변수들 선언
uniform sampler2D diffuseMap; // diffuse 텍스쳐 (0번 texture unit 에 바인딩된 텍스쳐 객체 샘플링)
uniform sampler2D normalMap; // normal 텍스쳐 (1번 texture unit 에 바인딩된 텍스쳐 객체 샘플링)
uniform sampler2D depthMap; // 포토샵에서 흑백 반전으로 heightMap(displacementMap) 을 뒤집음 -> depthMap 텍스쳐 (2번 texture unit 에 바인딩된 텍스쳐 객체 샘플링)

uniform float heightScale; // depthMap 에서 샘플링한 depth(= height 을 반전시킨 값)들의 규모를 전체적으로 조정하는 값 

/* 현재의 texCoord 로부터 변위(= 위치 이동, offset, displacement)된 텍스쳐 좌표를 반환하는 ParallaxMapping 함수 */ 
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir) {
  // depthMap 에서 depth(= height 을 반전시킨 값) 샘플링
  float height = texture2D(depthMap, texCoords).r;

  // 현재 texCoord 로부터 이동시킬 offset 에 해당하는 벡터 p 계산 (하단 필기 참고)
  vec2 p = viewDir.xy / viewDir.z * (height * heightScale);

  /*
    depthMap(= heightMap 반전시킨 텍스쳐)을 사용할 경우, 
    뷰 벡터의 반대 방향으로 offset 시켜야 하므로, 
    현재 texCoords 에서 offset 벡터 p 를 빼줌.
  */
  return texCoords - p;
}

void main() {
  // 뷰 벡터 (탄젠트 공간 카메라 위치 ~ 탄젠트 공간 프래그먼트 위치)
  vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);

  // 현재 프래그먼트의 texCoords 에서 변위(displacement)된 텍스쳐 좌표를 계산
  vec2 texCoords = ParallaxMapping(fs_in.TexCoords, viewDir);

  // 텍스쳐 좌표 변위(displacement)로 인해, 일반적인 텍스쳐 좌표 범위인 [0, 1]을 넘어설 경우, 프래그먼트를 discard 하여 경계 부분의 artifact 제거
  if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0) {
    discard;
  }

  // normal map 으로부터 탄젠트 공간 노멀벡터 샘플링
  vec3 normal = texture2D(normalMap, texCoords).rgb;

  // [0, 1] 범위에 저장되어 있던 탄젠트 공간 노멀벡터를 [-1, 1] 범위로 변환
  normal = normalize(normal * 2.0 - 1.0);

  // diffsue map 으로부터 diffuse 색상 샘플링
  vec3 color = texture2D(diffuseMap, texCoords).rgb;

  /* ambient 성분 계산 */

  // ambient 강도
  float ambientStrength = 0.1;

  // ambient 강도에 텍스쳐 색상 곱해서 최종 ambient 성분값 계산
  vec3 ambient = ambientStrength * color; 

  /* diffuse 성분 계산 */
  // 조명계산에 사용되는 모든 방향벡터들은 항상 정규화를 해줄 것! -> 그래야 내적계산 시 정확한 cos 값만 얻을 수 있음!

  // 조명벡터 (탄젠트 공간 프래그먼트 위치 ~ 탄젠트 공간 광원 위치)
  vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);

  // 노멀벡터와 조명벡터 내적 > diffuse 성분의 세기(조도) 계산 (참고로, 음수인 diffuse 값은 조명값 계산을 부정확하게 만들기 때문에, 0.0 으로 clamping 시킴)
  float diff = max(dot(lightDir, normal), 0.0);

  // diffuse 조도에 텍스쳐 색상 곱해서 최종 diffuse 성분값 계산
  vec3 diffuse = diff * color; 

  /* specular 성분 계산 */

  // 뷰 벡터와 조명벡터 사이의 halfway 벡터 계산 (두 벡터의 합 -> 두 벡터 사이를 가로지르는 하프 벡터 (<셰이더 코딩 입문> p.222 참고))
  vec3 halfwayDir = normalize(lightDir + viewDir);

  // specular 성분의 조도 계산
  // 프래그먼트 지점의 normal 벡터와 halfway 벡터를 clamping 내적함
  // 내적값을 32제곱 > 32는 shininess 값으로써, 값이 클수록 highlight 영역이 정반사되고, 값이 작을수록 난반사됨.
  // Blinn-Phong 모델에서는 동일한 조건 하에 기본 Phong 모델에 비해 내적값이 더 작게 계산되기 때문에,
  // 일반적인 관례상 기본 Phong 모델보다 2~4배 큰 shininess(광택값)을 사용한다.
  float spec = pow(max(dot(normal, halfwayDir), 0.0), 32);

  // specular 조도에 조명색상(어두운 흰색)을 곱해 specular 성분값 계산
  vec3 specular = spec * vec3(0.2); 

  // 3가지 성분을 모두 더하여 최종 색상 결정
  FragColor = vec4(ambient + diffuse + specular, 1.0);
}

/*
  버텍스 쉐이더로부터
  탄젠트 공간 기준의 조명 위치값, 카메라 위치값, 프래그먼트 위치값을 모두 얻어왔으므로,

  normal map 에서 샘플링한
  탄젠트 공간 노멀벡터를 별도의 변환 처리 없이

  조명계산에 곧바로 사용할 수 있음!
*/

/*
  현재 texCoord 로부터 이동시킬 offset 에 해당하는 벡터 p 계산


  1. (height * heightScale)
  
  샘플링한 depth 값의 전체적인 규모를 조정함.


  2. viewDir.xy / viewDir.z

  뷰 벡터는 정규화된 상태이므로,
  현재 카메라가 현재 프래그먼트를 수직 방향으로 위에서 바라볼수록, viewDir.z 는 0에 가깝고,
  현재 카메라가 현재 프래그먼트를 비스듬한 각도에서 바라볼수록, viewDir.z 는 1에 가까움.

  따라서, 이 값을 viewDir.xy 로 나눠서 이동시킬 offset 벡터 p 를 구한다면,
  현재 카메라가 수직방향으로 바라볼수록, 벡터 p 의 이동거리가 더 짧아질 것임.
  반대로, 비스듬한 각도에서 바라볼수록, 벡터 p 의 이동거리가 더 길어질 것임.

  Parallax Mapping 자체가 비스듬히 바라봤을 때
  더 튀어나와 보이도록 텍스쳐를 맵핑하는 기법이기 때문에,

  비스듬한 각도에서 볼수록 더 사실적으로 보이기 위해 
  벡터 p 의 이동거리를 조정(adjusting)한 것임!


  3. viewDir.xy / viewDir.z * (height * heightScale)

  기본적으로 벡터 p 의 길이는
  샘플링한 depth 값에 의해 결정되기 때문에,

  조정된 height 값을 벡터 p 에 최종적으로 곱해줌.
*/