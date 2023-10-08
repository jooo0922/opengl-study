#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal; // 정점 노멀 데이터를 전송받는 attribute 변수 선언

// 프래그먼트 쉐이더로 출력할 보간변수 선언
out vec3 LightingColor;

// glm::mat4 타입 좌표계 변환 행렬을 전송받는 uniform 변수 선언
uniform mat4 model; // 모델 행렬
uniform mat4 view; // 뷰 행렬
uniform mat4 projection; // 투영 행렬

// 조명 계산 관련 uniform 변수들 선언
uniform vec3 lightPos; // 광원 위치 > 조명벡터 계산에서 사용
uniform vec3 viewPos; // 카메라 위치 > 뷰 벡터 계산에서 사용
uniform vec3 lightColor; // 광원의 색상값 (= 조명 색상)

void main() {
  /* Gouraud Shading 계산 */

  vec3 WorldPos = vec3(model * vec4(aPos, 1.0)); // 정점 위치에 모델행렬만 곱해서 월드공간 좌표로 변환 > 프래그먼트 쉐이더로 보간 전송
  vec3 WorldNormal = mat3(transpose(inverse(model))) * aNormal; // 정점 노멀벡터를 월드공간 노멀벡터로 변환 > 프래그먼트 쉐이더로 보간 전송

  /* ambient 성분 계산 */
  float ambientStrength = 0.1; // ambient 강도
  vec3 ambient = ambientStrength * lightColor; // ambient 강도에 조명색상 곱해서 최종 ambient 성분값 계산 (원래는 각 성분마다 색상을 별도 지정할 수 있어야 함.)

  /* diffuse 성분 계산 */
  // 조명계산에 사용되는 모든 방향벡터들은 항상 정규화를 해줄 것! -> 그래야 내적계산 시 정확한 cos 값만 얻을 수 있음!
  vec3 norm = normalize(WorldNormal); // 프래그먼트에 수직인 노멀벡터
  vec3 lightDir = normalize(lightPos - WorldPos); // 조명벡터 (프래그먼트 위치 ~ 광원 위치)
  float diff = max(dot(norm, lightDir), 0.0); // 노멀벡터와 조명벡터 내적 > diffuse 성분의 세기(조도) 계산 (참고로, 음수인 diffuse 값은 조명값 계산을 부정확하게 만들기 때문에, 0.0 으로 clamping 시킴)
  vec3 diffuse = diff * lightColor; // diffuse 조도에 조명색상 곱해서 최종 diffuse 성분값 계산

  /* specular 성분 계산 */
  float specularStrength = 0.5; // specular 강도
  vec3 viewDir = normalize(viewPos - WorldPos); // 뷰 벡터 (카메라 위치 ~ 프래그먼트 위치)
  vec3 reflectDir = reflect(-lightDir, norm); // 반사 벡터 (조명벡터는 카메라 위치부터 출발하도록 방향을 negate)
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32); // 뷰 벡터와 반사 벡터의 내적값을 32제곱 > 32는 shininess 값으로써, 값이 클수록 highlight 영역이 정반사되고, 값이 작을수록 난반사됨. > specular 조도 계산
  vec3 specular = specularStrength * spec * lightColor; // specular 조도에 specular 강도와 조명색상을 곱해 specular 성분값 계산

  // 3가지 성분을 모두 더한 조명 색상에 물체 색상을 component-wise 곱으로 계산하여 최종 색상을 계산하고,
  // 이를 프래그먼트 쉐이더로 보간하여 전송 (Gouraud Shading)
  LightingColor = (ambient + diffuse + specular);

  gl_Position = projection * view * vec4(WorldPos, 1.0); // 오브젝트 공간 좌표에 모델 행렬 > 뷰 행렬 > 투영 행렬 순으로 곱해서 좌표계를 변환시킴.
}

/*
  노멀 행렬 (Normal Matrix)

  (0.5, 1.4) 같은 비균일 스케일링(non-uniform sacle) 변환이 물체에 적용될 때,
  기존에 정의한 노멀벡터는 더 이상 정점에 대해 수직(perpendicular)이지 않음.

  따라서, 월드좌표로 변환된 정점에 따라서
  노멀벡터도 월드공간 노멀벡터로 변환해줘야 하는데,
  이게 일반적인 4*4 모델행렬을 노멀벡터에 곱해서 사용할 수는 없음.

  첫 번째로,
  노멀벡터는 vec3 타입인데 mat4 행렬을 곱할 수가 없고, (즉, 동차좌표 w가 없음)

  두 번째로,
  노멀벡터는 방향값만 존재할 뿐, 위치는 존재하지 않으므로,
  이동변환이 포함된 모델행렬에 곱해봤자 이동이 불가함.

  따라서, 4*4 모델행렬에서 이동변환에 대한 값이 존재하는 component 부분을 제외한,
  '좌상단 3*3 행렬'만 가지고서 노멀벡터를 월드공간으로 변환할 수 있음.

  -> 이러한 이유 때문에 4*4 모델행렬로부터 좌상단 3*3 행렬을 가져와서
  노멀행렬을 만들었던 것임!

  또, 앞서 얘기한 것처럼 비균일 변환에 의한
  노멀벡터 왜곡을 방지하기 위해, 해당 3*3 행렬의 역행렬의 전치행렬을 구해서
  노멀벡터의 월드공간 변환에 특별 맞춤으로 계산된 행렬이 바로 '노멀행렬' 인 것임!

  이때, 역행렬 계산은 굉장히 비용이 높은 연산이기 때문에,
  가급적 쉐이더 단에서 버텍스마다 중복 계산하기 보다는,
  CPU 단에서 계산한 다음, 쉐이더 프로그램으로 넘겨주는 방식이 권장된다고 함!
  -> 실제로 TAOS 프로젝트 만들 때 이렇게 했었지!
*/