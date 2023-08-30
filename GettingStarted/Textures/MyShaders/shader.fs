#version 330 core

out vec4 FragColor;

in vec3 ourColor;
in vec2 texCoord; // 버텍스 쉐이더로부터 linking 시 보간된 텍스쳐 좌표를 입력받는 입력변수 선언

uniform sampler2D ourTexture; // 그리기 명령을 수행하기 전, 바인딩하여 전송한 OpenGL 2D 텍스쳐 객체에 접근할 수 있는 sampler2D 타입의 uniform 변수 선언

void main() {
  // FragColor = vec4(ourColor, 1.0);
   // 전달받은 텍스쳐를 보간된 uv 좌표로 샘플링하는 함수 texture2D() 함수 사용
   // 이때, 보간된 uv좌표를 이용해서 샘플링할 색상을 결정해주는 방식이 GL_LINEAR, GL_NEAREST 같은 모드
   // 텍스쳐에서 샘플링된 색상과 보간된 버텍스 색상 섞어서 최종 색상 계산
  FragColor = texture2D(ourTexture, texCoord) * vec4(ourColor, 1.0);
}