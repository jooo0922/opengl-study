#version 330 core

out vec4 FragColor;

// vertex shader 단계에서 전달받는 입력 변수 선언
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

/* OpenGL 에서 전송해 줄 uniform 변수들 선언 */

// PBR Material 파라미터 값이 텍셀 단위로 저장된 텍스쳐 객체들의 sampler 변수 선언
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

// 광원 정보를 전송받는 uniform 변수 선언
uniform vec3 lightPositions[1];
uniform vec3 lightColors[1];

void main() {

}
