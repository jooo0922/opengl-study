#version 330 core

// 입력 primitive 를 삼각형으로 지정
layout(triangles) in;

// 출력 primitive 를 line_strip 으로 지정. -> 삼각형의 각 정점마다 1개씩의 line 을 그릴 것임.
// 따라서, 1개의 line 을 그리려면 정점이 2개(시작점, 끝점)씩 필요.
// -> 삼각형 3개의 정점마다 1개의 line 을 그리려면 총 6개의 정점이 필요하므로, max_vertices = 6 으로 설정!
layout(line_strip, max_vertices = 6) out;

// vertex shader 단계에서 전달받는 입력 interface block 선언
// block name 은 쉐이더 단계 간 이름을 동일하게 맞출 것!
in VS_OUT {
  vec3 normal;
} gs_in[]; // geometry shader 에서는 항상 interface block 을 배열로 선언해줘야 한댔지?

// 노멀벡터를 시각화할 line_strip 길이를 제한할 상수
const float MAGNITUDE = 0.2;

// 투영행렬을 전송받을 uniform 변수 선언
uniform mat4 projection;

// 입력 primitive 삼각형의 각 정점 인덱스를 입력받은 후,
// 해당 정점의 노멀벡터를 시각화하는 출력 primitive line_strip 생성 함수 
void GenerateLine(int index) {
  // 삼각형 정점의 클립좌표 (투영행렬 * 뷰 좌표계) 계산 -> 노멀벡터를 시각화하는 line_strip 의 시작점 추가
  gl_Position = projection * gl_in[index].gl_Position;
  EmitVertex();

  // 삼각형 정점에서 노멀벡터 방향으로 MAGNITUDE 만큼 이동한 지점의 클립좌표 (투영행렬 * 뷰 좌표계) 계산 -> 노멀벡터를 시각화하는 line_strip 의 끝점 추가
  // 이 끝점 계산 때문에 projection 행렬 변환만 예외적으로 geometry shader 에서 처리해준 것!
  // 왜냐하면, 노멀벡터를 시각화한 line 의 길이를 MAGNITUDE 만큼 맞추도록 끝점을 계산하려면, 뷰 좌표계에서 먼저 해당 길이만큼 offset 을 적용한 다음,
  // 투영행렬을 곱해서 클립좌표로 변환시켜야 정확한 끝점 좌표가 나옴. 클립좌표계는 -1 ~ 1 사이의 값만 갖는 좌표계라서, 
  // 클립좌표계에서 직접 MAGNITUDE 만큼 offset 을 때리면 부정확한 값이 나오겠지!
  gl_Position = projection * (gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0) * MAGNITUDE);
  EmitVertex();

  // EmitVertex() 로 추가된 정점들을 binding(결합)하여 1개의 line_strip primitive 생성
  // -> 이것이 그래픽스 파이프라인에서 'primitive assembly' 단계에 해당!
  EndPrimitive();
}

void main() {
  // 삼각형 primitive 의 0번 정점 노멀벡터 시각화하는 line_strip 생성
  GenerateLine(0);

  // 삼각형 primitive 의 1번 정점 노멀벡터 시각화하는 line_strip 생성
  GenerateLine(1);

  // 삼각형 primitive 의 2번 정점 노멀벡터 시각화하는 line_strip 생성
  GenerateLine(2);
}