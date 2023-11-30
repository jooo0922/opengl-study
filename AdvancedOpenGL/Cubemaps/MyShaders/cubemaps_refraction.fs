#version 330 core

out vec4 FragColor;

// 버텍스 쉐이더로부터 보간되어 입력된 월드공간 위치 및 노멀벡터
in vec3 Normal;
in vec3 Position;

/* uniform 변수 선언 */
uniform vec3 cameraPos; // 뷰 벡터 계산을 위해 선언한 카메라 위치 변수
uniform samplerCube skybox; // 큐브맵 반사 구현을 위해 선언한 큐브맵 sampler 변수

void main() {
  /* 큐브맵 '굴절' 재질 구현 */

  // <공기의 IOR(Index of Rafraction) / 유리의 IOR> 식으로 계산된 굴절률 
  float ratio = 1.00 / 1.52;

  // 뷰 벡터 계산
  vec3 I = normalize(Position - cameraPos); 

  // 정규화된 뷰 벡터와 노멀벡터, 굴절률을 인자로 전달하여 굴절벡터 계산 (refract() 내장함수 사용)
  vec3 R = refract(I, normalize(Normal), ratio); 

  // 계산된 반사벡터를 방향벡터 삼아 큐브맵 샘플링 후, 최종 색상 계산 -> 큐브맵 반사(거울 재질) 구현!
  FragColor = vec4(texture(skybox, R).rgb, 1.0);
}

/*
  refract() 내장함수와 IOR(굴절률)


  "IOR"은 "Index of Refraction"의 약자로 굴절률을 나타내는 개념. 
  본문에서는 "Refractive Index" 라는 단어로 정의되어 있음.

  굴절률은 빛이 특정 매질에서 다른 매질로 옮겨갈 때 
  어떻게 굴절되는지를 나타내는 물리적인 특성임.

  GLSL 내장함수 refract() 에 전달되는 굴절률은 이 IOR 값이며, 
  두 매질 간의 속도 비율을 나타낸다! 
  
  일반적으로 굴절률이 클수록 굴절이 크게 일어나게 되며, 
  이는 빛의 방향이 매질 경계에서 변화함을 의미함.


  refract(vec3 incident, vec3 normal, float eta) 
  함수는 세 개의 매개변수를 사용합니다:

  incident: 굴절되기 전의 빛의 방향을 나타내는 벡터.
  normal: 표면의 법선 벡터. 표면을 수직으로 향하는 벡터로, 매질의 경계면에서 굴절을 계산하는 데 사용됩니다.
  eta: 굴절률 (Refractive Index)을 나타내는 비율입니다.
*/