#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal; // 정점 노멀 데이터를 전송받는 attribute 변수 선언
layout(location = 2) in vec2 aTexCoords; // 정점 uv 데이터를 전송받는 attribute 변수 선언

// 프래그먼트 쉐이더로 출력할 보간변수 선언
out vec3 FragPos; // 보간될 프래그먼트 월드공간 좌표 
out vec3 Normal; // 보간될 프래그먼트 노멀벡터
out vec2 TexCoords; // 보간된 프래그먼트 uv 좌표

// glm::mat4 타입 좌표계 변환 행렬을 전송받는 uniform 변수 선언
uniform mat4 model; // 모델 행렬
uniform mat4 view; // 뷰 행렬
uniform mat4 projection; // 투영 행렬

void main() {
  FragPos = vec3(model * vec4(aPos, 1.0)); // 정점 위치에 모델행렬만 곱해서 월드공간 좌표로 변환 > 프래그먼트 쉐이더로 보간 전송
  Normal = mat3(transpose(inverse(model))) * aNormal; // 정점 노멀벡터를 월드공간 노멀벡터로 변환 > 프래그먼트 쉐이더로 보간 전송
  TexCoords = aTexCoords; // 정점 uv 좌표를 프래그먼트 쉐이더로 보간 전송

  gl_Position = projection * view * vec4(FragPos, 1.0); // 오브젝트 공간 좌표에 모델 행렬 > 뷰 행렬 > 투영 행렬 순으로 곱해서 좌표계를 변환시킴.
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