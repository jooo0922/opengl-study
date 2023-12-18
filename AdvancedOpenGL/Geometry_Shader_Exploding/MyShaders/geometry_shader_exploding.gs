#version 330 core

// 입력 primitive 를 삼각형으로 지정
layout(triangles) in;

// 출력 primitive 를 triangle_strip 으로 지정. 삼각형의 정점 개수는 최대 3개로 지정.
layout(triangle_strip, max_vertices = 3) out;

// vertex shader 단계에서 전달받는 입력 interface block 선언
// block name 은 쉐이더 단계 간 이름을 동일하게 맞출 것!
in VS_OUT {
  vec2 texCoords;
} gs_in[]; // geometry shader 에서는 항상 interface block 을 배열로 선언해줘야 한댔지?

// 프래그먼트 쉐이더로 보간하여 전송할 uv 좌표 출력 변수 선언
out vec2 TexCoords;

// 경과시간(elapsed time)을 전송받을 uniform 변수 선언
uniform float time;

// 입력 primitive 삼각형의 노멀벡터를 계산하여 반환하는 함수
vec3 GetNormal() {
  // 삼각형의 0번 정점 ~ 1번 정점 사이의 벡터 a 계산
  vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);

  // 삼각형의 2번 정점 ~ 1번 정점 사이의 벡터 b 계산
  vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);

  // 삼각형 내의 두 벡터 a, b 를 외적하여 정규화한 벡터 계산
  // -> 외적으로 계산된 벡터는 삼각형 내의 두 벡터 a, b 모두에 직교하므로, 입력 primitive 삼각형에도 직교함!
  // -> 노멀벡터 계산 완료!
  return normalize(cross(a, b));
}

// 입력 primitive 삼각형 각 정점의 위치값과 삼각형 노멀벡터를 입력받은 후,
// 경과시간에 따라 노멀벡터 방향으로 이동할 정점의 위치값을 계산하는 함수 
vec4 explode(vec4 position, vec3 normal) {
  // 경과시간에 따라 노멀벡터 방향으로 왕복할 최대 거리 (값이 클수록 더 먼 거리를 왕복함)
  float magnitude = 2.0;

  // 현재 삼각형 각 정점 위치에 더해줄 offset(이동량)
  // 아래 공식을 찬찬히 계산해보면, normal 방향으로 0 ~ magnitude 거리만큼 이동할 offset 을 계산하고 있음.
  vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude;

  // 각 정점의 현재 위치 + offset(이동량) 계산하여 반환
  return position + vec4(direction, 0.0);
}

void main() {
  // 입력 primitive 삼각형의 노멀벡터 (== 삼각형 각 정점의 노멀벡터) 계산
  vec3 normal = GetNormal();

  // 삼각형의 0번 정점 이동 위치 및 출력할 uv 좌표 계산
  gl_Position = explode(gl_in[0].gl_Position, normal);
  TexCoords = gs_in[0].texCoords;
  EmitVertex();

  // 삼각형의 1번 정점 이동 위치 및 출력할 uv 좌표 계산
  gl_Position = explode(gl_in[1].gl_Position, normal);
  TexCoords = gs_in[1].texCoords;
  EmitVertex();

  // 삼각형의 2번 정점 이동 위치 및 출력할 uv 좌표 계산
  gl_Position = explode(gl_in[2].gl_Position, normal);
  TexCoords = gs_in[2].texCoords;
  EmitVertex();

  // EmitVertex() 로 추가된 정점들을 binding(결합)하여 1개의 triangle_strip primitive 생성
  // -> 이것이 그래픽스 파이프라인에서 'primitive assembly' 단계에 해당!
  EndPrimitive();
}