#version 330 core

out vec4 FragColor;

in vec3 ourColor;
in vec2 texCoord; // 버텍스 쉐이더로부터 linking 시 보간된 텍스쳐 좌표를 입력받는 입력변수 선언

// 그리기 명령을 수행하기 전, 바인딩하여 전송한 OpenGL 2D 텍스쳐 객체에 접근할 수 있는 sampler2D 타입의 uniform 변수 선언
uniform sampler2D texture1; // 0번 텍스쳐 유닛 위치를 전송받는 sampler 변수 선언
uniform sampler2D texture2; // 1번 텍스쳐 유닛 위치를 전송받는 sampler 변수 선언
uniform float mixAlpha; // mix() 함수의 세 번째 파라미터로 전달할 값을 uniform 변수로 관리

void main() {
  // FragColor = vec4(ourColor, 1.0);
   // 전달받은 텍스쳐를 보간된 uv 좌표로 샘플링하는 함수 texture2D() 함수 사용
   // 이때, 보간된 uv좌표를 이용해서 샘플링할 색상을 결정해주는 방식이 GL_LINEAR, GL_NEAREST 같은 모드
   // 텍스쳐에서 샘플링된 색상과 보간된 버텍스 색상 섞어서 최종 색상 계산
  // FragColor = texture2D(texture1, texCoord) * vec4(ourColor, 1.0);

  // texture1 과 texture2 를 샘플링한 색상을 혼합해서 최종 색상 계산.
  // mix() 함수의 세 번째 인자에 0.2 를 전달함으로써, 첫 번째 샘플링 색상을 80% 로, 두 번째 샘플링 색상을 20% 비율로 선형보간하여 섞어주도록 함.
  FragColor = mix(texture2D(texture1, texCoord), texture2D(texture2, texCoord), mixAlpha);
}