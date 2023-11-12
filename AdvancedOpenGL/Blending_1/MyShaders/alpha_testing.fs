#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main() {
  // 전송받은 텍스쳐 객체를 샘플링 -> .png 이미지를 사용한다면, RGBA 채널을 모두 갖는 vec4 타입 texel 을 반환함
  vec4 texColor = texture2D(texture1, TexCoords);

  // texel 의 Alpha 채널 값이 특정 threshold(0.1) 보다 작으면 프래그먼트 쉐이더 단계에서 
  // 해당 프래그먼트를 탈락시킴 -> 이러한 기법을 Alpha Testing 이라고 함!
  // Alpha Testing 은 Depth Testing 과 엮이는 문제가 없다보니 Blending 보다는 간단한 기법!
  // 반면, Alpha Testing 은 '반투명' 프래그먼트를 처리할 수 없다는 한계가 존재함!
  if(texColor.a < 0.1) {
    discard;
  }

  // Alpha Testing 을 통과한 색상값만 최종 색상으로 출력
  FragColor = texColor;
}