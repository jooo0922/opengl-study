#version 330 core

out vec4 FragColor;

in vec2 texCoord; // 버텍스 쉐이더로부터 linking 시 보간된 텍스쳐 좌표를 입력받는 입력변수 선언

// 그리기 명령을 수행하기 전, 바인딩하여 전송한 OpenGL 2D 텍스쳐 객체에 접근할 수 있는 sampler2D 타입의 uniform 변수 선언
uniform sampler2D texture1; // 0번 텍스쳐 유닛 위치를 전송받는 sampler 변수 선언
uniform sampler2D texture2; // 1번 텍스쳐 유닛 위치를 전송받는 sampler 변수 선언

void main() {
  // texture1 과 texture2 를 샘플링한 색상을 혼합해서 최종 색상 계산.
  // mix() 함수의 세 번째 인자에 0.2 를 전달함으로써, 첫 번째 샘플링 색상을 80% 로, 두 번째 샘플링 색상을 20% 비율로 선형보간하여 섞어주도록 함.
  FragColor = mix(texture2D(texture1, texCoord), texture2D(texture2, texCoord), 0.2);
}