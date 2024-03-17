#version 330 core

// layout location specifier 로 MRT 프레임버퍼에 바인딩된 각 color attachment 에 대응되는 출력 변수 선언
// ex> location = 0 으로 지정 시, 해당 출력 변수에 입력되는 색상은 GL_COLOR_ATTACHMENT0 버퍼에 저장됨.
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

/* vertex shader 단계에서 전달받는 입력 interface block 선언 */
in VS_OUT {
  vec3 FragPos;
  vec3 Normal;
  vec2 TexCoords;
} fs_in;

/* 
  OpenGL 에서 전송해 줄 uniform 변수들 선언 
*/

// 광원 큐브 색상을 전송받을 uniform 변수 선언
uniform vec3 lightColor;

void main() {
  // FragColor 출력 변수(= 즉, GL_COLOR_ATTACHMENT0 텍스쳐 객체)에는 씬에 렌더링되는 광원 큐브 색상을 저장함.
  FragColor = vec4(lightColor, 1.0);

  // 최종 색상을 grayscale 로 변환하여 해당 색상의 밝기값 계산
  // https://github.com/jooo0922/opengl-study/blob/main/AdvancedOpenGL/Framebuffers/MyShaders/framebuffers_screen_grayscale.fs 참고!
  // 위 쉐이더 코드에서의 grayscale 계산을 내적 계산(dot)으로 단순화한 것 뿐임!
  float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));

  // BrightColor 출력 변수(= 즉, GL_COLOR_ATTACHMENT1 텍스쳐 객체)에는 광원큐브 영역의 색상만 저장함. 
  if(brightness > 1.0) {
    // 특정 밝기값 threshold 를 넘는 색상 (= 즉, 광원큐브 영역의 색상)만 BrightColor 출력 변수에 저장
    BrightColor = vec4(FragColor.rgb, 1.0);
  } else {
    // 특정 밝기값 threshold 를 넘지 못하는 색상들은 모두 검정색으로 BrightColor 출력 변수에 저장
    BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
  }
}